/**
 * zicModKick32_v2 desktop emulator.
 *
 * Emulator only (not part of the module): ALSA output, SFML window, sequencer.
 * Module code lives in ../core and ../../audio/engines/Kick32v2.h.
 */

#include <alsa/asoundlib.h>
#include <atomic>
#include <chrono>
#include <iostream>
#include <mutex>
#include <pthread.h>
#include <sched.h>
#include <thread>
#include <vector>

#include "EmuUi.h"
#include "Persist.h"
#include "Sequencer.h"
#include "audio/engines/Kick32v2.h"
#include "core/Kick32v2Panel.h"

std::atomic<bool> keep_running(true);
std::mutex audioMutex;
std::atomic<int> kickTrigger { 0 };
std::atomic<bool> kickMuted { false };

constexpr unsigned int SAMPLE_RATE = 44100;
constexpr size_t NUM_FRAMES = 256;

Kick32v2 kick(SAMPLE_RATE);
Kick32v2Panel::Panel panel(kick);
Sequencer seq(SAMPLE_RATE);

EmuUi::Scope scope;
int scopePos = -1; // sample index in the current capture, -1 = idle

constexpr int SCOPE_BUCKET = SAMPLE_RATE / 1000; // 1 ms per bucket

void noteOn(int note)
{
    kick.noteOn((uint8_t)std::clamp(note, 0, 127), 1.0f);
    for (int i = 0; i < EmuUi::Scope::N; ++i) scope.out[i] = scope.layer[i] = 0.0f;
    scopePos = 0;
}

snd_pcm_t* audioInit()
{
    snd_pcm_t* pcm;
    int err = snd_pcm_open(&pcm, "default", SND_PCM_STREAM_PLAYBACK, 0);
    if (err < 0) {
        std::cerr << "Audio open error: " << snd_strerror(err) << "\n";
        return nullptr;
    }
    snd_pcm_set_params(pcm, SND_PCM_FORMAT_S16_LE, SND_PCM_ACCESS_RW_INTERLEAVED, 2, SAMPLE_RATE, 1, 20000);
    return pcm;
}

void audioWorker(snd_pcm_t* pcm)
{
    std::vector<int16_t> buffer(NUM_FRAMES * 2);
    while (keep_running) {
        {
            std::lock_guard<std::mutex> lock(audioMutex);
            if (kickTrigger.exchange(0)) noteOn(60 + seq.transpose);
            for (size_t f = 0; f < NUM_FRAMES; ++f) {
                int note = 0;
                if (seq.tick(note) && !kickMuted) noteOn(60 + seq.transpose + note);
                float out = kick.sample();
                if (scopePos >= 0) {
                    int b = scopePos++ / SCOPE_BUCKET;
                    if (b < EmuUi::Scope::N) {
                        scope.out[b] = std::max(scope.out[b], std::abs(out));
                        scope.layer[b] = std::max(scope.layer[b], std::abs(kick.lastLayer));
                    } else {
                        scopePos = -1;
                    }
                }
                int16_t v = (int16_t)(std::clamp(out, -1.0f, 1.0f) * 32767.0f);
                buffer[f * 2] = buffer[f * 2 + 1] = v;
            }
        }

        int w = snd_pcm_writei(pcm, buffer.data(), NUM_FRAMES);
        if (w < 0 && snd_pcm_recover(pcm, w, 0) < 0) std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
}

int main()
{
    snd_pcm_t* pcm = audioInit();
    if (!pcm) return 1;

    const bool screenshot = std::getenv("ZIC_SCREENSHOT") != nullptr; // never touch the save file then
    const std::string stateFile = Persist::path();
    if (!screenshot) Persist::load(stateFile, panel, seq);
    std::string lastSaved = Persist::serialize(panel, seq);
    if (screenshot && std::getenv("ZIC_DEMO")) { // screenshot with a late, tuned tail, to see it in the scope
        kick.tailStart.set(55); kick.tailPitch.set(7); kick.glide.set(45); kick.stack.set(60); kick.tailShape.set(60); kick.decay.set(500);
    }

    std::thread audioThread(audioWorker, pcm);
    sched_param sch {};
    sch.sched_priority = 35;
    pthread_setschedparam(audioThread.native_handle(), SCHED_FIFO, &sch); // best effort

    // fit the window on small screens, keep the logical size for drawing
    auto desktop = sf::VideoMode::getDesktopMode();
    float scale = std::min({ 1.0f, (desktop.width - 40.0f) / EmuUi::W, (desktop.height - 100.0f) / EmuUi::H });
    sf::ContextSettings ctx;
    ctx.antialiasingLevel = 8;
    sf::RenderWindow window(sf::VideoMode((unsigned)(EmuUi::W * scale), (unsigned)(EmuUi::H * scale)), "zicModKick32_v2", sf::Style::Default, ctx);
    window.setFramerateLimit(60);
    window.setKeyRepeatEnabled(false);

    EmuUi ui(panel, seq, scope);

    // keep the aspect ratio: letterbox the logical view
    auto fitView = [&](unsigned w, unsigned h) {
        sf::View view(sf::FloatRect(0, 0, EmuUi::W, EmuUi::H));
        float winRatio = (float)w / h, uiRatio = EmuUi::W / EmuUi::H;
        float sx = 1, sy = 1, px = 0, py = 0;
        if (winRatio > uiRatio) { sx = uiRatio / winRatio; px = (1 - sx) * 0.5f; }
        else { sy = winRatio / uiRatio; py = (1 - sy) * 0.5f; }
        view.setViewport(sf::FloatRect(px, py, sx, sy));
        window.setView(view);
    };
    fitView(window.getSize().x, window.getSize().y);

    auto logical = [&](int x, int y) { return window.mapPixelToCoords({ x, y }); };

    int autosave = 0;
    while (window.isOpen()) {
        sf::Event e;
        while (window.pollEvent(e)) {
            bool shift = sf::Keyboard::isKeyPressed(sf::Keyboard::LShift) || sf::Keyboard::isKeyPressed(sf::Keyboard::RShift);
            switch (e.type) {
            case sf::Event::Closed: window.close(); break;
            case sf::Event::Resized: fitView(e.size.width, e.size.height); break;
            case sf::Event::MouseButtonPressed: ui.mousePressed(logical(e.mouseButton.x, e.mouseButton.y), e.mouseButton.button, shift); break;
            case sf::Event::MouseButtonReleased: ui.mouseReleased(); break;
            case sf::Event::MouseMoved: ui.mouseMoved(logical(e.mouseMove.x, e.mouseMove.y), shift); break;
            case sf::Event::MouseWheelScrolled: ui.wheel(logical(e.mouseWheelScroll.x, e.mouseWheelScroll.y), e.mouseWheelScroll.delta, shift); break;
            case sf::Event::KeyPressed:
                if (e.key.code == sf::Keyboard::Escape) window.close();
                else if (e.key.code == sf::Keyboard::Space) ui.trigDown();
                else if (e.key.code == sf::Keyboard::Return || e.key.code == sf::Keyboard::P) { std::lock_guard<std::mutex> l(audioMutex); seq.toggle(); }
                break;
            case sf::Event::KeyReleased:
                if (e.key.code == sf::Keyboard::Space) ui.trigUp();
                break;
            default: break;
            }
        }
        // autosave once a second, only when something changed
        if (!screenshot && ++autosave >= 60) {
            autosave = 0;
            std::string now;
            {
                std::lock_guard<std::mutex> l(audioMutex);
                now = Persist::serialize(panel, seq);
            }
            if (now != lastSaved && Persist::save(stateFile, now)) lastSaved = now;
        }

        auto mp = sf::Mouse::getPosition(window);
        ui.draw(window, logical(mp.x, mp.y));
        window.display();

        // ZIC_SCREENSHOT=file.png ./zic : save a frame and quit (docs / quick visual check)
        static int frame = 0;
        if (const char* shot = std::getenv("ZIC_SCREENSHOT")) {
            if (frame == 6 && std::getenv("ZIC_DEMO")) kickTrigger = 1;
            if (++frame == (std::getenv("ZIC_DEMO") ? 60 : 10)) { // demo: let the whole hit reach the scope
                sf::Texture tex;
                tex.create(window.getSize().x, window.getSize().y);
                tex.update(window);
                tex.copyToImage().saveToFile(shot);
                window.close();
            }
        }
    }

    keep_running = false;
    audioThread.join();
    snd_pcm_close(pcm);
    if (!screenshot) Persist::save(stateFile, Persist::serialize(panel, seq));
    return 0;
}
