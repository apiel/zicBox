#include <SFML/Graphics.hpp>
#include <algorithm>
#include <alsa/asoundlib.h>
#include <atomic>
#include <chrono>
#include <cmath>
#include <cstring>
#include <deque>
#include <iomanip>
#include <iostream>
#include <memory>
#include <mutex>
#include <pthread.h>
#include <sstream>
#include <thread>
#include <vector>

// NEW: JSON library inclusion
#include "libs/nlohmann/json.hpp"
using json = nlohmann::json;

#define AUDIO_FOLDER std::string("../data/audio")

#include "audio/Eq.h"
#include "audio/effects/fxBuffer.h"
#include "audio/engines/SynthAI.h"
#include "draw/draw.h"
#include "helpers/clamp.h"
#include "helpers/enc.h"
#include "helpers/format.h"
#include "helpers/midiNote.h"

static constexpr uint32_t SAMPLE_RATE = 44100;
static constexpr int BUFFER_SIZE = 4096;
static constexpr int WAVE_HISTORY = 60;

static constexpr int MARGIN = 10;
static constexpr int ROW_H = 26;
static constexpr int TRACK_H = 20;
static constexpr int STEP_H = 14;
static constexpr int LANE_H = 18;

static constexpr int SPEC_W = 500;

static constexpr int EQ_ZONE_H = 120;
static constexpr int EQ_TRACK_W = 80;
static constexpr float EQ_DB_RANGE = 12.f;
static constexpr int EQ_DOT_R = 7;

static constexpr int JSON_BOX_H = 300; // Slightly shorter as grid is efficient

static bool showHelp = false;
static int eqActiveTrack = 0;
static bool eqDragging = false;
static int eqDragBand = -1;

static sf::IntRect specRects;
static sf::IntRect eqCanvasRect;
static float eqCanvasY_f;
static float eqCanvasH_f;
static sf::IntRect eqTrackBtnRects;
static sf::IntRect helpBtnRect, helpCloseRect;

// NEW: JSON state
static sf::IntRect jsonBoxRect;
static std::string patchJsonStr = "";
static bool jsonBoxFocused = false;

// NEW: JSON Interactive elements
struct JsonLine {
    sf::IntRect clickArea;
    int paramIdx;
};
static std::vector<JsonLine> jsonParamHitboxes;

static int copyTrackIdx = -1, copyStepIdx = -1;
struct Track {
    std::unique_ptr<IEngine> engine;
    Color themeColor;
    std::atomic<float> vumeter;
    sf::IntRect vuRect, trackBounds;
    std::vector<sf::IntRect> stepRects;
    std::deque<float> history;
    std::mutex historyMtx;
    int activeParamIdx = -1;
    std::chrono::steady_clock::time_point lastEditTime;
    std::vector<uint32_t> lastShiftTicks;

    EQ eq;
    SpectrumAnalyser spectrum;

    Track(std::unique_ptr<IEngine> e, Color c)
        : engine(std::move(e))
        , themeColor(c)
        , vumeter(0.0f)
    {
        history.resize(WAVE_HISTORY, 0.0f);
        lastShiftTicks.resize(engine->getParamCount(), 0);
        eq.recompute(SAMPLE_RATE);
    }
};

class Studio {
public:
    Track track;
    std::mutex audioMutex;

    Studio()
        : track(std::make_unique<SynthAI>(SAMPLE_RATE, createFxBuffer(), createFxBuffer()), { 0, 200, 255 })
    {
    }

    ~Studio()
    {
        for (auto& b : fxBuffers)
            delete[] b;
    }

protected:
    std::vector<float*> fxBuffers;
    float* createFxBuffer()
    {
        fxBuffers.push_back(new float[FX_BUFFER_SIZE]());
        return fxBuffers.back();
    }
};

Studio studio;
std::atomic<bool> keep_running { true };

std::string serializePatch(IEngine* engine)
{
    json j;
    j["engine"] = engine->getName();
    Param* params = engine->getParams();
    for (size_t i = 0; i < engine->getParamCount(); i++) {
        j["params"][params[i].label] = params[i].value;
    }
    return j.dump(4);
}

void deserializePatch(IEngine* engine, const std::string& rawJson)
{
    try {
        std::lock_guard<std::mutex> lock(studio.audioMutex);
        auto j = json::parse(rawJson);
        Param* params = engine->getParams();
        for (size_t i = 0; i < engine->getParamCount(); i++) {
            std::string label = params[i].label;
            if (j.contains("params") && j["params"].contains(label)) {
                params[i].set(j["params"][label].get<float>());
            }
        }
    } catch (...) {
        std::cerr << "JSON Parse Error\n";
    }
}

snd_pcm_t* audioInit()
{
    snd_pcm_t* pcm_h;
    int err = snd_pcm_open(&pcm_h, "default", SND_PCM_STREAM_PLAYBACK, 0);
    if (err < 0) {
        std::cerr << "Audio open error: " << snd_strerror(err) << std::endl;
        return nullptr;
    }
    snd_pcm_set_params(pcm_h, SND_PCM_FORMAT_S16_LE, SND_PCM_ACCESS_RW_INTERLEAVED, 2, SAMPLE_RATE, 1, 20000);
    return pcm_h;
}

void audioWorker(snd_pcm_t* pcm)
{
    if (!pcm) return;
    const size_t num_frames = 256;
    std::vector<int16_t> buf(num_frames * 2);

    while (keep_running) {
        {
            std::lock_guard<std::mutex> lock(studio.audioMutex);
            std::fill(buf.begin(), buf.end(), 0);

            for (uint32_t f = 0; f < num_frames; f++) {
                Track* trk = &studio.track;
                float s = trk->engine->sample();
                s = trk->eq.process(s);
                trk->spectrum.push(s);

                if (f == 0) {
                    std::lock_guard<std::mutex> hl(trk->historyMtx);
                    trk->history.push_back(std::abs(s));
                    trk->history.pop_front();
                }
                int16_t v = (int16_t)(CLAMP(s, -1.f, 1.f) * 32767.f);
                buf[f * 2] += v;
                buf[f * 2 + 1] += v;
            }
        }

        sf::Int64 w = snd_pcm_writei(pcm, buf.data(), num_frames);
        if (w < 0) {
            w = snd_pcm_recover(pcm, (int)w, 0);
            if (w < 0) {
                std::cerr << "ALSA: " << snd_strerror((int)w) << "\n";
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
            }
        }
    }
}

void drawEqUI(Draw& d, sf::Vector2u size, int currentY)
{
    int eqZoneTop = currentY;
    d.filledRect({ 0, eqZoneTop }, { (int)size.x, EQ_ZONE_H }, { .color = { 10, 10, 14 } });

    int eqCx = specRects.left;
    int eqCw = SPEC_W;
    int eqCy = eqZoneTop + 10;
    int eqCh = EQ_ZONE_H - 20;

    eqCanvasRect = sf::IntRect(eqCx, eqCy, eqCw, eqCh);
    eqCanvasY_f = (float)eqCy;
    eqCanvasH_f = (float)eqCh;

    d.filledRect({ eqCx, eqCy }, { eqCw, eqCh }, { .color = { 8, 8, 12 } });
    d.rect({ eqCx, eqCy }, { eqCw, eqCh }, { .color = { 45, 45, 58 } });

    Color gridCol = { 30, 30, 40 }, gridLbl = { 65, 65, 80 };
    for (int db : { -12, -6, 0, 6, 12 }) {
        int y = eqCy + eqCh / 2 - (int)((float)db / EQ_DB_RANGE * (eqCh / 2));
        d.line({ eqCx, y }, { eqCx + eqCw, y }, { .color = db == 0 ? Color { 50, 50, 65 } : gridCol });
        d.text({ eqCx - 38, y - 4 }, (db > 0 ? "+" : "") + std::to_string(db) + "dB", 8, { .color = gridLbl, .font = &PoppinsLight_8 });
    }

    for (float f : { 100.f, 500.f, 1000.f, 5000.f, 10000.f }) {
        float t = logf(f / 20.f) / logf(1000.f);
        int x = eqCx + (int)(eqCw * t);
        d.line({ x, eqCy }, { x, eqCy + eqCh }, { .color = gridCol });
    }

    auto& eq = studio.track.eq;
    Color col = studio.track.themeColor;
    auto pts = eq.curvePoints((float)eqCx, (float)eqCy, (float)eqCw, (float)eqCh, EQ_DB_RANGE, SAMPLE_RATE, 250);
    for (int i = 1; i < (int)pts.size(); i++)
        d.line({ (int)pts[i - 1].first, (int)pts[i - 1].second }, { (int)pts[i].first, (int)pts[i].second }, { .color = col });

    const char* bLbl[] = { "Lo", "Mid", "Hi" };
    for (int b = 0; b < EQ::NUM_BANDS; b++) {
        auto [px, py] = eq.dotPos(b, (float)eqCanvasRect.left, eqCanvasY_f, (float)eqCanvasRect.width, eqCanvasH_f, EQ_DB_RANGE);
        d.filledCircle({ (int)px, (int)py }, EQ_DOT_R, { .color = col });
        d.circle({ (int)px, (int)py }, EQ_DOT_R, { .color = { 230, 230, 240 } });
        d.text({ (int)px - 8, (int)py + EQ_DOT_R + 2 }, bLbl[b], 8, { .color = { 150, 150, 165 }, .font = &PoppinsLight_8 });
    }

    auto fmtHz = [](float f) -> std::string {
        if (f >= 1000.f) {
            std::ostringstream s;
            s << std::fixed << std::setprecision(1) << f / 1000.f << "k";
            return s.str();
        }
        return std::to_string((int)f) + "Hz";
    };
    int xLo = (int)EQ::freqToPx(eq.crossoverLow, (float)eqCx, (float)eqCw);
    int xHi = (int)EQ::freqToPx(eq.crossoverHigh, (float)eqCx, (float)eqCw);
    d.line({ xLo, eqCy }, { xLo, eqCy + eqCh }, { .color = { 70, 70, 90 } });
    d.line({ xHi, eqCy }, { xHi, eqCy + eqCh }, { .color = { 70, 70, 90 } });
    d.text({ xLo + 2, eqCy + 2 }, fmtHz(eq.crossoverLow), 8, { .color = { 110, 110, 130 }, .font = &PoppinsLight_8 });
    d.text({ xHi + 2, eqCy + 2 }, fmtHz(eq.crossoverHigh), 8, { .color = { 110, 110, 130 }, .font = &PoppinsLight_8 });
}

void updateSpectrumPixels(std::vector<sf::Uint8>& pixels, int stride)
{
    Track* trk = &studio.track;
    if (!trk->spectrum.compute(SAMPLE_RATE)) return;
    const auto& cols = trk->spectrum.columns;
    const auto& sr = specRects;
    Color col = trk->themeColor;

    for (int y = 0; y < sr.height; y++) {
        for (int x = 0; x < sr.width; x++) {
            size_t idx = ((sr.top + y) * stride + sr.left + x) * 4;
            if (idx + 2 < pixels.size()) {
                pixels[idx] = 8;
                pixels[idx + 1] = 8;
                pixels[idx + 2] = 12;
            }
        }
    }
    if (!trk->spectrum.updated) return;
    float colPxW = (float)sr.width / SPEC_COLS;
    for (int c = 0; c < SPEC_COLS; c++) {
        float norm = cols[c];
        int barH = std::max(0, std::min(sr.height, (int)(norm * sr.height)));
        int barX = sr.left + (int)(c * colPxW);
        int barW = std::max(1, (int)colPxW);
        for (int y = 0; y < barH; y++) {
            int py = sr.top + sr.height - 1 - y;
            float bright = 0.55f + 0.45f * ((float)y / std::max(1, barH));
            for (int x = 0; x < barW; x++) {
                size_t idx = ((py)*stride + barX + x) * 4;
                if (idx + 2 < pixels.size()) {
                    pixels[idx] = (uint8_t)(col.r * bright);
                    pixels[idx + 1] = (uint8_t)(col.g * bright);
                    pixels[idx + 2] = (uint8_t)(col.b * bright);
                }
            }
        }
    }
}

// ================================================================
// Fancy JSON Renderer: Grid Structure (8 columns)
// ================================================================
void drawFancyJsonEditor(Draw& d, Track& trk, sf::IntRect rect, sf::RenderWindow& window)
{
    d.filledRect({ rect.left, rect.top }, { rect.width, rect.height }, { .color = { 12, 12, 18 } });
    d.rect({ rect.left, rect.top }, { rect.width, rect.height }, { .color = jsonBoxFocused ? trk.themeColor : Color { 40, 40, 45 } });

    int startX = rect.left + 15;
    int startY = rect.top + 15;
    int fontSize = 12;
    int lineH = 20;
    const int paramsPerRow = 8;
    const int colW = (rect.width - 40) / paramsPerRow;

    Color colKey = { 140, 180, 250 };
    Color colVal = { 200, 230, 150 };
    Color colBracket = { 180, 180, 180 };

    jsonParamHitboxes.clear();

    // Opening Structure
    d.text({ startX, startY }, "{", fontSize, { .color = colBracket });
    std::string trackname = trk.engine->getName(); // for whatever reason we need to assign to string...
    d.text({ startX + 15, startY + lineH }, "\"engine\": \"" + trackname + "\",", fontSize, { .color = colKey });
    d.text({ startX + 15, startY + lineH * 2 }, "\"params\": {", fontSize, { .color = colBracket });

    Param* params = trk.engine->getParams();
    size_t pCount = trk.engine->getParamCount();
    int gridStartY = startY + (lineH * 3);

    // Get current mouse pos for hover effect
    sf::Vector2i mPos = sf::Mouse::getPosition(window); // Assuming d has access to the window, or use a global

    for (size_t i = 0; i < pCount; i++) {
        int row = i / paramsPerRow;
        int col = i % paramsPerRow;
        int x = startX + 30 + (col * colW);
        int y = gridStartY + (row * lineH);

        std::string key = "\"" + std::string(params[i].label) + "\": ";
        std::stringstream ss;
        ss << std::fixed << std::setprecision(1) << params[i].value;
        std::string val = ss.str() + (i == pCount - 1 ? "" : ",");

        // Calculate total width for the hitbox (Key + Value)
        int totalWidth = colW - 10;
        sf::IntRect fullHitbox(x - 4, y, totalWidth, lineH);

        // Highlight if hovered
        if (fullHitbox.contains(mPos.x, mPos.y)) {
            d.filledRect({ fullHitbox.left, fullHitbox.top - 3 }, { fullHitbox.width, fullHitbox.height }, { .color = { 30, 30, 45 } });
        }

        int xValStart = d.text({ x, y }, key, fontSize, { .color = colKey, .font = &PoppinsLight_12 });
        d.text({ xValStart + 2, y }, val, fontSize, { .color = colVal, .font = &PoppinsLight_12 });

        // Register hitbox
        jsonParamHitboxes.push_back({ fullHitbox, (int)i });
    }

    int closingY = gridStartY + (((int)pCount - 1) / paramsPerRow + 1) * lineH;
    d.text({ startX + 15, closingY }, "}", fontSize, { .color = colBracket });
    d.text({ startX, closingY + lineH }, "}", fontSize, { .color = colBracket });
}

void drawStaticUI(Draw& d, sf::Vector2u size, sf::RenderWindow& window)
{
    d.clear();
    const int winW = (int)size.x;
    int currentY = 35;
    const int paramsPerRow = 8;
    const int colW = (winW - MARGIN * 2) / paramsPerRow;
    auto now = std::chrono::steady_clock::now();

    Track& trk = studio.track;
    int startY = currentY;

    d.filledRect({ MARGIN, currentY + 2 }, { colW / 2, TRACK_H - 4 }, { .color = d.styles.colors.quaternary });
    d.text({ MARGIN + 4, currentY + 4 }, trk.engine->getName(), 8, { .color = trk.themeColor, .font = &PoppinsLight_8 });

    int vuX = MARGIN + colW / 2 + 4;
    trk.vuRect = sf::IntRect(vuX, currentY + 2, WAVE_HISTORY, TRACK_H - 4);
    int specX = vuX + WAVE_HISTORY + 4;
    specRects = sf::IntRect(specX, currentY + 2, SPEC_W, TRACK_H - 4);

    currentY += TRACK_H;
    Param* params = trk.engine->getParams();
    Color bgColor[4] = { darken(d.styles.colors.quaternary, 0.1), lighten(d.styles.colors.quaternary, 0.5), darken(d.styles.colors.quaternary, 0.25), lighten(d.styles.colors.quaternary, 0.1) };

    for (size_t p = 0; p < trk.engine->getParamCount(); p++) {
        int x = MARGIN + ((int)p % paramsPerRow) * colW;
        int y = currentY + ((int)p / paramsPerRow) * ROW_H;
        // d.filledRect({ x, y }, { colW - 2, ROW_H - 2 }, { .color = d.styles.colors.quaternary });
        d.filledRect({ x, y }, { colW - 2, ROW_H - 2 }, { .color = bgColor[params[p].group % 4] });
        d.text({ x + 4, y + 2 }, params[p].label, 12, { .color = d.styles.colors.text, .font = &PoppinsLight_12 });
        if (winW >= 900 || (trk.activeParamIdx == (int)p && std::chrono::duration_cast<std::chrono::milliseconds>(now - trk.lastEditTime).count() < 1500)) {
            std::stringstream ss;
            ss << std::fixed << std::setprecision(params[p].precision) << params[p].value << params[p].unit;
            d.textRight({ x + colW - 6, y + 2 }, params[p].string ? params[p].string : ss.str(), 8, { .color = { 120, 120, 130 }, .font = &PoppinsLight_8 });
        }

        float range = params[p].max - params[p].min;
        float pct = (params[p].value - params[p].min) / (range <= 0 ? 1.f : range);
        int bX = x + 4, bY = y + ROW_H - 8, bW = colW - 10;
        if (params[p].type & VALUE_CENTERED) {
            int mid = bX + bW / 2;
            int fw = (int)((bW / 2) * (params[p].value / params[p].max));
            if (fw < 0) d.filledRect({ mid + fw, bY }, { std::abs(fw), 3 }, { .color = trk.themeColor });
            else d.filledRect({ mid, bY }, { fw, 3 }, { .color = trk.themeColor });
            d.filledRect({ mid, bY - 1 }, { 1, 5 }, { .color = { 100, 100, 100 } });
        } else {
            d.filledRect({ bX, bY }, { (int)(bW * pct), 3 }, { .color = trk.themeColor });
        }
    }
    int secH = (((int)trk.engine->getParamCount() + 7) / 8) * ROW_H + TRACK_H;
    trk.trackBounds = sf::IntRect(MARGIN, startY, winW - MARGIN * 2, secH);
    currentY += secH - TRACK_H + 2;

    currentY += 10;
    drawEqUI(d, size, currentY);

    currentY += EQ_ZONE_H + 25;
    jsonBoxRect = sf::IntRect(MARGIN, currentY, winW - (MARGIN * 2), JSON_BOX_H);
    drawFancyJsonEditor(d, trk, jsonBoxRect, window);
    d.text({ jsonBoxRect.left, jsonBoxRect.top + jsonBoxRect.height + 2 }, "PATCH JSON (Scroll on values to edit / CTRL+C to copy)", 12, { .color = { 100, 100, 110 }, .font = &PoppinsLight_12 });
}

void updateWaveforms(std::vector<sf::Uint8>& pixels, int stride)
{
    Track* p = &studio.track;
    std::lock_guard<std::mutex> lk(p->historyMtx);
    for (int x = 0; x < WAVE_HISTORY; x++) {
        int bH = (int)(std::min(p->history[x], 1.f) * (p->vuRect.height / 2));
        for (int y = 0; y < p->vuRect.height; y++) {
            size_t idx = ((p->vuRect.top + y) * stride + p->vuRect.left + x) * 4;
            bool w = std::abs(y - p->vuRect.height / 2) <= bH;
            pixels[idx] = w ? p->themeColor.r : 20;
            pixels[idx + 1] = w ? p->themeColor.g : 20;
            pixels[idx + 2] = w ? p->themeColor.b : 25;
        }
    }
}

void triggerPreview(Track& trk, int note, float velocity, int durationMs = 200)
{
    {
        std::lock_guard<std::mutex> lock(studio.audioMutex);
        trk.engine->noteOn(note, velocity);
    }
    std::thread([&trk, note, durationMs]() {
        std::this_thread::sleep_for(std::chrono::milliseconds(durationMs));
        std::lock_guard<std::mutex> lock(studio.audioMutex);
        trk.engine->noteOff(note);
    }).detach();
}

int main()
{
    snd_pcm_t* pcm_h = audioInit();
    pthread_setname_np(pthread_self(), "zicBox_UI");
    sf::RenderWindow window(sf::VideoMode(1180, 850), "Patch generator");
    window.setFramerateLimit(60);
    window.setKeyRepeatEnabled(false);

    Styles appStyles = { .screen = { 1180, 850 }, .margin = 2, .colors = { { 15, 15, 18 }, { 255, 255, 255 }, { 120, 120, 130 }, { 0, 180, 255 }, { 10, 10, 12 }, { 28, 28, 32 }, { 35, 35, 40 } } };
    auto drawer = std::make_unique<Draw>(appStyles);
    sf::Texture screenTexture;
    screenTexture.create(BUFFER_SIZE, BUFFER_SIZE);
    sf::Sprite screenSprite(screenTexture);
    std::vector<sf::Uint8> pixelBuffer(BUFFER_SIZE * BUFFER_SIZE * 4, 15);

    bool static_needs_redraw = true;
    std::thread aThread(audioWorker, pcm_h);
    pthread_setname_np(aThread.native_handle(), "zicBox_Audio");

    while (window.isOpen() && keep_running) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) window.close();
            if (event.type == sf::Event::Resized) {
                window.setView(sf::View(sf::FloatRect(0, 0, (float)event.size.width, (float)event.size.height)));
                static_needs_redraw = true;
            }

            if (event.type == sf::Event::MouseButtonReleased) {
                eqDragging = false;
                eqDragBand = -1;
            }

            if (event.type == sf::Event::MouseMoved) {
                if (eqDragging) {
                    studio.track.eq.applyDrag(eqDragBand, (float)event.mouseMove.x, (float)event.mouseMove.y, (float)eqCanvasRect.left, eqCanvasY_f, (float)eqCanvasRect.width, eqCanvasH_f, EQ_DB_RANGE, SAMPLE_RATE);
                    static_needs_redraw = true;
                }
                // Trigger redraw if mouse is within JSON box to handle hover effect
                if (jsonBoxRect.contains(event.mouseMove.x, event.mouseMove.y)) {
                    static_needs_redraw = true;
                }
            }

            if (event.key.code == sf::Keyboard::Space || event.key.code == sf::Keyboard::Return) {
                std::lock_guard<std::mutex> lock(studio.audioMutex);
                if (event.type == sf::Event::KeyReleased) studio.track.engine->noteOff(60);
                if (event.type == sf::Event::KeyPressed) studio.track.engine->noteOn(60, 1.f);
            }

            if (event.type == sf::Event::KeyPressed) {
                if (sf::Keyboard::isKeyPressed(sf::Keyboard::LControl)) {
                    if (event.key.code == sf::Keyboard::C) {
                        sf::Clipboard::setString(serializePatch(studio.track.engine.get()));
                        static_needs_redraw = true;
                    }
                    if (event.key.code == sf::Keyboard::V) {
                        deserializePatch(studio.track.engine.get(), sf::Clipboard::getString());
                        static_needs_redraw = true;
                        triggerPreview(studio.track, 60, 1.f);
                    }
                }
            }

            if (event.type == sf::Event::MouseButtonPressed) {
                int mx = event.mouseButton.x, my = event.mouseButton.y;
                jsonBoxFocused = jsonBoxRect.contains(mx, my);
                static_needs_redraw = true;

                if (eqCanvasRect.contains(mx, my)) {
                    auto& eq = studio.track.eq;
                    for (int b = 0; b < EQ::NUM_BANDS; b++) {
                        auto [px, py] = eq.dotPos(b, (float)eqCanvasRect.left, eqCanvasY_f, (float)eqCanvasRect.width, eqCanvasH_f, EQ_DB_RANGE);
                        int dx = mx - (int)px, dy = my - (int)py;
                        if (dx * dx + dy * dy <= (EQ_DOT_R + 4) * (EQ_DOT_R + 4)) {
                            eqDragging = true;
                            eqDragBand = b;
                            break;
                        }
                    }
                }
            }

            if (event.type == sf::Event::MouseWheelScrolled) {
                int mx = event.mouseWheelScroll.x, my = event.mouseWheelScroll.y;
                float delta = event.mouseWheelScroll.delta;
                uint32_t now_ms = (uint32_t)std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now().time_since_epoch()).count();

                // Logic for the JSON grid
                if (jsonBoxRect.contains(mx, my)) {
                    for (auto& hb : jsonParamHitboxes) {
                        if (hb.clickArea.contains(mx, my)) {
                            std::lock_guard<std::mutex> lock(studio.audioMutex);
                            Param& p = studio.track.engine->getParams()[hb.paramIdx];
                            int scaled = encGetScaledDirection(delta, now_ms, studio.track.lastShiftTicks[hb.paramIdx]);
                            studio.track.lastShiftTicks[hb.paramIdx] = now_ms;
                            p.set(p.value + scaled * p.step * (sf::Keyboard::isKeyPressed(sf::Keyboard::LShift) ? 5.f : 1.f));
                            static_needs_redraw = true;
                            break;
                        }
                    }
                }
                // Logic for the UI Sliders
                else if (studio.track.trackBounds.contains(mx, my)) {
                    const int winW = (int)window.getSize().x;
                    const int cW = (winW - MARGIN * 2) / 8;
                    int pIdx = ((my - (studio.track.trackBounds.top + TRACK_H)) / ROW_H) * 8 + (mx - MARGIN) / cW;
                    if (pIdx >= 0 && (size_t)pIdx < studio.track.engine->getParamCount()) {
                        std::lock_guard<std::mutex> lock(studio.audioMutex);
                        Param& p = studio.track.engine->getParams()[pIdx];
                        int scaled = encGetScaledDirection(delta, now_ms, studio.track.lastShiftTicks[pIdx]);
                        studio.track.lastShiftTicks[pIdx] = now_ms;
                        p.set(p.value + scaled * p.step * (sf::Keyboard::isKeyPressed(sf::Keyboard::LShift) ? 5.f : 1.f));
                        studio.track.activeParamIdx = pIdx;
                        studio.track.lastEditTime = std::chrono::steady_clock::now();
                        static_needs_redraw = true;
                    }
                }
            }
        }

        if (static_needs_redraw) {
            sf::Vector2u winSize = window.getSize();
            drawer->setScreenSize({ (int)winSize.x, (int)winSize.y });
            drawStaticUI(*drawer, winSize, window);
            for (unsigned y = 0; y < winSize.y; y++)
                std::memcpy(&pixelBuffer[y * BUFFER_SIZE * 4], drawer->screenBuffer[y], winSize.x * 4);
            static_needs_redraw = false;
        }

        updateWaveforms(pixelBuffer, BUFFER_SIZE);
        updateSpectrumPixels(pixelBuffer, BUFFER_SIZE);

        screenTexture.update(pixelBuffer.data());
        window.clear();
        window.draw(screenSprite);
        window.display();
    }

    keep_running = false;
    aThread.join();
    snd_pcm_close(pcm_h);
    return 0;
}