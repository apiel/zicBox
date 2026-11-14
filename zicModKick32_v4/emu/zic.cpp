/**
 * zicModKick32_v4 desktop emulator.
 *
 * Emulator only (not part of the module): ALSA output, SFML window, test sequencer.
 * Module code lives in ../core and ../../audio/engines/Kick32v4.h.
 *
 *   ZIC_SCREENSHOT=file.png ./zic             save a frame and quit
 *   ZIC_SCREENSHOT=file.png ZIC_DEMO=1 ./zic  same, with a busy patch and a hit in the scope
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
#include "audio/engines/Kick32v4.h"
#include "core/Kick32v4Panel.h"

std::atomic<bool> keep_running(true);
std::mutex audioMutex;
std::atomic<int> kickTrigger { 0 };
std::atomic<bool> kickMuted { false };

constexpr unsigned int SAMPLE_RATE = 44100;
constexpr size_t NUM_FRAMES = 256;

Kick32v4 kick(SAMPLE_RATE);
Kick32v4Panel::Panel panel(kick);
Sequencer seq(SAMPLE_RATE);

Capture cap;
int scopePos = -1; // sample index in the current capture, -1 = idle

constexpr int SCOPE_BUCKET = (int)(SAMPLE_RATE * Capture::MS_PER_BUCKET / 1000.0f);

void noteOn(int note)
{
    kick.noteOn((uint8_t)std::clamp(note, 0, 127), 1.0f);
    for (int i = 0; i < Capture::N; ++i) cap.lo[i] = cap.hi[i] = 0.0f;
    cap.note = note;
    cap.hitId++;
    cap.count = 0;
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
                // the last hit: min / max buckets + raw samples for the spectrogram
                if (scopePos >= 0) {
                    int b = scopePos / SCOPE_BUCKET;
                    if (b < Capture::N) {
                        cap.lo[b] = std::min(cap.lo[b], out);
                        cap.hi[b] = std::max(cap.hi[b], out);
                        if (scopePos < Capture::SAMPLES) cap.samples[scopePos] = out;
                        scopePos++;
                        cap.count = std::min(scopePos, Capture::SAMPLES);
                    } else {
                        scopePos = -1;
                    }
                }
                // live analyzer ring + drive in / out trail
                cap.ring[cap.ringPos] = out;
                cap.ringPos = (cap.ringPos + 1) % Capture::RING;
                if (++cap.trailDiv >= 3) {
                    cap.trailDiv = 0;
                    cap.trailPre[cap.trailPos] = kick.livePre;
                    cap.trailPost[cap.trailPos] = kick.livePost;
                    cap.trailPos = (cap.trailPos + 1) % Capture::TRAIL;
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

    const char* shot = std::getenv("ZIC_SCREENSHOT");
    const bool screenshot = shot != nullptr; // never touch the save file then
    const bool demo = screenshot && std::getenv("ZIC_DEMO");
    const std::string stateFile = Persist::path();
    if (!screenshot) Persist::load(stateFile, panel, seq);
    std::string lastSaved = Persist::serialize(panel, seq);
    if (demo) { // screenshot with a busy patch: every block engaged, so the previews show something
        kick.skew.set(35); kick.sync.set(30); kick.resonator.set(40); kick.fold.set(20);
        kick.bend.set(30); kick.fmDepth.set(25); kick.fmRatio.set(50); kick.shift.set(40);
        kick.noise.set(45); kick.noiseColor.set(65); kick.click.set(60);
        kick.drive.set(60); kick.character.set(45); kick.punch.set(-40); kick.feedback.set(40);
        kick.cutoff.set(60); kick.cutoff2.set(25); kick.reso.set(55); kick.env.set(45); kick.filterFm.set(25); kick.filterDrive.set(40); kick.vowel.set(40);
        kick.decay.set(300); kick.harm2.set(30); kick.harm3.set(-25);
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
    sf::RenderWindow window(sf::VideoMode((unsigned)(EmuUi::W * scale), (unsigned)(EmuUi::H * scale)), "zicModKick32_v4", sf::Style::Default, ctx);
    window.setFramerateLimit(60);
    window.setKeyRepeatEnabled(false);

    EmuUi ui(panel, seq, cap);

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
    int frame = 0;
    bool shotDone = false;
    while (window.isOpen()) {
        sf::Event e;
        while (window.pollEvent(e)) {
            bool shift = sf::Keyboard::isKeyPressed(sf::Keyboard::LShift) || sf::Keyboard::isKeyPressed(sf::Keyboard::RShift);
            switch (e.type) {
            case sf::Event::Closed:
                if (!screenshot || shotDone) window.close(); // a screenshot run ignores a stray close from the window manager
                break;
            case sf::Event::Resized: fitView(e.size.width, e.size.height); break;
            case sf::Event::MouseButtonPressed: ui.mousePressed(logical(e.mouseButton.x, e.mouseButton.y), e.mouseButton.button, shift); break;
            case sf::Event::MouseButtonReleased: ui.mouseReleased(); break;
            case sf::Event::MouseMoved: ui.mouseMoved(logical(e.mouseMove.x, e.mouseMove.y), shift); break;
            case sf::Event::MouseWheelScrolled: ui.wheel(logical(e.mouseWheelScroll.x, e.mouseWheelScroll.y), e.mouseWheelScroll.delta, shift); break;
            case sf::Event::KeyPressed:
                if (e.key.code == sf::Keyboard::Escape) window.close();
                else if (e.key.code == sf::Keyboard::Space) ui.trigDown();
                else if (e.key.code == sf::Keyboard::Return || e.key.code == sf::Keyboard::P) { std::lock_guard<std::mutex> l(audioMutex); seq.toggle(); }
                else ui.key(e.key.code);
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
        if (screenshot) {
            if (demo && (frame == 6 || frame == 50)) kickTrigger = 1; // one hit for the scope, one for the live spectrum
            if (demo && frame == 60) ui.mouseMoved(EmuUi::knobCenter(15), false); // show the help of a pot
            if (++frame == (demo ? 70 : 10)) { // demo: let the whole hit reach the scope and the spectrogram
                sf::Texture tex;
                tex.create(window.getSize().x, window.getSize().y);
                tex.update(window);
                if (!tex.copyToImage().saveToFile(shot)) std::cerr << "could not save " << shot << "\n";
                shotDone = true;
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
