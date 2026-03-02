#include <SFML/Graphics.hpp>
#include <alsa/asoundlib.h>
#include <atomic>
#include <cmath>
#include <fstream>
#include <functional>
#include <iomanip>
#include <iostream>
#include <mutex>
#include <sstream>
#include <sys/stat.h>
#include <thread>
#include <vector>

#include "audio/engines/DrumEdge.h"
#include "audio/engines/DrumImpact.h"
#include "audio/engines/DrumKick2.h"
#include "audio/engines/DrumKickFM.h"
#include "audio/engines/DrumKickSegment.h"

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

// --- Menu Structure ---
struct MenuItem {
    std::string label;
    enum Type { PARAM,
        ACTION } type;
    Param* param = nullptr;
    std::function<void()> callback = nullptr;
};

// --- Global State ---
DrawMono<SCREEN_W, SCREEN_H> display;
std::atomic<bool> keep_running { true };
std::mutex engine_mutex;

// Shared buffer for engines
FX_BUFFER

DrumKick2 drumKick2(SAMPLE_RATE);
DrumKickFM drumKickfm(SAMPLE_RATE);
DrumKickSeg drumKickseg(SAMPLE_RATE);
DrumEdge drumEdge(SAMPLE_RATE, buffer);
DrumImpact drumImpact(SAMPLE_RATE, buffer);

IEngine* engines[] = { &drumKick2, &drumKickfm, &drumKickseg, &drumEdge, &drumImpact };
static constexpr size_t NUM_ENGINES = sizeof(engines) / sizeof(engines[0]);
IEngine* currentEngine = engines[0];

// --- Engine Selection Callback ---
void onEngineChange(void* ctx, float val)
{
    int idx = (int)val;
    if (idx < 0 || idx >= (int)NUM_ENGINES) return;

    std::lock_guard<std::mutex> lock(engine_mutex);
    currentEngine = engines[idx];

    Param* p = (Param*)ctx;
    p->string = (char*)currentEngine->getName();
}

// RESTORED PARAM ORDERING
Param shiftParams[12] = {
    { "Master Vol", "%", .value = 100.0f },
    { "Engine", .value = 0.0f, .max = (float)NUM_ENGINES - 1, .context = &shiftParams[1], .onUpdate = onEngineChange },
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

Param slotParam { "Select Slot", .value = 1.0f, .min = 1.0f, .max = 99.0f, .precision = 0 };
int active_slot = -1;
std::vector<MenuItem> menuItems;

std::atomic<float> v_meter { 0.0f };
std::atomic<bool> mc101_connected { false };
std::atomic<bool> is_shift_pressed { false };
std::atomic<bool> is_menu_open { false };
std::atomic<int> active_param_idx { -1 };
std::atomic<int> active_shift_idx { -1 };
std::atomic<int> menu_scroll_idx { 0 };
std::string status_msg = "";
int msg_timer = 0;

// --- Helper Functions ---
std::string formatValue(const Param& p)
{
    if (p.string != nullptr) return std::string(p.string);
    std::stringstream ss;
    ss << std::fixed << std::setprecision(p.precision) << p.value;
    if (p.unit != nullptr && p.unit != "") ss << " " << p.unit;
    return ss.str();
}

void setMessage(std::string msg, int duration = 120)
{
    status_msg = msg;
    msg_timer = duration;
}

void saveState(int slot)
{
    mkdir("data", 0777); // Ensure directory exists
    std::string filename = "data/patch_" + std::to_string(slot) + ".bin";
    std::ofstream os(filename, std::ios::binary);
    if (!os) {
        setMessage("SAVE ERROR");
        return;
    }

    float engineIdx = shiftParams[1].value;
    os.write((char*)&engineIdx, sizeof(float));

    Param* p = currentEngine->getParams();
    for (int i = 0; i < 12; i++)
        os.write((char*)&p[i].value, sizeof(float));
    for (int i = 0; i < 12; i++)
        os.write((char*)&shiftParams[i].value, sizeof(float));

    active_slot = slot;
    setMessage("SAVED SLOT " + std::to_string(slot));
}

void loadState(int slot, int messageDuration = 120)
{
    std::string filename = "data/patch_" + std::to_string(slot) + ".bin";
    std::ifstream is(filename, std::ios::binary);
    if (!is) {
        if (messageDuration > 0) setMessage("PATCH NOT FOUND", messageDuration);
        return;
    }

    float engineIdx;
    is.read((char*)&engineIdx, sizeof(float));

    // IMPORTANT: Set engine BEFORE locking engine_mutex
    // to avoid deadlock in onEngineChange callback
    shiftParams[1].set(engineIdx);

    std::lock_guard<std::mutex> lock(engine_mutex);
    Param* p = currentEngine->getParams();
    for (int i = 0; i < 12; i++)
        is.read((char*)&p[i].value, sizeof(float));
    for (int i = 0; i < 12; i++)
        is.read((char*)&shiftParams[i].value, sizeof(float));

    active_slot = slot;
    if (messageDuration > 0) setMessage("LOADED SLOT " + std::to_string(slot), messageDuration);
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
        : drawer(d)
        , scale(pixelScale)
        , window(sf::VideoMode(W * pixelScale, H * pixelScale), "Kick2 System")
    {
        window.setFramerateLimit(60);
    }

    bool isOpen() { return window.isOpen(); }

    void handleEvents()
    {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                window.close();
                keep_running = false;
            }
            if (event.type == sf::Event::KeyPressed) {
                if (event.key.code == sf::Keyboard::S) is_shift_pressed = true;
                if (event.key.code == sf::Keyboard::X && is_shift_pressed) is_menu_open = !is_menu_open;
                if (event.key.code == sf::Keyboard::X && is_menu_open && !is_shift_pressed) {
                    auto& item = menuItems[menu_scroll_idx];
                    if (item.type == MenuItem::ACTION && item.callback) item.callback();
                }
                if (event.key.code == sf::Keyboard::A) {
                    std::lock_guard<std::mutex> lock(engine_mutex);
                    currentEngine->noteOn(60, 1.0f);
                    v_meter = 1.0f;
                }
            } else if (event.type == sf::Event::KeyReleased) {
                if (event.key.code == sf::Keyboard::S) is_shift_pressed = false;
            }
            if (event.type == sf::Event::MouseWheelScrolled) {
                uint32_t currentTick = timer.getElapsedTime().asMilliseconds();
                int8_t rawDir = (event.mouseWheelScroll.delta > 0) ? 1 : -1;
                int col = (event.mouseWheelScroll.x / scale) / CELL_W;
                int row = (event.mouseWheelScroll.y / scale) / CELL_H;
                int index = row * COLS + col;

                if (is_menu_open) {
                    if (col == 0) menu_scroll_idx = CLAMP(menu_scroll_idx + rawDir, 0, (int)menuItems.size() - 1);
                    else {
                        auto& item = menuItems[menu_scroll_idx];
                        if (item.type == MenuItem::PARAM) item.param->set(item.param->value + (rawDir * item.param->step));
                    }
                } else if (is_shift_pressed) {
                    if (index >= 0 && index < 12 && shiftParams[index].label != "") {
                        active_shift_idx = index;
                        int scaled = encGetScaledDirection(rawDir, currentTick, lastShiftTicks[index]);
                        lastShiftTicks[index] = currentTick;
                        shiftParams[index].set(shiftParams[index].value + (scaled * shiftParams[index].step));
                    }
                } else {
                    if (index >= 0 && index < 12) {
                        active_param_idx = index;
                        int scaled = encGetScaledDirection(rawDir, currentTick, lastTicks[index]);
                        lastTicks[index] = currentTick;
                        std::lock_guard<std::mutex> lock(engine_mutex);
                        currentEngine->getParams()[index].set(currentEngine->getParams()[index].value + (scaled * currentEngine->getParams()[index].step));
                    }
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

// --- Workers ---
void audio_worker(snd_pcm_t* pcm)
{
    const size_t num_frames = 256;
    std::vector<int16_t> buffer_pcm(num_frames * CHANNEL);
    while (keep_running) {
        {
            std::lock_guard<std::mutex> lock(engine_mutex);
            float masterVol = shiftParams[0].value * 0.01f;
            for (uint32_t i = 0; i < num_frames; i++) {
                float s = currentEngine->sample() * masterVol;
                int16_t v16 = static_cast<int16_t>(CLAMP(s, -1.0f, 1.0f) * 32767.0f);
                buffer_pcm[i * 2] = v16;
                buffer_pcm[i * 2 + 1] = v16;
            }
        }
        snd_pcm_sframes_t ret = snd_pcm_writei(pcm, buffer_pcm.data(), num_frames);
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
                        std::lock_guard<std::mutex> lock(engine_mutex);
                        currentEngine->noteOn(60, (float)buf[i + 2] / 127.0f);
                        v_meter = 1.0f;
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

    for (int i = 0; i < 12; i++)
        shiftParams[i].finalize();
    slotParam.finalize();

    menuItems = {
        { "Select Slot", MenuItem::PARAM, &slotParam },
        { "Save Patch", MenuItem::ACTION, nullptr, []() { saveState((int)slotParam.value); } },
        { "Load Patch", MenuItem::ACTION, nullptr, []() { loadState((int)slotParam.value); } },
        { "Exit Menu", MenuItem::ACTION, nullptr, []() { is_menu_open = false; } }
    };

    loadState(1, 0); // This won't deadlock anymore

    std::thread aThread(audio_worker, pcm_h);
    std::thread mThread(midi_manager);
    SFMLEmulator<SCREEN_W, SCREEN_H> emulator(display, PIXEL_SCALE);

    while (emulator.isOpen() && keep_running) {
        emulator.handleEvents();
        display.clear();

        if (!mc101_connected) {
            display.textCentered({ 64, 64 }, "CONNECT MC-101", { .font = &PoppinsLight_8 });
        } else if (is_menu_open) {
            display.text({ 4, 4 }, "SYSTEM MENU", { .font = &PoppinsLight_8 });
            display.line({ 0, 16 }, { 128, 16 });
            for (int i = 0; i < (int)menuItems.size(); i++) {
                int y = 24 + (i * 12);
                bool isSelected = (i == menu_scroll_idx);
                display.text({ 4, y }, (isSelected ? "> " : "  ") + menuItems[i].label, { .font = &PoppinsLight_8 });
                if (menuItems[i].type == MenuItem::PARAM) {
                    std::string valStr = formatValue(*menuItems[i].param);
                    if (menuItems[i].label == "Select Slot" && (int)slotParam.value == active_slot) valStr += " ON";
                    display.text({ 90, y }, valStr, { .font = &PoppinsLight_8 });
                }
            }
            display.line({ 0, 106 }, { 128, 106 });
            if (active_slot != -1) display.text({ 4, 110 }, "ACTIVE: PATCH " + std::to_string(active_slot), { .font = &PoppinsLight_8 });
        } else {
            Param* currentSet = is_shift_pressed ? shiftParams : currentEngine->getParams();
            for (int i = 0; i < 12; i++) {
                int col = i % COLS, row = i / COLS, x = col * CELL_W, y = row * CELL_H;
                Param& p = currentSet[i];
                if (p.label == "") continue;
                display.text({ x + 3, y + 2 }, p.label, { .font = &PoppinsLight_8, .maxWidth = CELL_W - 6 });
                display.rect({ x + 3, y + 14 }, { CELL_W - 6, 4 });
                float pct = (p.value - p.min) / (p.max - p.min);
                display.filledRect({ x + 3, y + 14 }, { (int)((CELL_W - 6) * pct), 4 });
                if (col < COLS - 1)
                    for (int dY = y; dY < y + CELL_H; dY += 3)
                        display.setPixel({ x + CELL_W - 1, dY });
                if (row < ROWS - 1)
                    for (int dX = x; dX < x + CELL_W; dX += 3)
                        display.setPixel({ dX, y + CELL_H - 1 });
            }
            display.line({ 0, 106 }, { 128, 106 });
            int actIdx = is_shift_pressed ? active_shift_idx.load() : active_param_idx.load();
            if (actIdx != -1 && currentSet[actIdx].label != "") {
                std::string info = (is_shift_pressed ? "S." : "") + std::to_string(actIdx + 1) + " " + currentSet[actIdx].label + ": " + formatValue(currentSet[actIdx]);
                display.text({ 4, 109 }, info, { .font = &PoppinsLight_8 });
            }
        }

        if (msg_timer > 0) {
            display.filledRect({ 15, 50 }, { 98, 20 });
            display.rect({ 15, 50 }, { 98, 20 });
            display.textCentered({ 64, 55 }, status_msg, { .font = &PoppinsLight_8, .color = false });
            msg_timer--;
        }
        if (v_meter > 0.001f) {
            display.filledRect({ 4, 124 }, { (int)(v_meter * 120), 2 });
            v_meter = v_meter * 0.92f;
        }
        emulator.render();
    }

    keep_running = false;
    aThread.join();
    mThread.join();
    snd_pcm_close(pcm_h);
    return 0;
}