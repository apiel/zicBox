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

struct Track {
    std::unique_ptr<IEngine> engine;
    std::string name;
    float volume;
    Color themeColor;
    std::atomic<float> vumeter;
    sf::IntRect vuRect;
    sf::IntRect trackBounds;

    std::deque<float> history;
    std::mutex historyMtx;

    int activeParamIdx = -1;
    std::chrono::steady_clock::time_point lastEditTime;

    uint32_t lastShiftTicks[100] = { 0 }; // might want to use a vector or something like this instead

    Track(std::unique_ptr<IEngine> e, std::string n, float v, Color c)
        : engine(std::move(e))
        , name(n)
        , volume(v)
        , themeColor(c)
        , vumeter(0.0f)
    {
        history.resize(WAVE_HISTORY, 0.0f);
    }
};

class Studio {
public:
    std::vector<std::unique_ptr<Track>> tracks;
    std::mutex audioMutex;
    float* sharedReverbBuffer;

    Studio()
    {
        sharedReverbBuffer = new float[SAMPLE_RATE * 2]();
        tracks.reserve(MAX_TRACKS);
        Color palette[8] = { { 0, 200, 255 }, { 255, 100, 100 }, { 100, 255, 100 }, { 255, 200, 50 }, { 200, 100, 255 }, { 50, 255, 200 }, { 255, 150, 50 }, { 180, 180, 180 } };
        for (int i = 0; i < 4; i++)
            tracks.push_back(std::make_unique<Track>(std::make_unique<DrumKick23>(SAMPLE_RATE, sharedReverbBuffer), "KICK " + std::to_string(i + 1), 0.7f, palette[i]));
        for (int i = 4; i < 8; i++)
            tracks.push_back(std::make_unique<Track>(std::make_unique<DrumClap>(SAMPLE_RATE, sharedReverbBuffer), "CLAP " + std::to_string(i - 3), 0.7f, palette[i]));
    }
    ~Studio() { delete[] sharedReverbBuffer; }
};

Studio studio;
std::atomic<bool> keep_running { true };

void audio_worker(snd_pcm_t* pcm)
{
    const size_t num_frames = 256;
    std::vector<int16_t> buffer_pcm(num_frames * 2);
    while (keep_running) {
        {
            std::lock_guard<std::mutex> lock(studio.audioMutex);
            std::fill(buffer_pcm.begin(), buffer_pcm.end(), 0);
            for (auto& trk : studio.tracks) {
                float blockPeak = 0.0f;
                for (uint32_t f = 0; f < num_frames; f++) {
                    float s = trk->engine->sample();
                    float absS = std::abs(s);
                    if (absS > blockPeak) blockPeak = absS;
                    float p = s * trk->volume;
                    int16_t sampleVal = (int16_t)(CLAMP(p, -1.0f, 1.0f) * 32767.0f / (MAX_TRACKS / 2));
                    buffer_pcm[f * 2] += sampleVal;
                    buffer_pcm[f * 2 + 1] += sampleVal;
                }
                std::lock_guard<std::mutex> hLock(trk->historyMtx);
                trk->history.push_back(blockPeak);
                trk->history.pop_front();
                trk->vumeter.store(blockPeak);
            }
        }
        snd_pcm_writei(pcm, buffer_pcm.data(), num_frames);
    }
}

void drawStaticUI(Draw& d, sf::Vector2u size)
{
    d.clear();
    int winW = (int)size.x;
    int currentY = 10, margin = 10, paramsPerRow = 8, rowH = 26;
    int colW = (winW - (margin * 2)) / paramsPerRow;

    auto now = std::chrono::steady_clock::now();

    for (auto& trkPtr : studio.tracks) {
        Track& trk = *trkPtr;
        int startY = currentY;

        // Track Header
        d.filledRect({ margin, currentY }, { colW / 2, 12 }, { .color = d.styles.colors.quaternary });
        d.text({ margin + 4, currentY + 1 }, trk.name, 8, { .color = trk.themeColor, .font = &PoppinsLight_8 });

        // VU/Waveform area mapping
        trk.vuRect = sf::IntRect(margin + (colW / 2) + 10, currentY - 2, WAVE_HISTORY, 16);

        currentY += 14;
        Param* params = trk.engine->getParams();
        size_t pCount = trk.engine->getParamCount();

        for (size_t p = 0; p < pCount; p++) {
            int x = margin + ((p % paramsPerRow) * colW);
            int y = currentY + ((p / paramsPerRow) * rowH);

            d.filledRect({ x, y }, { colW - 2, rowH - 2 }, { .color = d.styles.colors.quaternary });

            // Label (Left side)
            d.text({ x + 4, y + 2 }, params[p].label, 12, { .color = d.styles.colors.text, .font = &PoppinsLight_12 });

            // Value Visibility Logic (Threshold 900px)
            bool showValue = true;
            if (winW < 900) {
                auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - trk.lastEditTime).count();
                showValue = (trk.activeParamIdx == (int)p && elapsed < 1500);
            }

            if (showValue) {
                std::string displayVal;

                // NEW: Logic to show p.string if available
                if (params[p].string != nullptr) {
                    displayVal = std::string(params[p].string);
                } else {
                    std::stringstream ss;
                    ss << std::fixed << std::setprecision(1) << params[p].value << params[p].unit;
                    displayVal = ss.str();
                }

                d.textRight({ x + colW - 6, y + 2 }, displayVal, 8, { .color = { 90, 90, 90 }, .font = &PoppinsLight_8 });
            }

            // Progress Bar
            float pct = (params[p].value - params[p].min) / (params[p].max - params[p].min);
            d.filledRect({ x + 4, y + rowH - 8 }, { (int)((colW - 10) * pct), 3 }, { .color = trk.themeColor });
        }

        int sectionHeight = (((pCount + 7) / 8) * rowH) + 14;
        trk.trackBounds = sf::IntRect(margin, startY, winW - (margin * 2), sectionHeight);
        currentY += sectionHeight - 2;
    }
}

void updateWaveforms(std::vector<sf::Uint8>& pixels, int stride)
{
    for (auto& trkPtr : studio.tracks) {
        Track& trk = *trkPtr;
        std::lock_guard<std::mutex> hLock(trk.historyMtx);
        for (int x = 0; x < WAVE_HISTORY; x++) {
            float val = std::min(trk.history[x], 1.0f);
            int halfH = trk.vuRect.height / 2;
            int barHeight = (int)(val * halfH);
            for (int y = 0; y < trk.vuRect.height; y++) {
                size_t idx = ((trk.vuRect.top + y) * stride + (trk.vuRect.left + x)) * 4;
                if (std::abs(y - halfH) <= barHeight) {
                    pixels[idx] = trk.themeColor.r;
                    pixels[idx + 1] = trk.themeColor.g;
                    pixels[idx + 2] = trk.themeColor.b;
                } else {
                    pixels[idx] = 20;
                    pixels[idx + 1] = 20;
                    pixels[idx + 2] = 25;
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

    sf::RenderWindow window(sf::VideoMode(1080, 850), "zicBox Studio");
    window.setFramerateLimit(60);

    Styles appStyles = { .screen = { 1080, 850 }, .margin = 2, .colors = { { 15, 15, 18 }, { 255, 255, 255 }, { 120, 120, 130 }, { 0, 180, 255 }, { 10, 10, 12 }, { 28, 28, 32 }, { 35, 35, 40 } } };
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

            if (event.type == sf::Event::MouseWheelScrolled) {
                int mx = event.mouseWheelScroll.x, my = event.mouseWheelScroll.y;
                float delta = event.mouseWheelScroll.delta;

                for (auto& trkPtr : studio.tracks) {
                    if (trkPtr->trackBounds.contains(mx, my)) {
                        int margin = 10, paramsPerRow = 8, rowH = 26;
                        int colW = ((int)window.getSize().x - (margin * 2)) / paramsPerRow;
                        int localY = my - (trkPtr->trackBounds.top + 14);
                        if (localY >= 0) {
                            int pIdx = (localY / rowH) * paramsPerRow + (mx - margin) / colW;
                            if (pIdx >= 0 && (size_t)pIdx < trkPtr->engine->getParamCount()) {
                                std::lock_guard<std::mutex> lock(studio.audioMutex);
                                Param& p = trkPtr->engine->getParams()[pIdx];
                                // float range = p.max - p.min;
                                // float step = range * 0.02f;
                                // if (sf::Keyboard::isKeyPressed(sf::Keyboard::LShift)) step = range * 0.005f; // Precision mode

                                // p.value = CLAMP(p.value + (delta * step), p.min, p.max);
                                trkPtr->lastEditTime = std::chrono::steady_clock::now();
                                uint32_t currentTick = trkPtr->lastEditTime.time_since_epoch().count();
                                int scaled = encGetScaledDirection(delta, currentTick, trkPtr->lastShiftTicks[pIdx]);
                                trkPtr->lastShiftTicks[pIdx] = currentTick;
                                p.set(p.value + (scaled * p.step));
                                trkPtr->activeParamIdx = pIdx;
                                static_needs_redraw = true;
                            }
                        }
                    }
                }
            }
            if (event.type == sf::Event::KeyPressed && event.key.code >= sf::Keyboard::Num1 && event.key.code <= sf::Keyboard::Num8) {
                std::lock_guard<std::mutex> lock(studio.audioMutex);
                studio.tracks[event.key.code - sf::Keyboard::Num1]->engine->noteOn(60, 1.0f);
            }
        }

        // Auto-hide peek values after 1.5s
        if (window.getSize().x < 900) {
            auto now = std::chrono::steady_clock::now();
            for (auto& trk : studio.tracks) {
                if (trk->activeParamIdx != -1) {
                    auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - trk->lastEditTime).count();
                    if (elapsed > 1500) {
                        trk->activeParamIdx = -1;
                        static_needs_redraw = true;
                    }
                }
            }
        }

        if (static_needs_redraw) {
            sf::Vector2u winSize = window.getSize();
            drawer->setScreenSize({ (int)winSize.x, (int)winSize.y });
            drawStaticUI(*drawer, winSize);
            for (unsigned int y = 0; y < winSize.y; y++) {
                for (unsigned int x = 0; x < winSize.x; x++) {
                    auto& c = drawer->screenBuffer[y][x];
                    size_t idx = (y * BUFFER_SIZE + x) * 4;
                    pixelBuffer[idx] = c.r;
                    pixelBuffer[idx + 1] = c.g;
                    pixelBuffer[idx + 2] = c.b;
                    pixelBuffer[idx + 3] = 255;
                }
            }
            static_needs_redraw = false;
        }

        updateWaveforms(pixelBuffer, BUFFER_SIZE);
        screenTexture.update(pixelBuffer.data());
        window.clear(sf::Color(15, 15, 18));
        window.draw(screenSprite);
        window.display();
    }
    keep_running = false;
    aThread.join();
    snd_pcm_close(pcm_h);
    return 0;
}