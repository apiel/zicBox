/**
 * zicStudioAcid desktop synthesizer.
 *
 * Full Acid / Bass / Lead standalone studio application with piano roll sequencer.
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

#include "AcidSequencer.h"
#include "EmuUi.h"
#include "Persist.h"
#include "audio/engines/Acid32.h"
#include "core/Acid32Panel.h"

std::atomic<bool> keep_running(true);
std::mutex audioMutex;
std::atomic<int> auditionNote { 0 };
std::atomic<bool> acidMuted { false };

constexpr unsigned int SAMPLE_RATE = 44100;
constexpr size_t NUM_FRAMES = 256;

Acid32 acid(SAMPLE_RATE);
Acid32Panel::Panel panel(acid);
AcidSequencer seq(SAMPLE_RATE);
AcidCapture cap;

void noteOn(int note, float vel = 1.0f, bool slide = false, bool acc = false)
{
    acid.noteOn((uint8_t)std::clamp(note, 0, 127), vel, slide, acc);
}

void noteOff(int note = 0)
{
    acid.noteOff((uint8_t)std::clamp(note, 0, 127));
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
    int lastAudition = 0;  // track previous audition note for noteOff
    while (keep_running) {
        {
            std::lock_guard<std::mutex> lock(audioMutex);
            int audition = auditionNote.exchange(0);
            // Release previous audition note when it changes or clears
            if (lastAudition > 0 && audition != lastAudition) {
                noteOff(lastAudition);
            }
            if (audition > 0) {
                noteOn(audition, 1.0f, false, true);
            }
            lastAudition = audition;

            for (size_t f = 0; f < NUM_FRAMES; ++f) {
                AcidSequencer::TriggerEvent ev = seq.tick();
                if (ev.noteOff) {
                    noteOff();
                }
                if (ev.noteOn && !acidMuted) {
                    noteOn(ev.note, ev.velocity, ev.slide, ev.accent);
                }

                float out = acid.sample();

                // Live analyzer ring
                cap.ring[cap.ringPos] = out;
                cap.ringPos = (cap.ringPos + 1) % AcidCapture::RING;
                cap.livePre = acid.livePre;
                cap.livePost = acid.livePost;
                cap.liveCutoff = acid.liveCutoffHz;
                cap.liveReso = acid.liveReso;
                cap.liveEnvAmp = acid.liveEnvAmp;
                cap.liveEnvFilter = acid.liveEnvFilter;
                cap.liveLfoVal = acid.liveLfoVal;

                int16_t v = (int16_t)(std::clamp(out, -1.0f, 1.0f) * 32767.0f);
                buffer[f * 2] = buffer[f * 2 + 1] = v;
            }
        }
        if (pcm) {
            snd_pcm_sframes_t frames = snd_pcm_writei(pcm, buffer.data(), NUM_FRAMES);
            if (frames < 0) frames = snd_pcm_recover(pcm, frames, 0);
        } else {
            std::this_thread::sleep_for(std::chrono::microseconds(5000));
        }
    }
}

int main(int argc, char** argv)
{
    snd_pcm_t* pcm = audioInit();

    // Load saved settings
    std::string stateFile = Persist::path();
    Persist::load(stateFile, panel, seq);

    std::thread audioThread(audioWorker, pcm);

    sched_param sch_params;
    sch_params.sched_priority = sched_get_priority_max(SCHED_FIFO);
    pthread_setschedparam(audioThread.native_handle(), SCHED_FIFO, &sch_params);

    sf::ContextSettings settings;
    settings.antialiasingLevel = 4;
    sf::RenderWindow window(sf::VideoMode(EmuUi::W, EmuUi::H), "zicStudioAcid", sf::Style::Close | sf::Style::Titlebar, settings);
    window.setFramerateLimit(60);

    EmuUi ui(panel, seq, cap);

    const char* snapEnv = std::getenv("ZIC_SCREENSHOT");

    while (window.isOpen()) {
        sf::Event ev;
        while (window.pollEvent(ev)) {
            if (ev.type == sf::Event::Closed) window.close();
            else if (ev.type == sf::Event::MouseButtonPressed) {
                ui.mousePressed({ (float)ev.mouseButton.x, (float)ev.mouseButton.y }, ev.mouseButton.button, sf::Keyboard::isKeyPressed(sf::Keyboard::LShift));
            } else if (ev.type == sf::Event::MouseButtonReleased) {
                ui.mouseReleased();
            } else if (ev.type == sf::Event::MouseMoved) {
                ui.mouseMoved({ (float)ev.mouseMove.x, (float)ev.mouseMove.y }, sf::Keyboard::isKeyPressed(sf::Keyboard::LShift));
            } else if (ev.type == sf::Event::MouseWheelScrolled) {
                ui.wheel({ (float)ev.mouseWheelScroll.x, (float)ev.mouseWheelScroll.y }, ev.mouseWheelScroll.delta, sf::Keyboard::isKeyPressed(sf::Keyboard::LShift));
            } else if (ev.type == sf::Event::KeyPressed) {
                if (ev.key.code == sf::Keyboard::Space) {
                    seq.toggle();
                } else {
                    ui.key(ev.key.code);
                }
            }
        }

        sf::Vector2i m = sf::Mouse::getPosition(window);
        ui.draw(window, { (float)m.x, (float)m.y });
        window.display();

        if (snapEnv && snapEnv[0]) {
            sf::Texture tex;
            tex.create(window.getSize().x, window.getSize().y);
            tex.update(window);
            tex.copyToImage().saveToFile(snapEnv);
            std::cout << "Saved screenshot to " << snapEnv << "\n";
            window.close();
            break;
        }
    }

    keep_running = false;
    audioThread.join();

    if (pcm) snd_pcm_close(pcm);

    Persist::save(stateFile, Persist::serialize(panel, seq));

    return 0;
}
