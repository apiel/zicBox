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
static constexpr uint32_t CHANNEL = 2; 
static constexpr int SCREEN_W = 128;
static constexpr int SCREEN_H = 128;
static constexpr int COLS = 3;
static constexpr int ROWS = 4;
static constexpr int CELL_W = SCREEN_W / COLS;
static constexpr int CELL_H = 26; 
static constexpr int PIXEL_SCALE = 2;

// --- Global State ---
std::atomic<bool> keep_running { true };
std::mutex engine_mutex;
DrumKick2 kick2(SAMPLE_RATE);
DrawMono<SCREEN_W, SCREEN_H> display;

// --- Shift Layer Parameters ---
// You can fill this with up to 12 parameters. Empty labels won't be drawn.
Param shiftParams[12] = {
    { "Master Vol", "%", .value = 100.0f },
    { "" },
    { "" },
    { "" },
    { "" },
    { "" },
    { "" },
    { "" },
    { "" },
    { "" },
    { "" },
    { "" },
};

std::atomic<float> v_meter { 0.0f };
std::atomic<bool> mc101_connected { false };
std::atomic<bool> is_shift_pressed { false };
std::atomic<int> active_param_idx { -1 }; 
std::atomic<int> active_shift_idx { -1 };

// --- Helper: Format Value ---
std::string formatValue(const Param& p)
{
    std::stringstream ss;
    // Special case for % units if you want to show 0.7 as 70%
    ss << std::fixed << std::setprecision(p.precision) << p.value;
    if (p.unit != "") ss << " " << p.unit;
    return ss.str();
}

// --- SFML Emulator Class ---
template <int W, int H>
class SFMLEmulator {
private:
    int scale;
    sf::RenderWindow window;
    DrawMono<W, H>& drawer;
    sf::Clock timer;
    uint32_t lastTicks[12] = { 0 }; 
    uint32_t lastShiftTicks[12] = { 0 };

public:
    SFMLEmulator(DrawMono<W, H>& d, int pixelScale)
        : drawer(d), scale(pixelScale), window(sf::VideoMode(W * pixelScale, H * pixelScale), "Kick2 Grid Optim")
    {
        window.setFramerateLimit(60);
        timer.restart();
    }

    bool isOpen() { return window.isOpen(); }

    void handleEvents()
    {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) { window.close(); keep_running = false; }

            if (event.type == sf::Event::KeyPressed) {
                if (event.key.code == sf::Keyboard::S) is_shift_pressed = true;
                if (event.key.code == sf::Keyboard::A) {
                    std::lock_guard<std::mutex> lock(engine_mutex);
                    kick2.noteOn(60, 1.0f);
                    v_meter = 1.0f;
                }
            } else if (event.type == sf::Event::KeyReleased) {
                if (event.key.code == sf::Keyboard::S) is_shift_pressed = false;
            }

            if (event.type == sf::Event::MouseWheelScrolled) {
                sf::Vector2i m = { event.mouseWheelScroll.x / scale, event.mouseWheelScroll.y / scale };
                int col = m.x / CELL_W;
                int row = m.y / CELL_H;
                int index = row * COLS + col;
                if (index < 0 || index >= 12) continue;

                uint32_t currentTick = timer.getElapsedTime().asMilliseconds();
                int8_t rawDir = (event.mouseWheelScroll.delta > 0) ? 1 : -1;

                if (is_shift_pressed) {
                    // Edit Shift Layer
                    if (shiftParams[index].label != "") {
                        active_shift_idx = index;
                        int scaledDir = encGetScaledDirection(rawDir, currentTick, lastShiftTicks[index]);
                        lastShiftTicks[index] = currentTick;
                        shiftParams[index].set(shiftParams[index].value + (scaledDir * shiftParams[index].step));
                    }
                } else {
                    // Edit Main Layer
                    active_param_idx = index;
                    int scaledDir = encGetScaledDirection(rawDir, currentTick, lastTicks[index]);
                    lastTicks[index] = currentTick;
                    std::lock_guard<std::mutex> lock(engine_mutex);
                    kick2.params[index].set(kick2.params[index].value + (scaledDir * kick2.params[index].step));
                }
            }
        }
    }

    void render() {
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
    const size_t num_frames = 256;
    std::vector<int16_t> buffer(num_frames * CHANNEL);
    while (keep_running) {
        {
            std::lock_guard<std::mutex> lock(engine_mutex);
            float masterVol = shiftParams[0].value * 0.01f; // Using first Shift Param as Master Vol
            for (uint32_t i = 0; i < num_frames; i++) {
                float s = kick2.sample() * masterVol;
                int16_t v16 = static_cast<int16_t>(CLAMP(s, -1.0f, 1.0f) * 32767.0f);
                if (CHANNEL == 2) { buffer[i * 2] = v16; buffer[i * 2 + 1] = v16; }
                else { buffer[i] = v16; }
            }
        }
        snd_pcm_sframes_t ret = snd_pcm_writei(pcm, buffer.data(), num_frames);
        if (ret < 0) snd_pcm_recover(pcm, (int)ret, 0);
    }
}

// ... midi_manager code remains same as previous ...

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

int main()
{
    snd_pcm_t* pcm_h;
    if (snd_pcm_open(&pcm_h, "default", SND_PCM_STREAM_PLAYBACK, 0) < 0) return 1;
    snd_pcm_set_params(pcm_h, SND_PCM_FORMAT_S16_LE, SND_PCM_ACCESS_RW_INTERLEAVED, CHANNEL, SAMPLE_RATE, 1, 20000);

    std::thread aThread(audio_worker, pcm_h);
    std::thread mThread(midi_manager);

    for (int i = 0; i < 12; i++) {
        shiftParams[i].finalize();
    }

    SFMLEmulator<SCREEN_W, SCREEN_H> emulator(display, PIXEL_SCALE);

    while (emulator.isOpen() && keep_running) {
        emulator.handleEvents();
        display.clear();

        if (!mc101_connected) {
            display.textCentered({ 64, 64 }, "CONNECT MC-101", { .font = &PoppinsLight_8 });
        } else {
            // Pick the parameter set based on Shift state
            Param* currentSet = is_shift_pressed ? shiftParams : kick2.params;

            // 1. Draw 3x4 Grid
            for (int i = 0; i < 12; i++) {
                int col = i % COLS;
                int row = i / COLS;
                int x = col * CELL_W;
                int y = row * CELL_H;
                Param& p = currentSet[i];

                if (p.label == "") continue;

                display.text({ x + 3, y + 2 }, p.label, { .font = &PoppinsLight_8, .maxWidth = CELL_W - 6 });

                int barX = x + 3, barY = y + 14, barW = CELL_W - 6, barH = 4;
                display.rect({ barX, barY }, { barW, barH }); 

                if (p.min == -p.max && p.max != 0) {
                    int midX = barX + (barW / 2);
                    float pct = p.value / p.max;
                    int fillW = (int)((barW / 2.0f) * pct);
                    if (fillW > 0) display.filledRect({ midX, barY }, { fillW, barH });
                    else if (fillW < 0) display.filledRect({ midX + fillW, barY }, { -fillW, barH });
                } else {
                    float pct = (p.value - p.min) / (p.max - p.min);
                    display.filledRect({ barX, barY }, { (int)(barW * pct), barH });
                }

                // Dotted separators
                if (col < COLS - 1) for (int dotY = y; dotY < y + CELL_H; dotY += 3) display.setPixel({ x + CELL_W - 1, dotY });
                if (row < ROWS - 1) for (int dotX = x; dotX < x + CELL_W; dotX += 3) display.setPixel({ dotX, y + CELL_H - 1 });
            }

            // 2. BOTTOM STATUS BAR
            display.line({ 0, 106 }, { 128, 106 }); 
            int activeIdx = is_shift_pressed ? active_shift_idx.load() : active_param_idx.load();
            
            if (activeIdx != -1 && currentSet[activeIdx].label != "") {
                Param& p = currentSet[activeIdx];
                std::string focusedText = (is_shift_pressed ? "S." : "") + std::to_string(activeIdx + 1) + " " + p.label + ": " + formatValue(p);
                display.text({ 4, 109 }, focusedText, { .font = &PoppinsLight_8 });
            }

            // 3. VELOCITY METER
            float v = v_meter.load();
            if (v > 0.001f) {
                display.filledRect({ 4, 124 }, { (int)(v * 120), 2 });
                v_meter = v * 0.92f;
            }
        }
        emulator.render();
    }

    keep_running = false;
    aThread.join(); mThread.join();
    snd_pcm_close(pcm_h);
    return 0;
}
