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
static constexpr int CELL_H = 26; // Height per grid row
static constexpr int PIXEL_SCALE = 2; 

// --- Global State ---
std::atomic<bool> keep_running{true};
std::mutex engine_mutex;
DrumKick2 kick2(SAMPLE_RATE);
DrawMono<SCREEN_W, SCREEN_H> display;

std::atomic<float> v_meter{0.0f}; 
std::atomic<bool> mc101_connected{false};
std::atomic<int> active_param_idx{-1}; // Tracks which encoder was last moved

// --- Helper: Format Value ---
std::string formatValue(const Param& p) {
    std::stringstream ss;
    ss << std::fixed << std::setprecision(p.precision) << p.value;
    if (p.unit) ss << " " << p.unit;
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
          window(sf::VideoMode(W * pixelScale, H * pixelScale), "Kick2 Grid Optim") {
        window.setFramerateLimit(60);
    }

    bool isOpen() { return window.isOpen(); }

    sf::Vector2i getMousePos() {
        sf::Vector2i pos = sf::Mouse::getPosition(window);
        return { pos.x / scale, pos.y / scale };
    }

    void handleEvents() {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) { window.close(); keep_running = false; }

            if (event.type == sf::Event::MouseWheelScrolled) {
                sf::Vector2i m = getMousePos();
                int col = m.x / CELL_W;
                int row = m.y / CELL_H;
                int index = row * COLS + col;

                if (index >= 0 && index < 12) {
                    active_param_idx = index; // Set this as the active parameter
                    float delta = event.mouseWheelScroll.delta * kick2.params[index].step;
                    std::lock_guard<std::mutex> lock(engine_mutex);
                    kick2.params[index].set(kick2.params[index].value + delta);
                }
            }
        }
    }

    void render() {
        window.clear(sf::Color::Black);
        sf::RectangleShape pixel(sf::Vector2f(scale, scale));
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

// --- ALSA Workers ---
void audio_worker(snd_pcm_t* pcm) {
    std::vector<int16_t> buffer(256);
    while (keep_running) {
        {
            std::lock_guard<std::mutex> lock(engine_mutex);
            for (uint32_t i = 0; i < 256; ++i) {
                float s = kick2.sample() * 0.7f;
                buffer[i] = static_cast<int16_t>(std::max(-1.0f, std::min(1.0f, s)) * 32767.0f);
            }
        }
        snd_pcm_sframes_t ret = snd_pcm_writei(pcm, buffer.data(), 256);
        if (ret < 0) snd_pcm_recover(pcm, (int)ret, 0);
    }
}

void midi_manager() {
    snd_rawmidi_t* midi_h = nullptr;
    unsigned char buf[32];
    while (keep_running) {
        if (!mc101_connected) {
            int card = -1;
            while (snd_card_next(&card) >= 0 && card >= 0) {
                char* n; snd_card_get_name(card, &n);
                if (n && std::string(n).find("MC-101") != std::string::npos) {
                    if (snd_rawmidi_open(&midi_h, nullptr, ("hw:" + std::to_string(card) + ",0").c_str(), SND_RAWMIDI_NONBLOCK) >= 0)
                        mc101_connected = true;
                }
                free(n);
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
        } else {
            ssize_t n = snd_rawmidi_read(midi_h, buf, sizeof(buf));
            if (n > 0) {
                for (int i = 0; i < n; i++) {
                    if ((buf[i] & 0xF0) == 0x90 && buf[i+1] == 38 && buf[i+2] > 0) {
                        float vel = (float)buf[i+2] / 127.0f;
                        std::lock_guard<std::mutex> lock(engine_mutex);
                        kick2.noteOn(38, vel);
                        v_meter = vel;
                    }
                }
            } else if (n < 0 && n != -EAGAIN) {
                snd_rawmidi_close(midi_h);
                mc101_connected = false;
            }
            std::this_thread::sleep_for(std::chrono::microseconds(200));
        }
    }
}

// --- MAIN ---
int main() {
    snd_pcm_t* pcm_h;
    if (snd_pcm_open(&pcm_h, "default", SND_PCM_STREAM_PLAYBACK, 0) < 0) return 1;
    snd_pcm_set_params(pcm_h, SND_PCM_FORMAT_S16_LE, SND_PCM_ACCESS_RW_INTERLEAVED, 1, SAMPLE_RATE, 1, 20000);

    std::thread aThread(audio_worker, pcm_h);
    std::thread mThread(midi_manager);

    SFMLEmulator<SCREEN_W, SCREEN_H> emulator(display, PIXEL_SCALE);

    while (emulator.isOpen() && keep_running) {
        emulator.handleEvents();
        display.clear();

        if (!mc101_connected) {
            display.textCentered({64, 64}, "WAITING FOR MC-101", {.font = &PoppinsLight_8});
        } else {
            // 1. Draw 3x4 Grid (Labels + Bars only)
            for (int i = 0; i < 12; i++) {
                int col = i % COLS;
                int row = i / COLS;
                int x = col * CELL_W;
                int y = row * CELL_H;
                Param& p = kick2.params[i];

                // Label (High readability)
                display.text({x + 3, y + 2}, p.label, {.font = &PoppinsLight_8, .maxWidth = CELL_W - 6});
                
                // Progress Bar
                float pct = (p.value - p.min) / (p.max - p.min);
                display.rect({x + 3, y + 14}, {CELL_W - 6, 4});
                display.filledRect({x + 3, y + 14}, {(int)((CELL_W - 6) * pct), 4});
                
                // Separators
                if (col < COLS - 1) display.line({x + CELL_W - 1, y}, {x + CELL_W - 1, y + CELL_H - 2});
            }

            // 2. BOTTOM STATUS BAR (The "Focus" area)
            display.line({0, 106}, {128, 106}); // Divider
            
            int active = active_param_idx.load();
            if (active != -1) {
                Param& p = kick2.params[active];
                std::string focusedText = std::to_string(active + 1) + ". " + p.label + ": " + formatValue(p);
                display.text({4, 109}, focusedText, {.font = &PoppinsLight_8});
            }

            // 3. VELOCITY METER (Slanted at the very bottom)
            float v = v_meter.load();
            if (v > 0.001f) {
                int barW = (int)(v * 120);
                display.filledRect({4, 122}, {barW, 3});
                v_meter = v * 0.94f; 
            }
        }
        emulator.render();
    }

    keep_running = false;
    aThread.join(); mThread.join();
    snd_pcm_close(pcm_h);
    return 0;
}