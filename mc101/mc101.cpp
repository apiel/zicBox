#include <alsa/asoundlib.h>
#include <SFML/Graphics.hpp>
#include <iostream>
#include <vector>
#include <cmath>
#include <atomic>
#include <thread>
#include <mutex>
#include <iomanip>
#include <sstream>

#include "audio/engines/DrumKick2.h"
#include "draw/drawMono.h"

// --- Configuration ---
static constexpr uint32_t SAMPLE_RATE = 44100;
static constexpr int SCREEN_W = 128;
static constexpr int SCREEN_H = 128;
static constexpr int COLS = 3;
static constexpr int ROWS = 4;
static constexpr int CELL_W = SCREEN_W / COLS;
static constexpr int CELL_H = SCREEN_H / ROWS;

std::atomic<bool> keep_running{true};
std::mutex engine_mutex;
DrumKick2 kick2(SAMPLE_RATE);
DrawMono<SCREEN_W, SCREEN_H> display;

// --- Helper: Format Float to String based on Precision ---
std::string formatValue(const Param& p) {
    std::fixed;
    std::stringstream ss;
    ss << std::fixed << std::setprecision(p.precision) << p.value;
    return ss.str();
}

// --- SFML Emulator with Mouse Wheel Encoder Simulation ---
class SFMLEmulator {
    int scale;
    sf::RenderWindow window;
    DrawMono<SCREEN_W, SCREEN_H>& drawer;

public:
    SFMLEmulator(DrawMono<SCREEN_W, SCREEN_H>& d, int pixelScale = 3) 
        : drawer(d), scale(pixelScale),
          window(sf::VideoMode(SCREEN_W * pixelScale, SCREEN_H * pixelScale), "Kick2 Controller") {
        window.setFramerateLimit(60);
    }

    bool isOpen() { return window.isOpen(); }

    void handleEvents() {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) window.close();

            // Simulate Encoders with Mouse Wheel
            if (event.type == sf::Event::MouseWheelScrolled) {
                int mx = event.mouseWheelScroll.x / scale;
                int my = event.mouseWheelScroll.y / scale;

                // Determine which cell (0-11) the mouse is over
                int col = mx / CELL_W;
                int row = my / CELL_H;
                int index = row * COLS + col;

                if (index >= 0 && index < 12) {
                    float delta = event.mouseWheelScroll.delta * kick2.params[index].step;
                    std::lock_guard<std::mutex> lock(engine_mutex);
                    kick2.params[index].set(kick2.params[index].value + delta);
                }
            }
        }
    }

    void render() {
        window.clear(sf::Color(10, 10, 10));
        sf::RectangleShape pixel(sf::Vector2f(scale - 1, scale - 1));
        for (int y = 0; y < SCREEN_H; y++) {
            for (int x = 0; x < SCREEN_W; x++) {
                if (drawer.getPixel({x, y})) {
                    pixel.setPosition(x * scale, y * scale);
                    pixel.setFillColor(sf::Color::White);
                    window.draw(pixel);
                }
            }
        }
        window.display();
    }
};

// --- AUDIO WORKER (Same as before) ---
void audio_worker(snd_pcm_t* pcm) {
    std::vector<int16_t> buffer(256);
    while (keep_running) {
        {
            std::lock_guard<std::mutex> lock(engine_mutex);
            for (uint32_t i = 0; i < 256; ++i) {
                float s = kick2.sample() * 0.8f; // Headroom
                s = std::max(-1.0f, std::min(1.0f, s));
                buffer[i] = static_cast<int16_t>(s * 32767.0f);
            }
        }
        snd_pcm_sframes_t ret = snd_pcm_writei(pcm, buffer.data(), 256);
        if (ret < 0) snd_pcm_recover(pcm, (int)ret, 0);
    }
}

// --- MIDI WORKER ---
void midi_worker(snd_rawmidi_t* midi) {
    unsigned char buf[32];
    while (keep_running) {
        ssize_t n = snd_rawmidi_read(midi, buf, sizeof(buf));
        if (n <= 0) continue;
        for (int i = 0; i < n; i++) {
            if ((buf[i] & 0xF0) == 0x90 && buf[i+2] > 0) {
                if (buf[i+1] == 38) {
                    std::lock_guard<std::mutex> lock(engine_mutex);
                    kick2.noteOn(buf[i+1], (float)buf[i+2] / 127.0f);
                }
                i += 2;
            }
        }
    }
}

int main() {
    // 1. MIDI Init
    int card = -1; std::string midi_id = "";
    while (snd_card_next(&card) >= 0 && card >= 0) {
        char* n; snd_card_get_name(card, &n);
        if (std::string(n).find("MC-101") != std::string::npos) midi_id = "hw:" + std::to_string(card) + ",0";
        free(n);
    }

    // 2. Audio/MIDI Opening
    snd_pcm_t* pcm_h; snd_rawmidi_t* midi_h = nullptr;
    if (snd_pcm_open(&pcm_h, "default", SND_PCM_STREAM_PLAYBACK, 0) < 0) return 1;
    snd_pcm_set_params(pcm_h, SND_PCM_FORMAT_S16_LE, SND_PCM_ACCESS_RW_INTERLEAVED, 1, SAMPLE_RATE, 1, 20000);
    
    bool has_midi = (midi_id != "" && snd_rawmidi_open(&midi_h, nullptr, midi_id.c_str(), 0) >= 0);

    std::thread aThread(audio_worker, pcm_h);
    std::thread mThread;
    if (has_midi) mThread = std::thread(midi_worker, midi_h);

    SFMLEmulator emulator(display);

    // 3. UI / Main Loop
    while (emulator.isOpen()) {
        emulator.handleEvents();
        display.clear();

        if (!has_midi) {
            display.textCentered({64, 60}, "PLEASE CONNECT", {.font = &PoppinsLight_8});
            display.textCentered({64, 72}, "THE MC-101", {.font = &PoppinsLight_8});
        } else {
            // Draw 3x4 Grid of Parameters
            for (int i = 0; i < 12; i++) {
                int col = i % COLS;
                int row = i / COLS;
                int x = col * CELL_W;
                int y = row * CELL_H;

                Param& p = kick2.params[i];

                // Label (Small font)
                display.text({x + 2, y + 2}, p.label, {.font = &PoppinsLight_6});
                
                // Value (Bold/Mid font)
                display.text({x + 2, y + 12}, formatValue(p), {.font = &PoppinsLight_8});

                // Small Visual Bar
                float pct = (p.value - p.min) / (p.max - p.min);
                display.rect({x + 2, y + 24}, {CELL_W - 4, 4});
                display.filledRect({x + 2, y + 24}, {(int)((CELL_W - 4) * pct), 4});

                // Grid Lines
                if (col < COLS - 1) display.line({x + CELL_W - 1, y}, {x + CELL_W - 1, y + CELL_H});
                if (row < ROWS - 1) display.line({x, y + CELL_H - 1}, {x + CELL_W, y + CELL_H - 1});
            }
        }

        emulator.render();
    }

    keep_running = false;
    aThread.join();
    if (has_midi) mThread.join();
    snd_pcm_close(pcm_h);
    if (has_midi) snd_rawmidi_close(midi_h);

    return 0;
}