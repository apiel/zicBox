#include <SFML/Graphics.hpp>
#include <alsa/asoundlib.h>
#include <atomic>
#include <cmath>
#include <iomanip>
#include <iostream>
#include <mutex>
#include <sstream>
#include <thread>
#include <vector>

#include "audio/engines/DrumKick2.h"
#include "draw/drawMono.h"
#include "helpers/clamp.h"
#include "helpers/enc.h"

// --- Configuration ---
static constexpr uint32_t SAMPLE_RATE = 44100;
// static constexpr uint32_t CHANNEL = 2;
static constexpr uint32_t CHANNEL = 1;
static constexpr int SCREEN_W = 128;
static constexpr int SCREEN_H = 128;
static constexpr int COLS = 3;
static constexpr int ROWS = 4;
static constexpr int CELL_W = SCREEN_W / COLS;
static constexpr int CELL_H = 26; // Height per grid row
static constexpr int PIXEL_SCALE = 2;

// --- Global State ---
std::atomic<bool> keep_running { true };
std::mutex engine_mutex;
DrumKick2 kick2(SAMPLE_RATE);
DrawMono<SCREEN_W, SCREEN_H> display;

std::atomic<float> v_meter { 0.0f };
std::atomic<bool> mc101_connected { false };
std::atomic<int> active_param_idx { -1 }; // Tracks which encoder was last moved

// --- Helper: Format Value ---
std::string formatValue(const Param& p)
{
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

    // Acceleration Tracking
    sf::Clock timer;
    uint32_t lastTicks[12] = { 0 }; // Store last interaction time per parameter

public:
    SFMLEmulator(DrawMono<W, H>& d, int pixelScale)
        : drawer(d)
        , scale(pixelScale)
        , window(sf::VideoMode(W * pixelScale, H * pixelScale), "Kick2 Grid Optim")
    {
        window.setFramerateLimit(60);
        timer.restart();
    }

    bool isOpen() { return window.isOpen(); }

    sf::Vector2i getMousePos()
    {
        sf::Vector2i pos = sf::Mouse::getPosition(window);
        return { pos.x / scale, pos.y / scale };
    }

    void handleEvents()
    {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                window.close();
                keep_running = false;
            }

            if (event.type == sf::Event::MouseWheelScrolled) {
                sf::Vector2i m = getMousePos();
                int col = m.x / CELL_W;
                int row = m.y / CELL_H;
                int index = row * COLS + col;

                if (index >= 0 && index < 12) {
                    active_param_idx = index;
                    uint32_t currentTick = timer.getElapsedTime().asMilliseconds();
                    int8_t rawDir = (event.mouseWheelScroll.delta > 0) ? 1 : -1;
                    int scaledDir = encGetScaledDirection(rawDir, currentTick, lastTicks[index]);
                    lastTicks[index] = currentTick;
                    float amount = (float)scaledDir * kick2.params[index].step;
                    std::lock_guard<std::mutex> lock(engine_mutex);
                    kick2.params[index].set(kick2.params[index].value + amount);
                    // Log for debugging
                    // std::cout << kick2.params[index].label << ": " << formatValue(kick2.params[index]) << std::endl;
                }
            } else if (event.type == sf::Event::KeyPressed) {
                if (event.key.code == sf::Keyboard::A) {
                    kick2.noteOn(60, 1.0f);
                    v_meter = 1.0f;
                }
            }
        }
    }

    void render()
    {
        window.clear(sf::Color::Black);
        sf::RectangleShape pixel(sf::Vector2f(scale, scale));
        for (int y = 0; y < H; y++) {
            for (int x = 0; x < W; x++) {
                if (drawer.getPixel({ x, y })) {
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
void audio_worker(snd_pcm_t* pcm)
{
    // 256 frames * 2 channels = 512 samples
    const size_t num_frames = 256;
    std::vector<int16_t> buffer(num_frames * CHANNEL);

    while (keep_running) {
        {
            std::lock_guard<std::mutex> lock(engine_mutex);
            for (uint32_t i = 0; i < num_frames; i++) {
                // Get one mono sample from engine
                float s = kick2.sample();
                int16_t v16 = static_cast<int16_t>(CLAMP(s, -1.0f, 1.0f) * 32767.0f);
                // int16_t v16 = static_cast<int16_t>(std::max(-1.0f, std::min(1.0f, s)) * 32767.0f);
                if (CHANNEL == 2) {
                    buffer[i * 2] = v16; // Left
                    buffer[i * 2 + 1] = v16; // Right
                } else {
                    buffer[i] = v16;
                }
            }
        }
        // Use writei (interleaved) and pass the number of FRAMES, not samples
        snd_pcm_sframes_t ret = snd_pcm_writei(pcm, buffer.data(), num_frames);
        if (ret < 0) snd_pcm_recover(pcm, (int)ret, 0);
    }
}

void midi_manager()
{
    snd_rawmidi_t* midi_h = nullptr;
    unsigned char buf[32];
    while (keep_running) {
        if (!mc101_connected) {
            int card = -1;
            while (snd_card_next(&card) >= 0 && card >= 0) {
                char* n;
                snd_card_get_name(card, &n);
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
                    if ((buf[i] & 0xF0) == 0x90 && buf[i + 1] == 38 && buf[i + 2] > 0) {
                        float vel = (float)buf[i + 2] / 127.0f;
                        std::lock_guard<std::mutex> lock(engine_mutex);
                        kick2.noteOn(60, vel);
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
int main()
{
    snd_pcm_t* pcm_h;
    if (snd_pcm_open(&pcm_h, "default", SND_PCM_STREAM_PLAYBACK, 0) < 0) return 1;
    snd_pcm_set_params(pcm_h, SND_PCM_FORMAT_S16_LE, SND_PCM_ACCESS_RW_INTERLEAVED, CHANNEL, SAMPLE_RATE, 1, 20000);

    std::thread aThread(audio_worker, pcm_h);
    std::thread mThread(midi_manager);

    SFMLEmulator<SCREEN_W, SCREEN_H> emulator(display, PIXEL_SCALE);

    while (emulator.isOpen() && keep_running) {
        emulator.handleEvents();
        display.clear();

        if (!mc101_connected) {
            display.textCentered({ 64, 55 }, "PLEASE CONNECT", { .font = &PoppinsLight_8 });
            display.textCentered({ 64, 70 }, "THE MC-101...", { .font = &PoppinsLight_8 });
        } else {
            // 1. Draw 3x4 Grid (Labels + Bars only)
            for (int i = 0; i < 12; i++) {
                int col = i % COLS;
                int row = i / COLS;
                int x = col * CELL_W;
                int y = row * CELL_H;
                Param& p = kick2.params[i];

                // Label
                display.text({ x + 3, y + 2 }, p.label, { .font = &PoppinsLight_8, .maxWidth = CELL_W - 6 });

                // Progress Bar Geometry
                int barX = x + 3;
                int barY = y + 14;
                int barW = CELL_W - 6;
                int barH = 4;

                display.rect({ barX, barY }, { barW, barH }); // Outer frame

                // Check for VALUE_CENTERED (min == -max)
                if (p.min == -p.max && p.max != 0) {
                    int midX = barX + (barW / 2);
                    float pct = p.value / p.max; // Result between -1.0 and 1.0
                    int fillW = (int)((barW / 2.0f) * pct);

                    if (fillW > 0) {
                        // Positive: Draw from center to right
                        display.filledRect({ midX, barY }, { fillW, barH });
                    } else if (fillW < 0) {
                        // Negative: Draw from current pos towards center
                        display.filledRect({ midX + fillW, barY }, { -fillW, barH });
                    }
                    // Optional: Small dot or line at the center to mark the zero point
                    // display.setPixel({ midX, barY - 1 });
                } else {
                    // Standard Uni-polar bar (Left to Right)
                    float pct = (p.value - p.min) / (p.max - p.min);
                    display.filledRect({ barX, barY }, { (int)(barW * pct), barH });
                }

                // Grid separators (Your dotted lines)
                if (col < COLS - 1) {
                    for (int dotY = y; dotY < y + CELL_H; dotY += 3)
                        display.setPixel({ x + CELL_W - 1, dotY });
                }
                if (row < ROWS - 1) {
                    for (int dotX = x; dotX < x + CELL_W; dotX += 3)
                        display.setPixel({ dotX, y + CELL_H - 1 });
                }
            }

            // 2. BOTTOM STATUS BAR (The "Focus" area)
            display.line({ 0, 106 }, { 128, 106 }); // Divider

            int active = active_param_idx.load();
            if (active != -1) {
                Param& p = kick2.params[active];
                std::string focusedText = std::to_string(active + 1) + ". " + p.label + ": " + formatValue(p);
                display.text({ 4, 109 }, focusedText, { .font = &PoppinsLight_8 });
            }

            // 3. VELOCITY METER (Slanted at the very bottom)
            float v = v_meter.load();
            if (v > 0.001f) {
                int barW = (int)(v * 120);
                display.filledRect({ 4, 122 }, { barW, 3 });
                v_meter = v * 0.94f;
            }
        }
        emulator.render();
    }

    keep_running = false;
    aThread.join();
    mThread.join();
    snd_pcm_close(pcm_h);
    return 0;
}