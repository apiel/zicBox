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
#include <sstream>
#include <thread>
#include <vector>

#include "audio/engines/DrumClap.h"
#include "audio/engines/DrumKick23.h"
#include "draw/draw.h"
#include "helpers/clamp.h"
#include "helpers/enc.h"

static constexpr int MAX_TRACKS = 8;
static constexpr uint32_t SAMPLE_RATE = 44100;
static constexpr int BUFFER_SIZE = 4096;
static constexpr int WAVE_HISTORY = 60;
static constexpr int SEQ_STEPS = 64;

struct Step {
    bool active = false;
    int note = 60;
    float velocity = 0.8f;
    float condition = 1.0f;
};

struct Track {
    std::unique_ptr<IEngine> engine;
    std::string name;
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

    Track(std::unique_ptr<IEngine> e, std::string n, float v, Color c)
        : engine(std::move(e))
        , name(n)
        , volume(v)
        , themeColor(c)
        , vumeter(0.0f)
    {
        history.resize(WAVE_HISTORY, 0.0f);
        lastShiftTicks.resize(engine->getParamCount(), 0);
        sequence.resize(SEQ_STEPS);
        stepRects.resize(SEQ_STEPS);
    }
};

class Studio {
public:
    std::vector<std::unique_ptr<Track>> tracks;
    std::mutex audioMutex;
    float* sharedReverbBuffer;
    std::atomic<float> bpm { 120.0f };
    std::atomic<bool> isPlaying { false };
    std::atomic<int> currentStep { 0 };
    std::atomic<double> sampleCounter { 0.0 }; 
    double samplesPerStep = 0;
    sf::IntRect bpmRect, transportRect;

    Studio()
    {
        sharedReverbBuffer = new float[SAMPLE_RATE * 2]();
        Color palette[8] = { { 0, 200, 255 }, { 255, 100, 100 }, { 100, 255, 100 }, { 255, 200, 50 }, { 200, 100, 255 }, { 50, 255, 200 }, { 255, 150, 50 }, { 180, 180, 180 } };
        for (int i = 0; i < 4; i++)
            tracks.push_back(std::make_unique<Track>(std::make_unique<DrumKick23>(SAMPLE_RATE, sharedReverbBuffer), "KICK " + std::to_string(i + 1), 0.7f, palette[i]));
        for (int i = 4; i < 8; i++)
            tracks.push_back(std::make_unique<Track>(std::make_unique<DrumClap>(SAMPLE_RATE, sharedReverbBuffer), "CLAP " + std::to_string(i - 3), 0.7f, palette[i]));
        updateClock();
    }
    void updateClock() { samplesPerStep = (SAMPLE_RATE * 60.0) / (bpm * 4.0); }
    ~Studio() { delete[] sharedReverbBuffer; }
};

Studio studio;
std::atomic<bool> keep_running { true };
uint32_t lastBpmTick = 0;

void audio_worker(snd_pcm_t* pcm)
{
    const size_t num_frames = 256;
    std::vector<int16_t> buffer_pcm(num_frames * 2);
    while (keep_running) {
        {
            std::lock_guard<std::mutex> lock(studio.audioMutex);
            std::fill(buffer_pcm.begin(), buffer_pcm.end(), 0);
            for (uint32_t f = 0; f < num_frames; f++) {
                if (studio.isPlaying) {
                    studio.sampleCounter = studio.sampleCounter + 1.0;
                    if (studio.sampleCounter >= studio.samplesPerStep) {
                        studio.sampleCounter = 0;
                        studio.currentStep = (studio.currentStep + 1) % SEQ_STEPS;
                        for (auto& trk : studio.tracks) {
                            if (trk->sequence[studio.currentStep].active && !trk->isMuted)
                                trk->engine->noteOn(trk->sequence[studio.currentStep].note, trk->sequence[studio.currentStep].velocity);
                        }
                    }
                }
                for (auto& trk : studio.tracks) {
                    float s = trk->engine->sample();
                    if (f == 0) {
                        std::lock_guard<std::mutex> hLock(trk->historyMtx);
                        trk->history.push_back(std::abs(s));
                        trk->history.pop_front();
                    }
                    float p = s * (trk->isMuted ? 0.0f : trk->volume);
                    int16_t val = (int16_t)(CLAMP(p, -1.0f, 1.0f) * 32767.0f / (MAX_TRACKS / 2));
                    buffer_pcm[f * 2] += val;
                    buffer_pcm[f * 2 + 1] += val;
                }
            }
        }
        snd_pcm_writei(pcm, buffer_pcm.data(), num_frames);
    }
}

void drawStaticUI(Draw& d, sf::Vector2u size)
{
    d.clear();
    int winW = (int)size.x, margin = 10, rowH = 26;

    // Header Bar
    d.filledRect({ 0, 0 }, { winW, 25 }, { .color = d.styles.colors.quaternary });

    // Transport Button
    studio.transportRect = { margin, 4, 60, 17 };
    d.filledRect({ studio.transportRect.left, studio.transportRect.top }, { 60, 17 }, { .color = studio.isPlaying ? Color { 200, 50, 50 } : Color { 50, 200, 50 } });
    d.text({ margin + 6, 7 }, studio.isPlaying ? "STOP" : "PLAY", 8, { .color = { 255, 255, 255 }, .font = &PoppinsLight_8 });

    // BPM Display
    std::stringstream bss;
    bss << "BPM: " << std::fixed << std::setprecision(1) << studio.bpm.load();
    studio.bpmRect = { winW - 100, 0, 90, 25 };
    d.textRight({ winW - margin, 6 }, bss.str(), 8, { .color = { 255, 255, 255 }, .font = &PoppinsLight_8 });

    int currentY = 35;
    int paramsPerRow = 8, colW = (winW - (margin * 2)) / paramsPerRow;
    auto now = std::chrono::steady_clock::now();

    for (auto& trkPtr : studio.tracks) {
        Track& trk = *trkPtr;
        int startY = currentY;
        d.filledRect({ margin, currentY }, { colW / 2, 12 }, { .color = d.styles.colors.quaternary });
        d.text({ margin + 4, currentY + 1 }, trk.name, 8, { .color = trk.themeColor, .font = &PoppinsLight_8 });
        trk.vuRect = sf::IntRect(margin + (colW / 2) + 10, currentY - 2, WAVE_HISTORY, 16);
        currentY += 14;
        Param* params = trk.engine->getParams();
        for (size_t p = 0; p < trk.engine->getParamCount(); p++) {
            int x = margin + ((p % paramsPerRow) * colW), y = currentY + ((p / paramsPerRow) * rowH);
            d.filledRect({ x, y }, { colW - 2, rowH - 2 }, { .color = d.styles.colors.quaternary });
            d.text({ x + 4, y + 2 }, params[p].label, 12, { .color = d.styles.colors.text, .font = &PoppinsLight_12 });
            if (winW >= 900 || (trk.activeParamIdx == (int)p && std::chrono::duration_cast<std::chrono::milliseconds>(now - trk.lastEditTime).count() < 1500)) {
                std::stringstream ss;
                ss << std::fixed << std::setprecision(1) << params[p].value << params[p].unit;
                d.textRight({ x + colW - 6, y + 2 }, params[p].string ? params[p].string : ss.str(), 8, { .color = { 120, 120, 130 }, .font = &PoppinsLight_8 });
            }
            float pct = (params[p].value - params[p].min) / (params[p].max - params[p].min);
            d.filledRect({ x + 4, y + rowH - 8 }, { (int)((colW - 10) * pct), 3 }, { .color = trk.themeColor });
        }
        int sectionHeight = (((trk.engine->getParamCount() + 7) / 8) * rowH) + 14;
        trk.trackBounds = sf::IntRect(margin, startY, winW - (margin * 2), sectionHeight);
        currentY += sectionHeight - 2;
    }

    currentY += 10;
    int mixerWidth = 120, stepW = (winW - (margin * 2 + mixerWidth)) / 64, stepH = 14;
    for (int i = 0; i < MAX_TRACKS; i++) {
        Track& trk = *studio.tracks[i];
        int ty = currentY + (i * (stepH + 4));
        trk.muteRect = { margin, ty, 25, stepH };
        trk.volRect = { margin + 30, ty, 70, stepH };
        d.filledRect({ trk.muteRect.left, trk.muteRect.top }, { 25, stepH }, { .color = trk.isMuted ? Color { 200, 50, 50 } : Color { 40, 40, 45 } });
        d.text({ trk.muteRect.left + 8, trk.muteRect.top + 1 }, "M", 8, { .color = { 255, 255, 255 }, .font = &PoppinsLight_8 });
        d.filledRect({ trk.volRect.left, trk.volRect.top }, { 70, stepH }, { .color = Color { 40, 40, 45 } });
        d.filledRect({ trk.volRect.left, trk.volRect.top + (stepH / 2) - 2 }, { (int)(70 * trk.volume), 4 }, { .color = trk.themeColor });
        for (int s = 0; s < SEQ_STEPS; s++)
            trk.stepRects[s] = { margin + mixerWidth + (s * stepW), ty, stepW - 1, stepH };
    }
}

void updateWaveforms(std::vector<sf::Uint8>& pixels, int stride)
{
    for (auto& trkPtr : studio.tracks) {
        std::lock_guard<std::mutex> hLock(trkPtr->historyMtx);
        for (int x = 0; x < WAVE_HISTORY; x++) {
            int barH = (int)(std::min(trkPtr->history[x], 1.0f) * (trkPtr->vuRect.height / 2));
            for (int y = 0; y < trkPtr->vuRect.height; y++) {
                size_t idx = ((trkPtr->vuRect.top + y) * stride + (trkPtr->vuRect.left + x)) * 4;
                bool isWave = std::abs(y - (trkPtr->vuRect.height / 2)) <= barH;
                pixels[idx] = isWave ? trkPtr->themeColor.r : 20;
                pixels[idx + 1] = isWave ? trkPtr->themeColor.g : 20;
                pixels[idx + 2] = isWave ? trkPtr->themeColor.b : 25;
            }
        }
    }
}

// void updateSequencerPixels(std::vector<sf::Uint8>& pixels, int stride)
// {
//     int current = studio.currentStep;
//     for (auto& trk : studio.tracks) {
//         for (int s = 0; s < SEQ_STEPS; s++) {
//             auto& r = trk->stepRects[s];
//             Color c = trk->sequence[s].active ? trk->themeColor : ((s % 4 == 0) ? Color { 35, 35, 40 } : Color { 25, 25, 30 });
//             if (s == current && studio.isPlaying) c = { 255, 255, 255 };
//             for (int y = 0; y < r.height; y++) {
//                 for (int x = 0; x < r.width; x++) {
//                     size_t idx = ((r.top + y) * stride + (r.left + x)) * 4;
//                     pixels[idx] = c.r;
//                     pixels[idx + 1] = c.g;
//                     pixels[idx + 2] = c.b;
//                 }
//             }
//         }
//     }
// }

void updateSequencerPixels(std::vector<sf::Uint8>& pixels, int stride)
{
    // Calculate precise playhead position across the step grid
    int stepWidth = studio.tracks[0]->stepRects[0].width + 1;
    double progressInStep = studio.sampleCounter.load() / studio.samplesPerStep;
    int playheadGlobalX = (int)((studio.currentStep + progressInStep) * stepWidth);
    int gridStartX = studio.tracks[0]->stepRects[0].left;

    for (auto& trk : studio.tracks) {
        for (int s = 0; s < SEQ_STEPS; s++) {
            auto& r = trk->stepRects[s];
            Color c = trk->sequence[s].active ? trk->themeColor : ((s % 4 == 0) ? Color { 35, 35, 40 } : Color { 25, 25, 30 });

            for (int y = 0; y < r.height; y++) {
                for (int x = 0; x < r.width; x++) {
                    int globalX = r.left + x;
                    size_t idx = ((r.top + y) * stride + globalX) * 4;

                    // Draw a 2-pixel wide white scanning line if playing
                    if (studio.isPlaying && (globalX == gridStartX + playheadGlobalX || globalX == gridStartX + playheadGlobalX - 1)) {
                        pixels[idx] = 255;
                        pixels[idx + 1] = 255;
                        pixels[idx + 2] = 255;
                    } else {
                        pixels[idx] = c.r;
                        pixels[idx + 1] = c.g;
                        pixels[idx + 2] = c.b;
                    }
                }
            }
        }
    }
}

int main()
{
    snd_pcm_t* pcm_h;
    snd_pcm_open(&pcm_h, "default", SND_PCM_STREAM_PLAYBACK, 0);
    snd_pcm_set_params(pcm_h, SND_PCM_FORMAT_S16_LE, SND_PCM_ACCESS_RW_INTERLEAVED, 2, SAMPLE_RATE, 1, 20000);
    sf::RenderWindow window(sf::VideoMode(1080, 950), "zicBox Studio");
    window.setFramerateLimit(60);

    // RESTORED COLORS
    Styles appStyles = { .screen = { 1080, 950 }, .margin = 2, .colors = { { 15, 15, 18 }, { 255, 255, 255 }, { 120, 120, 130 }, { 0, 180, 255 }, { 10, 10, 12 }, { 28, 28, 32 }, { 35, 35, 40 } } };

    auto drawer = std::make_unique<Draw>(appStyles);
    sf::Texture screenTexture;
    screenTexture.create(BUFFER_SIZE, BUFFER_SIZE);
    sf::Sprite screenSprite(screenTexture);
    std::vector<sf::Uint8> pixelBuffer(BUFFER_SIZE * BUFFER_SIZE * 4, 15);

    bool static_needs_redraw = true;
    std::thread aThread(audio_worker, pcm_h);

    while (window.isOpen() && keep_running) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) window.close();
            if (event.type == sf::Event::Resized) {
                window.setView(sf::View(sf::FloatRect(0, 0, event.size.width, event.size.height)));
                static_needs_redraw = true;
            }
            if (event.type == sf::Event::KeyPressed) {
                if (event.key.code == sf::Keyboard::Space) {
                    studio.isPlaying = !studio.isPlaying;
                    static_needs_redraw = true;
                }
                if (event.key.code >= sf::Keyboard::Num1 && event.key.code <= sf::Keyboard::Num8) {
                    std::lock_guard<std::mutex> lock(studio.audioMutex);
                    studio.tracks[event.key.code - sf::Keyboard::Num1]->engine->noteOn(60, 1.0f);
                }
            }
            if (event.type == sf::Event::MouseButtonPressed) {
                int mx = event.mouseButton.x, my = event.mouseButton.y;
                if (studio.transportRect.contains(mx, my)) {
                    studio.isPlaying = !studio.isPlaying;
                    static_needs_redraw = true;
                }
                for (auto& trk : studio.tracks) {
                    if (trk->muteRect.contains(mx, my)) {
                        trk->isMuted = !trk->isMuted;
                        static_needs_redraw = true;
                    }
                    for (int s = 0; s < SEQ_STEPS; s++)
                        if (trk->stepRects[s].contains(mx, my)) trk->sequence[s].active = !trk->sequence[s].active;
                }
            }
            if (event.type == sf::Event::MouseWheelScrolled) {
                int mx = event.mouseWheelScroll.x, my = event.mouseWheelScroll.y;
                float delta = event.mouseWheelScroll.delta;
                uint32_t now = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now().time_since_epoch()).count();

                if (studio.bpmRect.contains(mx, my)) {
                    int scaled = encGetScaledDirection(delta, now, lastBpmTick);
                    lastBpmTick = now;
                    studio.bpm = CLAMP(studio.bpm + (scaled * (sf::Keyboard::isKeyPressed(sf::Keyboard::LShift) ? 5.0f : 0.5f)), 20.0f, 300.0f);
                    studio.updateClock();
                    static_needs_redraw = true;
                } else {
                    for (auto& trk : studio.tracks) {
                        if (trk->volRect.contains(mx, my)) {
                            int scaled = encGetScaledDirection(delta, now, trk->lastVolShiftTick);
                            trk->lastVolShiftTick = now;
                            trk->volume = CLAMP(trk->volume + (scaled * (sf::Keyboard::isKeyPressed(sf::Keyboard::LShift) ? 0.05f : 0.01f)), 0.0f, 1.0f);
                            static_needs_redraw = true;
                        } else if (trk->trackBounds.contains(mx, my)) {
                            int margin = 10, pPerRow = 8, rowH = 26;
                            int colW = ((int)window.getSize().x - (margin * 2)) / pPerRow;
                            int pIdx = ((my - (trk->trackBounds.top + 14)) / rowH) * pPerRow + (mx - margin) / colW;
                            if (pIdx >= 0 && (size_t)pIdx < trk->engine->getParamCount()) {
                                std::lock_guard<std::mutex> lock(studio.audioMutex);
                                Param& p = trk->engine->getParams()[pIdx];
                                int scaled = encGetScaledDirection(delta, now, trk->lastShiftTicks[pIdx]);
                                trk->lastShiftTicks[pIdx] = now;
                                p.set(p.value + (scaled * p.step * (sf::Keyboard::isKeyPressed(sf::Keyboard::LShift) ? 5.0f : 1.0f)));
                                trk->activeParamIdx = pIdx;
                                trk->lastEditTime = std::chrono::steady_clock::now();
                                static_needs_redraw = true;
                            }
                        }
                    }
                }
            }
        }

        if (static_needs_redraw) {
            sf::Vector2u winSize = window.getSize();
            drawer->setScreenSize({ (int)winSize.x, (int)winSize.y });
            drawStaticUI(*drawer, winSize);
            for (unsigned int y = 0; y < winSize.y; y++) {
                std::memcpy(&pixelBuffer[y * BUFFER_SIZE * 4], drawer->screenBuffer[y], winSize.x * 4);
            }
            static_needs_redraw = false;
        }

        updateWaveforms(pixelBuffer, BUFFER_SIZE);
        updateSequencerPixels(pixelBuffer, BUFFER_SIZE);
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