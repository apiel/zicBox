#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstring>
#include <iomanip>
#include <iostream>
#include <memory>
#include <pthread.h>
#include <sstream>
#include <thread>
#include <vector>

#include "audio/Eq.h"
#include "helpers/enc.h"
#include "helpers/format.h"
#include "helpers/midiNote.h"

#include <alsa/asoundlib.h>
#include <atomic>
#include <iostream>
#include <mutex>
#include <vector>

#include "helpers/clamp.h"

#include <SFML/Graphics.hpp>
#include <atomic>
#include <deque>
#include <mutex>

#define AUDIO_FOLDER std::string("../data/audio")

#include "audio/Eq.h"
#include "audio/effects/fxBuffer.h"
#include "audio/engines/Synth23.h"
#include "draw/draw.h"
#include "helpers/random.h"
#include "zic23/generator.h"
#include "zic23/step.h"

static constexpr uint32_t SAMPLE_RATE = 44100;
static constexpr int BUFFER_SIZE = 4096;
static constexpr int WAVE_HISTORY = 60;

// ================================================================
// Global layout constants  (used by both draw and event handlers)
// ================================================================
static constexpr int MARGIN = 10;
static constexpr int ROW_H = 26; // param panel row height
static constexpr int TRACK_H = 20; // each track header row height
static constexpr int STEP_H = 14; // sequencer step height
static constexpr int LANE_H = 18; // note-lane pixels below step

// Spectrum strip (drawn next to the VU meter, same row as the track name)
static constexpr int SPEC_W = 500; // width of spectrum strip in px

// EQ editor zone (below the sequencer grid)
static constexpr int EQ_ZONE_H = 120; // total height of the EQ editor area
static constexpr int EQ_TRACK_W = 80; // left column: track selector buttons
static constexpr float EQ_DB_RANGE = 12.f;
static constexpr int EQ_DOT_R = 7; // dot hit radius

// ================================================================
// Global UI state
// ================================================================
static bool showHelp = false;
static int eqActiveTrack = 0;

// EQ drag
static bool eqDragging = false;
static int eqDragBand = -1;

// Spectrum strip rects — set during drawStaticUI, read by updateSpectrumPixels
// One per track: absolute pixel rect of the spectrum strip
static sf::IntRect specRects;

// EQ canvas rect — set during drawStaticUI, shared with mouse handler and pixel updater
static sf::IntRect eqCanvasRect; // the curve drawing area (absolute pixels)
static float eqCanvasY_f; // float version for sub-pixel accuracy
static float eqCanvasH_f;

// Track selector button rects (inside EQ zone)
static sf::IntRect eqTrackBtnRects;

// Other rects set during draw
static sf::IntRect helpBtnRect, helpCloseRect;

// Clipboard
static int copyTrackIdx = -1, copyStepIdx = -1;
static Step copiedStep;

struct Track {
    std::unique_ptr<IEngine> engine;
    float volume;
    bool isMuted = false;
    Color themeColor;
    std::atomic<float> vumeter;
    sf::IntRect vuRect, trackBounds, muteRect, volRect;
    std::vector<sf::IntRect> stepRects;
    std::vector<Step> sequence;
    std::deque<float> history;
    std::mutex historyMtx;
    int activeParamIdx = -1;
    std::chrono::steady_clock::time_point lastEditTime;
    std::vector<uint32_t> lastShiftTicks;
    uint32_t lastVolShiftTick = 0;
    sf::IntRect genRect, lenBtnRect;
    void (*generate)(std::vector<Step>& sequence) = nullptr;
    uint32_t noteSamplesRemaining = 0;
    uint32_t genLen = 64;

    EQ eq;
    SpectrumAnalyser spectrum;

    Track(std::unique_ptr<IEngine> e, float v, Color c, void (*gen)(std::vector<Step>& sequence) = nullptr)
        : engine(std::move(e))
        , volume(v)
        , themeColor(c)
        , generate(gen)
        , vumeter(0.0f)
    {
        history.resize(WAVE_HISTORY, 0.0f);
        lastShiftTicks.resize(engine->getParamCount(), 0);
        sequence.resize(SEQ_STEPS);
        stepRects.resize(SEQ_STEPS);

        // maybe pass sample rate as props
        eq.recompute(SAMPLE_RATE);
    }
};

class Studio {
public:
    Track track;
    std::mutex audioMutex;
    std::atomic<float> bpm { 160.0f };
    std::atomic<bool> isPlaying { false };
    std::atomic<int> currentStep { 0 };
    std::atomic<double> sampleCounter { 0.0 };
    double samplesPerStep = 0;
    sf::IntRect bpmRect, transportRect;

    // STEP EDITOR STATE
    int selTrack = -1;
    int selStep = -1;
    sf::IntRect editNoteRect, editVeloRect, editProbRect, editLenRect;


    Studio()
    : track(std::make_unique<Synth23>(SAMPLE_RATE, createFxBuffer(), createFxBuffer()), 0.7f, { 0, 200, 255 }, Generator::generateBass)
    {
        updateClock();
    }

    ~Studio()
    {
        for (auto& b : fxBuffers)
            delete[] b;
    }

    void updateClock() { samplesPerStep = (SAMPLE_RATE * 60.0) / (bpm * 4.0); }

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
uint32_t lastBpmTick = 0;
Random rnd;

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
                    s = trk->eq.process(s); // EQ (post-EQ samples go to spectrum)
                    trk->spectrum.push(s); // spectrum ring buffer

                    if (f == 0) {
                        std::lock_guard<std::mutex> hl(trk->historyMtx);
                        trk->history.push_back(std::abs(s));
                        trk->history.pop_front();
                    }
                    int16_t v = (int16_t)(CLAMP(s, -1.f, 1.f) * 32767.f );
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
    int eqZoneBot = eqZoneTop + EQ_ZONE_H;

    d.filledRect({ 0, eqZoneTop }, { (int)size.x, EQ_ZONE_H }, { .color = { 10, 10, 14 } });

    // EQ curve canvas
    int eqCx = specRects.left;
    int eqCw = SPEC_W; // same width as spectrum strip
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

    // Active track EQ curve
    {
        auto& eq = studio.track.eq;
        Color col = studio.track.themeColor;

        // // Ghost curves for other tracks (very faint)
        // for (int t = 0; t < MAX_TRACKS; t++) {
        //     if (t == eqActiveTrack) continue;
        //     Color gc = studio.tracks[t]->themeColor;
        //     gc.r = (uint8_t)(8 + gc.r * 0.20f);
        //     gc.g = (uint8_t)(8 + gc.g * 0.20f);
        //     gc.b = (uint8_t)(12 + gc.b * 0.20f);
        //     auto pts = studio.tracks[t]->eq.curvePoints(
        //         (float)eqCx, (float)eqCy, (float)eqCw, (float)eqCh, EQ_DB_RANGE, SAMPLE_RATE, 150);
        //     for (int i = 1; i < (int)pts.size(); i++)
        //         d.line({ (int)pts[i - 1].first, (int)pts[i - 1].second },
        //             { (int)pts[i].first, (int)pts[i].second }, { .color = gc });
        // }

        // Active curve (full color)
        auto pts = eq.curvePoints((float)eqCx, (float)eqCy, (float)eqCw, (float)eqCh,
            EQ_DB_RANGE, SAMPLE_RATE, 250);
        for (int i = 1; i < (int)pts.size(); i++)
            d.line({ (int)pts[i - 1].first, (int)pts[i - 1].second },
                { (int)pts[i].first, (int)pts[i].second }, { .color = col });

        // Dots + labels
        const char* bLbl[] = { "Lo", "Mid", "Hi" };
        for (int b = 0; b < EQ::NUM_BANDS; b++) {
            auto [px, py] = eq.dotPos(b, (float)eqCx, (float)eqCy, (float)eqCw, (float)eqCh, EQ_DB_RANGE);
            d.filledCircle({ (int)px, (int)py }, EQ_DOT_R, { .color = col });
            d.circle({ (int)px, (int)py }, EQ_DOT_R, { .color = { 230, 230, 240 } });
            d.text({ (int)px - 8, (int)py + EQ_DOT_R + 2 }, bLbl[b], 8, { .color = { 150, 150, 165 }, .font = &PoppinsLight_8 });
        }

        // Crossover lines
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
}

// Draw spectrum strip directly to pixels buffer for optimization
void updateSpectrumPixels(std::vector<sf::Uint8>& pixels, int stride)
{
        Track* trk = &studio.track;
        // Run FFT on this track's post-EQ ring buffer
        if (!trk->spectrum.compute(SAMPLE_RATE)) return;

        const auto& cols = trk->spectrum.columns;
        const auto& sr = specRects;
        Color col = trk->themeColor;

        // Clear strip to dark background
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

        // Draw frequency bars bottom-aligned
        float colPxW = (float)sr.width / SPEC_COLS;
        for (int c = 0; c < SPEC_COLS; c++) {
            float norm = cols[c];
            int barH = std::max(0, std::min(sr.height, (int)(norm * sr.height)));
            int barX = sr.left + (int)(c * colPxW);
            int barW = std::max(1, (int)colPxW);

            for (int y = 0; y < barH; y++) {
                int py = sr.top + sr.height - 1 - y; // bottom-aligned
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

void drawStaticUI(Draw& d, sf::Vector2u size)
{
    d.clear();
    const int winW = (int)size.x;

    // ---- Top bar ------------------------------------------------
    d.filledRect({ 0, 0 }, { winW, 25 }, { .color = d.styles.colors.quaternary });

    studio.transportRect = { MARGIN, 4, 60, 17 };
    d.filledRect({ MARGIN, 4 }, { 60, 17 }, { .color = studio.isPlaying ? Color { 200, 50, 50 } : Color { 50, 200, 50 } });
    d.text({ MARGIN + 6, 7 }, studio.isPlaying ? "STOP" : "PLAY", 8, { .color = { 255, 255, 255 }, .font = &PoppinsLight_8 });

    helpBtnRect = { MARGIN + 70, 4, 60, 17 };
    d.filledRect({ helpBtnRect.left, helpBtnRect.top }, { 60, 17 }, { .color = { 60, 60, 75 } });
    d.text({ helpBtnRect.left + 14, 7 }, "HELP", 8, { .color = { 255, 255, 255 }, .font = &PoppinsLight_8 });

    std::stringstream bss;
    bss << "BPM: " << std::fixed << std::setprecision(1) << studio.bpm.load();
    studio.bpmRect = { winW - 100, 0, 90, 25 };
    d.textRight({ winW - MARGIN, 6 }, bss.str(), 8, { .color = { 255, 255, 255 }, .font = &PoppinsLight_8 });

    // ---- Per-track engine param panels --------------------------
    int currentY = 35;
    const int paramsPerRow = 8;
    const int colW = (winW - MARGIN * 2) / paramsPerRow;
    auto now = std::chrono::steady_clock::now();

    int trackIdx = 0;
        Track& trk = studio.track;
        int startY = currentY;

        d.filledRect({ MARGIN, currentY + 2 }, { colW / 2, TRACK_H - 4 }, { .color = d.styles.colors.quaternary });
        d.text({ MARGIN + 4, currentY + 4 }, trk.engine->getName(), 8, { .color = trk.themeColor, .font = &PoppinsLight_8 });

        int vuX = MARGIN + colW / 2 + 4;
        trk.vuRect = sf::IntRect(vuX, currentY + 2, WAVE_HISTORY, TRACK_H - 4);

        int specX = vuX + WAVE_HISTORY + 4;
        specRects = sf::IntRect(specX, currentY + 2, SPEC_W, TRACK_H - 4);
        trackIdx++;

        currentY += TRACK_H;

        // Param rows
        Param* params = trk.engine->getParams();
        for (size_t p = 0; p < trk.engine->getParamCount(); p++) {
            int x = MARGIN + ((int)p % paramsPerRow) * colW;
            int y = currentY + ((int)p / paramsPerRow) * ROW_H;
            d.filledRect({ x, y }, { colW - 2, ROW_H - 2 }, { .color = d.styles.colors.quaternary });
            d.text({ x + 4, y + 2 }, params[p].label, 12, { .color = d.styles.colors.text, .font = &PoppinsLight_12 });
            if (winW >= 900 || (trk.activeParamIdx == (int)p && std::chrono::duration_cast<std::chrono::milliseconds>(now - trk.lastEditTime).count() < 1500)) {
                std::stringstream ss;
                ss << std::fixed << std::setprecision(params[p].precision) << params[p].value << params[p].unit;
                d.textRight({ x + colW - 6, y + 2 }, params[p].string ? params[p].string : ss.str(), 8,
                    { .color = { 120, 120, 130 }, .font = &PoppinsLight_8 });
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

    // ---- EQ editor zone ----------------------------------------
    drawEqUI(d, size, currentY);
}

void updateWaveforms(std::vector<sf::Uint8>& pixels, int stride)
{
    Track *p = &studio.track;
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

int main()
{
    snd_pcm_t* pcm_h = audioInit();
    pthread_setname_np(pthread_self(), "zicBox_UI");
    sf::RenderWindow window(sf::VideoMode(1080, 1080), "Patch generator");
    window.setFramerateLimit(60);

    Styles appStyles = {
        .screen = { 1080, 1080 }, .margin = 2, .colors = { { 15, 15, 18 }, { 255, 255, 255 }, { 120, 120, 130 }, { 0, 180, 255 }, { 10, 10, 12 }, { 28, 28, 32 }, { 35, 35, 40 } }
    };
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

            // Stop EQ drag on mouse release
            if (event.type == sf::Event::MouseButtonReleased) {
                eqDragging = false;
                eqDragBand = -1;
            }

            // EQ drag update
            if (event.type == sf::Event::MouseMoved && eqDragging) {
                float mx = (float)event.mouseMove.x, my = (float)event.mouseMove.y;
                studio.track.eq.applyDrag(
                    eqDragBand, mx, my,
                    (float)eqCanvasRect.left, eqCanvasY_f,
                    (float)eqCanvasRect.width, eqCanvasH_f,
                    EQ_DB_RANGE, SAMPLE_RATE);
                static_needs_redraw = true; // redraw EQ curve
            }

                if (event.key.code == sf::Keyboard::Num1) {
                    int trkIdx = event.key.code - sf::Keyboard::Num1;
                    int note = 60;
                    std::lock_guard<std::mutex> lock(studio.audioMutex);
                    if (event.type == sf::Event::KeyReleased)  studio.track.engine->noteOff(note);
                    if (event.type == sf::Event::KeyPressed)   studio.track.engine->noteOn(note, 1.f);
                }
            if (event.type == sf::Event::MouseButtonPressed) {
                int mx = event.mouseButton.x, my = event.mouseButton.y;

                if (showHelp) {
                    if (helpCloseRect.contains(mx, my)) {
                        showHelp = false;
                        static_needs_redraw = true;
                    }
                    continue;
                }


                // EQ dot hit-test
                if (eqCanvasRect.contains(mx, my)) {
                    auto& eq = studio.track.eq;
                    for (int b = 0; b < EQ::NUM_BANDS; b++) {
                        auto [px, py] = eq.dotPos(b,
                            (float)eqCanvasRect.left, eqCanvasY_f,
                            (float)eqCanvasRect.width, eqCanvasH_f, EQ_DB_RANGE);
                        int dx = mx - (int)px, dy = my - (int)py;
                        if (dx * dx + dy * dy <= (EQ_DOT_R + 4) * (EQ_DOT_R + 4)) {
                            eqDragging = true;
                            eqDragBand = b;
                            break;
                        }
                    }
                }

                if (studio.transportRect.contains(mx, my)) {
                    studio.isPlaying = !studio.isPlaying;
                    static_needs_redraw = true;
                }
                if (helpBtnRect.contains(mx, my)) {
                    showHelp = true;
                    static_needs_redraw = true;
                }
            }
            if (event.type == sf::Event::MouseWheelScrolled) {
                if (showHelp) continue;

                int mx = event.mouseWheelScroll.x, my = event.mouseWheelScroll.y;
                float delta = event.mouseWheelScroll.delta;
                uint32_t now = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now().time_since_epoch()).count();
                const int winW = (int)window.getSize().x; // local, from window

                bool handled = false;
                if (!handled) {
                    Track *trk = &studio.track;
                            if (trk->volRect.contains(mx, my)) {
                                int scaled = encGetScaledDirection(delta, now, trk->lastVolShiftTick);
                                trk->lastVolShiftTick = now;
                                trk->volume = CLAMP(trk->volume + scaled * (sf::Keyboard::isKeyPressed(sf::Keyboard::LShift) ? 0.05f : 0.01f), 0.f, 1.f);
                                static_needs_redraw = true;
                            } else if (trk->trackBounds.contains(mx, my)) {
                                const int cW = (winW - MARGIN * 2) / 8;
                                int pIdx = ((my - (trk->trackBounds.top + TRACK_H)) / ROW_H) * 8 + (mx - MARGIN) / cW;
                                if (pIdx >= 0 && (size_t)pIdx < trk->engine->getParamCount()) {
                                    std::lock_guard<std::mutex> lock(studio.audioMutex);
                                    Param& p = trk->engine->getParams()[pIdx];
                                    int scaled = encGetScaledDirection(delta, now, trk->lastShiftTicks[pIdx]);
                                    trk->lastShiftTicks[pIdx] = now;
                                    p.set(p.value + scaled * p.step * (sf::Keyboard::isKeyPressed(sf::Keyboard::LShift) ? 5.f : 1.f));
                                    trk->activeParamIdx = pIdx;
                                    trk->lastEditTime = std::chrono::steady_clock::now();
                                    static_needs_redraw = true;
                                }
                            }
                }
            }
        }

        if (static_needs_redraw) {
            sf::Vector2u winSize = window.getSize();
            drawer->setScreenSize({ (int)winSize.x, (int)winSize.y });
            drawStaticUI(*drawer, winSize);
            for (unsigned y = 0; y < winSize.y; y++)
                std::memcpy(&pixelBuffer[y * BUFFER_SIZE * 4], drawer->screenBuffer[y], winSize.x * 4);
            static_needs_redraw = false;
        }

        if (!showHelp) {
            updateWaveforms(pixelBuffer, BUFFER_SIZE);
            updateSpectrumPixels(pixelBuffer, BUFFER_SIZE);
        }

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