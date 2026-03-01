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

// Assuming these are in your include path based on your previous snippets
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
static constexpr int PIXEL_SCALE = 2; // Global UI Scale for desktop

// --- Global State ---
std::atomic<bool> keep_running{true};
std::mutex engine_mutex;
DrumKick2 kick2(SAMPLE_RATE);
DrawMono<SCREEN_W, SCREEN_H> display;

// Shared state for MIDI/UI
std::atomic<float> v_meter{0.0f}; 
std::atomic<bool> mc101_connected{false};

// --- Helper: Format Value ---
std::string formatValue(const Param& p) {
    std::stringstream ss;
    ss << std::fixed << std::setprecision(p.precision) << p.value;
    return ss.str();
}

// --- SFML Emulator Class ---
template <int W, int H>
class SFMLEmulator {
private:
    int scale;
    sf::RenderWindow window;
    DrawMono<W, H>& drawer;

public:
    SFMLEmulator(DrawMono<W, H>& d, int pixelScale) 
        : drawer(d), scale(pixelScale),
          window(sf::VideoMode(W * pixelScale, H * pixelScale), "Kick2 Emulator (MC-101)") {
        window.setFramerateLimit(60);
    }

    bool isOpen() { return window.isOpen(); }

    void handleEvents() {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                window.close();
                keep_running = false;
            }

            // Mouse Wheel Encoder Simulation
            if (event.type == sf::Event::MouseWheelScrolled) {
                // Convert mouse position to grid coordinates
                int mx = event.mouseWheelScroll.x / scale;
                int my = event.mouseWheelScroll.y / scale;

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
        window.clear(sf::Color(20, 20, 25));
        sf::RectangleShape pixel(sf::Vector2f(scale - 1, scale - 1));
        
        for (int y = 0; y < H; y++) {
            for (int x = 0; x < W; x++) {
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

// --- AUDIO WORKER ---
void audio_worker(snd_pcm_t* pcm) {
    std::vector<int16_t> buffer(256);
    while (keep_running) {
        {
            std::lock_guard<std::mutex> lock(engine_mutex);
            for (uint32_t i = 0; i < 256; ++i) {
                float s = kick2.sample() * 0.7f; // Master gain
                s = std::max(-1.0f, std::min(1.0f, s));
                buffer[i] = static_cast<int16_t>(s * 32767.0f);
            }
        }
        snd_pcm_sframes_t ret = snd_pcm_writei(pcm, buffer.data(), 256);
        if (ret < 0) snd_pcm_recover(pcm, (int)ret, 0);
    }
}

// --- MIDI & RECONNECT WORKER ---
void midi_manager() {
    snd_rawmidi_t* midi_h = nullptr;
    unsigned char buf[32];

    while (keep_running) {
        if (!mc101_connected) {
            int card = -1;
            std::string midi_id = "";
            while (snd_card_next(&card) >= 0 && card >= 0) {
                char* n;
                if (snd_card_get_name(card, &n) == 0) {
                    if (n && std::string(n).find("MC-101") != std::string::npos) {
                        midi_id = "hw:" + std::to_string(card) + ",0";
                    }
                    free(n);
                }
            }

            if (!midi_id.empty() && snd_rawmidi_open(&midi_h, nullptr, midi_id.c_str(), SND_RAWMIDI_NONBLOCK) >= 0) {
                mc101_connected = true;
            } else {
                std::this_thread::sleep_for(std::chrono::milliseconds(500));
            }
        } else {
            ssize_t n = snd_rawmidi_read(midi_h, buf, sizeof(buf));
            if (n > 0) {
                for (int i = 0; i < n; i++) {
                    // Check Note On (0x90) for Note 38
                    if ((buf[i] & 0xF0) == 0x90 && buf[i+1] == 38 && buf[i+2] > 0) {
                        float vel = (float)buf[i+2] / 127.0f;
                        std::lock_guard<std::mutex> lock(engine_mutex);
                        kick2.noteOn(38, vel);
                        v_meter = vel; 
                        i += 2;
                    }
                }
            } else if (n < 0 && n != -EAGAIN) {
                snd_rawmidi_close(midi_h);
                mc101_connected = false;
            }
            std::this_thread::sleep_for(std::chrono::microseconds(500));
        }
    }
    if (midi_h) snd_rawmidi_close(midi_h);
}

// --- MAIN ---
int main() {
    snd_pcm_t* pcm_h;
    if (snd_pcm_open(&pcm_h, "default", SND_PCM_STREAM_PLAYBACK, 0) < 0) {
        std::cerr << "Audio error" << std::endl;
        return 1;
    }
    snd_pcm_set_params(pcm_h, SND_PCM_FORMAT_S16_LE, SND_PCM_ACCESS_RW_INTERLEAVED, 1, SAMPLE_RATE, 1, 20000);

    std::thread aThread(audio_worker, pcm_h);
    std::thread mThread(midi_manager);

    SFMLEmulator<SCREEN_W, SCREEN_H> emulator(display, PIXEL_SCALE);

    while (emulator.isOpen() && keep_running) {
        emulator.handleEvents();
        display.clear();

        if (!mc101_connected) {
            display.textCentered({64, 55}, "PLEASE CONNECT", {.font = &PoppinsLight_8});
            display.textCentered({64, 70}, "THE MC-101...", {.font = &PoppinsLight_8});
        } else {
            // Draw 3x4 Grid
            for (int i = 0; i < 12; i++) {
                int col = i % COLS;
                int row = i / COLS;
                int x = col * CELL_W;
                int y = row * CELL_H;
                Param& p = kick2.params[i];

                display.text({x + 2, y + 2}, p.label, {.font = &PoppinsLight_8});
                display.text({x + 2, y + 13}, formatValue(p), {.font = &PoppinsLight_8});
                
                float pct = (p.value - p.min) / (p.max - p.min);
                display.filledRect({x + 2, y + 26}, {(int)((CELL_W - 4) * pct), 3});
                
                // Grid separators
                if (col < COLS - 1) display.line({x + CELL_W - 1, y}, {x + CELL_W - 1, y + CELL_H});
                if (row < ROWS - 1) display.line({x, y + CELL_H - 1}, {x + CELL_W, y + CELL_H - 1});
            }

            // Velocity Meter (Horizontal at the bottom)
            float current_v = v_meter.load();
            if (current_v > 0.001f) {
                int barW = (int)(current_v * 120);
                display.filledRect({4, 124}, {barW, 3});
                v_meter = current_v * 0.94f; // Decay speed
            }
        }
        emulator.render();
    }

    keep_running = false;
    if (aThread.joinable()) aThread.join();
    if (mThread.joinable()) mThread.join();
    snd_pcm_close(pcm_h);
    return 0;
}