/**
 * zicStudioFm4 desktop FM synthesizer.
 *
 * A standalone desktop studio app for the Fm4 engine (audio/engines/Fm4.h):
 * 30 pots on six cards, an algorithm visualizer, LFO, FX sends and a step sequencer.
 */

#include <alsa/asoundlib.h>
#include <algorithm>
#include <atomic>
#include <chrono>
#include <iostream>
#include <mutex>
#include <thread>
#include <vector>

#include "EmuUi.h"
#include <X11/Xlib.h>
#include <X11/extensions/Xrandr.h>
#include "Fm4Sequencer.h"
#include "Persist.h"
#include "audio/engines/Fm4.h"
#include "core/Fm4Panel.h"

std::atomic<bool> keep_running(true);
std::mutex audioMutex;
std::atomic<int> auditionNote { 0 };
std::atomic<bool> fm4Muted { false };

constexpr unsigned int SAMPLE_RATE = 44100;
constexpr size_t NUM_FRAMES = 256;

Fm4 fm4(SAMPLE_RATE);
Fm4Panel::Panel panel(fm4);
Fm4Sequencer seq(SAMPLE_RATE);
Fm4Capture cap;

void noteOn(int note, float vel = 1.0f, bool slide = false)
{
    cap.note = note;
    cap.hitId++;
    fm4.noteOn((uint8_t)std::clamp(note, 0, 127), vel, slide);
}

void noteOff()
{
    fm4.noteOff();
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
    int auditionLeft = 0;
    bool wasPlaying = false;

    while (keep_running) {
        {
            std::lock_guard<std::mutex> lock(audioMutex);

            int note = auditionNote.exchange(0);
            if (note > 0) {
                noteOn(note);
                // When the sequencer runs it owns the gate; otherwise hold the audition briefly.
                auditionLeft = seq.playing ? 0 : (int)(SAMPLE_RATE * 0.45f);
            }

            if (wasPlaying && !seq.playing) noteOff();
            wasPlaying = seq.playing;

            for (size_t f = 0; f < NUM_FRAMES; ++f) {
                if (auditionLeft > 0 && !seq.playing) {
                    if (--auditionLeft == 0) noteOff();
                }

                Fm4Sequencer::TriggerEvent ev = seq.tick();
                if (ev.noteOff) noteOff();
                if (ev.noteOn && !fm4Muted.load()) noteOn(ev.note, ev.velocity, ev.slide);

                float raw = fm4.sample();
                float out = fm4Muted.load() ? 0.0f : raw;

                cap.ring[cap.ringPos] = out;
                cap.ringPos = (cap.ringPos + 1) % Fm4Capture::RING;
                cap.peak = std::max(cap.peak, std::fabs(out));

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

static bool centerWindowOnPrimary(int w, int h, int& ox, int& oy)
{
    Display* d = XOpenDisplay(nullptr);
    if (!d) return false;
    XRRScreenResources* sr = XRRGetScreenResourcesCurrent(d, DefaultRootWindow(d));
    if (!sr) { XCloseDisplay(d); return false; }
    RROutput primary = XRRGetOutputPrimary(d, DefaultRootWindow(d));
    RRCrtc crtc = 0;
    XRROutputInfo* oi = nullptr;
    if (primary) oi = XRRGetOutputInfo(d, sr, primary);
    if (oi && oi->crtc) crtc = oi->crtc;
    if (!crtc) {
        for (int i = 0; i < sr->noutput; ++i) {
            XRROutputInfo* o = XRRGetOutputInfo(d, sr, sr->outputs[i]);
            if (o && o->connection == RR_Connected && o->crtc) { crtc = o->crtc; XRRFreeOutputInfo(o); break; }
            if (o) XRRFreeOutputInfo(o);
        }
    }
    if (!crtc) { if (oi) XRRFreeOutputInfo(oi); XRRFreeScreenResources(sr); XCloseDisplay(d); return false; }
    XRRCrtcInfo* ci = XRRGetCrtcInfo(d, sr, crtc);
    if (!ci) { if (oi) XRRFreeOutputInfo(oi); XRRFreeScreenResources(sr); XCloseDisplay(d); return false; }
    ox = ci->x + (int)(ci->width - w) / 2;
    oy = ci->y + (int)(ci->height - h) / 2;
    XRRFreeCrtcInfo(ci);
    if (oi) XRRFreeOutputInfo(oi);
    XRRFreeScreenResources(sr);
    XCloseDisplay(d);
    return true;
}

int main()
{
    snd_pcm_t* pcm = audioInit();

    std::string stateFile = Persist::path();
    Persist::load(stateFile, panel, seq);

    if (const char* autoPlay = std::getenv("ZIC_AUTOPLAY")) {
        if (autoPlay[0] == '1') seq.play();
    }

    std::thread audioThread(audioWorker, pcm);

    sf::ContextSettings settings;
    settings.antialiasingLevel = 4;
    sf::RenderWindow window(sf::VideoMode((unsigned)EmuUi::W, (unsigned)EmuUi::H), "zicStudioFm4",
                            sf::Style::Close | sf::Style::Titlebar, settings);
    window.setFramerateLimit(60);

    int cx = 0, cy = 0;
    if (centerWindowOnPrimary((int)EmuUi::W, (int)EmuUi::H, cx, cy)) {
        window.setPosition(sf::Vector2i(cx, cy));
    }

    EmuUi ui(panel, seq, cap);

    const char* snapEnv = std::getenv("ZIC_SCREENSHOT");

    while (window.isOpen()) {
        sf::Event ev;
        while (window.pollEvent(ev)) {
            if (ev.type == sf::Event::Closed) window.close();
            else if (ev.type == sf::Event::MouseButtonPressed) {
                ui.mousePressed({ (float)ev.mouseButton.x, (float)ev.mouseButton.y }, ev.mouseButton.button,
                                sf::Keyboard::isKeyPressed(sf::Keyboard::LShift));
            } else if (ev.type == sf::Event::MouseButtonReleased) {
                ui.mouseReleased();
            } else if (ev.type == sf::Event::MouseMoved) {
                ui.mouseMoved({ (float)ev.mouseMove.x, (float)ev.mouseMove.y }, sf::Keyboard::isKeyPressed(sf::Keyboard::LShift));
            } else if (ev.type == sf::Event::MouseWheelScrolled) {
                ui.wheel({ (float)ev.mouseWheelScroll.x, (float)ev.mouseWheelScroll.y }, ev.mouseWheelScroll.delta,
                         sf::Keyboard::isKeyPressed(sf::Keyboard::LShift));
            } else if (ev.type == sf::Event::KeyPressed) {
                ui.key(ev.key.code);
            }
        }

        ui.draw(window, sf::Vector2f(sf::Mouse::getPosition(window)));
        window.display();

        if (snapEnv && snapEnv[0]) {
            static int snapFrames = 0;
            if (++snapFrames >= 12) {
                sf::Texture tex;
                tex.create(window.getSize().x, window.getSize().y);
                tex.update(window);
                tex.copyToImage().saveToFile(snapEnv);
                std::cout << "Saved screenshot to " << snapEnv << "\n";
                window.close();
                break;
            }
        }
    }

    keep_running = false;
    audioThread.join();
    if (pcm) snd_pcm_close(pcm);
    Persist::save(stateFile, Persist::serialize(panel, seq));
    return 0;
}
