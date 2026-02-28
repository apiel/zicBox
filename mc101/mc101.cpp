#include <alsa/asoundlib.h>
#include <SFML/Graphics.hpp>
#include <iostream>
#include <vector>
#include <cmath>
#include <atomic>
#include <thread>
#include <mutex>

// Your custom headers
#include "audio/engines/DrumKick2.h"
#include "draw/drawMono.h"

// --- Configuration ---
static constexpr uint32_t SAMPLE_RATE = 44100;
static constexpr uint32_t CHUNK_SIZE = 256;
static constexpr int SCREEN_W = 128;
static constexpr int SCREEN_H = 128;

// --- Global State ---
std::atomic<bool> keep_running{true};
std::mutex engine_mutex;
DrumKick2 kick2(SAMPLE_RATE);
DrawMono<SCREEN_W, SCREEN_H> display;

// Statistics for the UI
std::atomic<int> last_midi_note{0};
std::atomic<float> last_velocity{0.0f};

// --- SFML Emulator Class ---
class SFMLEmulator {
    int scale;
    sf::RenderWindow window;
    DrawMono<SCREEN_W, SCREEN_H>& drawer;

public:
    SFMLEmulator(DrawMono<SCREEN_W, SCREEN_H>& d, int pixelScale = 4) 
        : drawer(d), scale(pixelScale),
          window(sf::VideoMode(SCREEN_W * pixelScale, SCREEN_H * pixelScale), "SH1107 Emulator") {
        window.setFramerateLimit(60);
    }

    bool isOpen() { return window.isOpen(); }

    void update() {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) window.close();
        }

        window.clear(sf::Color(20, 20, 25)); // Deep blue-grey background

        sf::RectangleShape pixel(sf::Vector2f(scale - 1, scale - 1));
        pixel.setFillColor(sf::Color::White);

        for (int y = 0; y < SCREEN_H; y++) {
            for (int x = 0; x < SCREEN_W; x++) {
                if (drawer.getPixel({x, y})) {
                    pixel.setPosition(x * scale, y * scale);
                    window.draw(pixel);
                }
            }
        }
        window.display();
    }
};

// --- AUDIO THREAD ---
void audio_worker(snd_pcm_t* pcm) {
    std::vector<int16_t> buffer(CHUNK_SIZE);
    while (keep_running) {
        {
            std::lock_guard<std::mutex> lock(engine_mutex);
            for (uint32_t i = 0; i < CHUNK_SIZE; ++i) {
                float s = kick2.sample();
                // Clamp and convert to S16_LE
                s = std::max(-1.0f, std::min(1.0f, s));
                buffer[i] = static_cast<int16_t>(s * 32767.0f);
            }
        }
        snd_pcm_sframes_t ret = snd_pcm_writei(pcm, buffer.data(), CHUNK_SIZE);
        if (ret < 0) snd_pcm_recover(pcm, (int)ret, 0);
    }
}

// --- MIDI THREAD ---
void midi_worker(snd_rawmidi_t* midi) {
    unsigned char buf[32];
    while (keep_running) {
        ssize_t n = snd_rawmidi_read(midi, buf, sizeof(buf));
        if (n <= 0) continue;

        for (int i = 0; i < n; i++) {
            if ((buf[i] & 0xF0) == 0x90) { // Note On
                uint8_t note = buf[i+1];
                float vel = (float)buf[i+2] / 127.0f;
                
                if (note == 38 && vel > 0) {
                    std::lock_guard<std::mutex> lock(engine_mutex);
                    kick2.noteOn(note, vel);
                    last_midi_note = note;
                    last_velocity = vel;
                }
                i += 2;
            }
        }
    }
}

// --- HELPER: FIND DEVICE ---
std::string findMC101() {
    int card = -1;
    while (snd_card_next(&card) >= 0 && card >= 0) {
        char* name;
        if (snd_card_get_name(card, &name) == 0) {
            std::string s(name); free(name);
            if (s.find("MC-101") != std::string::npos) return "hw:" + std::to_string(card) + ",0";
        }
    }
    return "";
}

// --- MAIN (UI THREAD) ---
int main() {
    // 1. Hardware Discovery
    std::string midi_id = findMC101();
    if (midi_id.empty()) { std::cerr << "MC-101 Not Found\n"; return 1; }

    // 2. ALSA Initialization
    snd_pcm_t* pcm_handle;
    snd_rawmidi_t* midi_handle;
    
    if (snd_pcm_open(&pcm_handle, "default", SND_PCM_STREAM_PLAYBACK, 0) < 0) return 1;
    snd_pcm_set_params(pcm_handle, SND_PCM_FORMAT_S16_LE, SND_PCM_ACCESS_RW_INTERLEAVED, 1, SAMPLE_RATE, 1, 20000);
    
    if (snd_rawmidi_open(&midi_handle, nullptr, midi_id.c_str(), 0) < 0) return 1;

    // 3. Launch Workers
    std::thread aThread(audio_worker, pcm_handle);
    std::thread mThread(midi_worker, midi_handle);
    SFMLEmulator emulator(display, 3);

    // 4. UI Loop
    while (emulator.isOpen()) {
        display.clear();
        
        // Header
        display.rect({0, 0}, {128, 16});
        display.text({4, 3}, "KICK2 SYNTH v1.0");

        // Info
        display.text({5, 30}, "DRUM PAD: 38");
        display.text({5, 45}, "STATUS: CONNECTED");
        
        // Visualizer
        display.text({5, 70}, "TRIGGER INFO:");
        std::string info = "N:" + std::to_string(last_midi_note.load()) + " V:" + std::to_string((int)(last_velocity * 100)) + "%";
        display.text({15, 85}, info);

        // Scope / Activity Bar
        int volBar = (int)(last_velocity * 120);
        display.filledRect({4, 110}, {volBar, 8});
        
        // Decay visualizer (example)
        if (last_velocity > 0) last_velocity = last_velocity * 0.95f; // Visual decay only

        emulator.update();
    }

    // 5. Cleanup
    keep_running = false;
    aThread.join();
    mThread.detach(); // RawMIDI read might be blocking, safer to detach or use non-blocking
    
    snd_pcm_close(pcm_handle);
    snd_rawmidi_close(midi_handle);
    
    return 0;
}