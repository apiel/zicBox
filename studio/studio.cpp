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
static constexpr int SEQ_STEPS = 64;

struct Step {
    bool active = false;
    int note = 60;
    float velocity = 0.8f;
    float probability = 1.0f;
};

struct Track {
    std::unique_ptr<IEngine> engine;
    std::string name;
    float volume = 0.7f;
    bool isMuted = false;
    Color themeColor;
    
    sf::IntRect trackBounds; 
    sf::IntRect muteRect;
    sf::IntRect volRect;
    std::vector<sf::IntRect> stepRects;

    std::vector<Step> sequence;
    int activeParamIdx = -1;
    std::chrono::steady_clock::time_point lastEditTime;
    std::vector<uint32_t> lastShiftTicks;

    Track(std::unique_ptr<IEngine> e, std::string n, Color c)
        : engine(std::move(e)), name(n), themeColor(c) {
        sequence.resize(SEQ_STEPS);
        stepRects.resize(SEQ_STEPS);
        lastShiftTicks.resize(engine->getParamCount(), 0);
    }
};

class Studio {
public:
    std::vector<std::unique_ptr<Track>> tracks;
    std::mutex audioMutex;
    float* sharedReverbBuffer;
    std::atomic<float> bpm { 120.0f };
    std::atomic<int> currentStep { 0 };
    double samplesPerStep = 0;
    double sampleCounter = 0;

    Studio() {
        sharedReverbBuffer = new float[SAMPLE_RATE * 2]();
        Color palette[8] = {{0,200,255},{255,100,100},{100,255,100},{255,200,50},{200,100,255},{50,255,200},{255,150,50},{180,180,180}};
        for (int i=0; i<4; i++) tracks.push_back(std::make_unique<Track>(std::make_unique<DrumKick23>(SAMPLE_RATE, sharedReverbBuffer), "KICK "+std::to_string(i+1), palette[i]));
        for (int i=4; i<8; i++) tracks.push_back(std::make_unique<Track>(std::make_unique<DrumClap>(SAMPLE_RATE, sharedReverbBuffer), "CLAP "+std::to_string(i-3), palette[i]));
        updateClock();
    }
    void updateClock() { samplesPerStep = (SAMPLE_RATE * 60.0) / (bpm * 4.0); }
    ~Studio() { delete[] sharedReverbBuffer; }
};

Studio studio;
std::atomic<bool> keep_running { true };

void audio_worker(snd_pcm_t* pcm) {
    const size_t num_frames = 256;
    std::vector<int16_t> buffer_pcm(num_frames * 2);
    while (keep_running) {
        {
            std::lock_guard<std::mutex> lock(studio.audioMutex);
            std::fill(buffer_pcm.begin(), buffer_pcm.end(), 0);
            for (uint32_t f = 0; f < num_frames; f++) {
                studio.sampleCounter++;
                if (studio.sampleCounter >= studio.samplesPerStep) {
                    studio.sampleCounter = 0;
                    studio.currentStep = (studio.currentStep + 1) % SEQ_STEPS;
                    for (auto& trk : studio.tracks) {
                        auto& step = trk->sequence[studio.currentStep];
                        if (step.active && !trk->isMuted) {
                            if ((float)rand()/RAND_MAX <= step.probability) trk->engine->noteOn(step.note, step.velocity);
                        }
                    }
                }
                for (auto& trk : studio.tracks) {
                    float s = trk->engine->sample() * (trk->isMuted ? 0.0f : trk->volume);
                    int16_t val = (int16_t)(CLAMP(s, -1.0f, 1.0f) * 32767.0f / (MAX_TRACKS / 2));
                    buffer_pcm[f * 2] += val; buffer_pcm[f * 2 + 1] += val;
                }
            }
        }
        snd_pcm_writei(pcm, buffer_pcm.data(), num_frames);
    }
}

void drawUI(Draw& d, sf::Vector2u size) {
    d.clear();
    int winW = (int)size.x, margin = 10;
    int currentY = 10, rowH = 26;
    auto now = std::chrono::steady_clock::now();

    // 1. Parameters Rack
    for (auto& trkPtr : studio.tracks) {
        Track& trk = *trkPtr;
        int startY = currentY;
        d.text({ margin, currentY }, trk.name, 8, { .color = trk.themeColor, .font = &PoppinsLight_8 });
        currentY += 14;

        Param* params = trk.engine->getParams();
        int paramsPerRow = 8;
        int colW = (winW - (margin * 2)) / paramsPerRow;
        for (size_t p = 0; p < trk.engine->getParamCount(); p++) {
            int x = margin + ((p % paramsPerRow) * colW);
            int y = currentY + ((p / paramsPerRow) * rowH);
            d.filledRect({ x, y }, { colW - 2, rowH - 2 }, { .color = d.styles.colors.quaternary });
            d.text({ x + 4, y + 2 }, params[p].label, 12, { .color = d.styles.colors.text, .font = &PoppinsLight_12 });

            if (winW >= 900 || (trk.activeParamIdx == (int)p && std::chrono::duration_cast<std::chrono::milliseconds>(now - trk.lastEditTime).count() < 1500)) {
                std::string dv;
                if (params[p].string) dv = params[p].string;
                else {
                    std::stringstream ss;
                    ss << std::fixed << std::setprecision(1) << params[p].value << params[p].unit;
                    dv = ss.str();
                }
                d.textRight({ x + colW - 6, y + 2 }, dv, 8, { .color = { 90, 90, 90 }, .font = &PoppinsLight_8 });
            }
            float pct = (params[p].value - params[p].min) / (params[p].max - params[p].min);
            d.filledRect({ x + 4, y + rowH - 8 }, { (int)((colW - 10) * pct), 3 }, { .color = trk.themeColor });
        }
        int sectionH = (((trk.engine->getParamCount() + 7) / 8) * rowH) + 14;
        trk.trackBounds = { margin, startY, winW - (margin * 2), sectionH };
        currentY += sectionH;
    }

    // 2. Sequencer Grid (Non-sticky, just follows)
    currentY += 20; 
    d.filledRect({0, currentY - 10}, {winW, 1}, {.color = {60,60,65}});
    
    int mixerWidth = 120;
    int stepW = (winW - (margin * 2 + mixerWidth)) / 64;
    int stepH = 14;

    for (int i = 0; i < MAX_TRACKS; i++) {
        Track& trk = *studio.tracks[i];
        int ty = currentY + (i * (stepH + 4));

        trk.muteRect = { margin, ty, 25, stepH };
        d.filledRect({ trk.muteRect.left, trk.muteRect.top }, { trk.muteRect.width, trk.muteRect.height }, { .color = trk.isMuted ? Color{200, 50, 50} : Color{40, 40, 45} });
        d.text({ trk.muteRect.left + 8, trk.muteRect.top + 1 }, "M", 8, { .color = {255,255,255}, .font = &PoppinsLight_8 });

        trk.volRect = { margin + 30, ty + (stepH/2) - 2, 70, 4 };
        d.filledRect({ trk.volRect.left, trk.volRect.top }, { trk.volRect.width, trk.volRect.height }, { .color = {30,30,35} });
        d.filledRect({ trk.volRect.left, trk.volRect.top }, { (int)(trk.volRect.width * trk.volume), trk.volRect.height }, { .color = trk.themeColor });

        for (int s = 0; s < SEQ_STEPS; s++) {
            int tx = margin + mixerWidth + (s * stepW);
            trk.stepRects[s] = { tx, ty, stepW - 1, stepH };
            Color c = trk.sequence[s].active ? trk.themeColor : Color{25, 25, 30};
            if (studio.currentStep == s) c = { 255, 255, 255 };
            else if (!trk.sequence[s].active && s % 4 == 0) c = Color{35, 35, 40};
            d.filledRect({ tx, ty }, { stepW - 1, stepH }, { .color = c });
        }
    }
}

int main() {
    snd_pcm_t* pcm_h;
    snd_pcm_open(&pcm_h, "default", SND_PCM_STREAM_PLAYBACK, 0);
    snd_pcm_set_params(pcm_h, SND_PCM_FORMAT_S16_LE, SND_PCM_ACCESS_RW_INTERLEAVED, 2, SAMPLE_RATE, 1, 20000);

    sf::RenderWindow window(sf::VideoMode(1400, 1050), "zicBox Studio", sf::Style::Default);
    window.setFramerateLimit(60);

    Styles appStyles = { .screen = { 1400, 1050 }, .margin = 2, .colors = { { 15, 15, 18 }, { 255, 255, 255 }, { 120, 120, 130 }, { 0, 180, 255 }, { 10, 10, 12 }, { 28, 28, 32 }, { 35, 35, 40 } } };
    auto drawer = std::make_unique<Draw>(appStyles);

    sf::Texture screenTexture;
    sf::Sprite screenSprite;
    std::vector<sf::Uint8> pixelBuffer;

    auto resizeBuffers = [&](unsigned int w, unsigned int h) {
        if (w == 0 || h == 0) return;
        screenTexture.create(w, h);
        screenSprite.setTexture(screenTexture, true);
        pixelBuffer.assign(w * h * 4, 0);
        drawer->setScreenSize({ (int)w, (int)h });
    };

    resizeBuffers(window.getSize().x, window.getSize().y);
    std::thread aThread(audio_worker, pcm_h);

    while (window.isOpen() && keep_running) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) window.close();
            
            if (event.type == sf::Event::Resized) {
                sf::FloatRect visibleArea(0, 0, event.size.width, event.size.height);
                window.setView(sf::View(visibleArea));
                resizeBuffers(event.size.width, event.size.height);
            }

            if (event.type == sf::Event::MouseButtonPressed) {
                int mx = event.mouseButton.x, my = event.mouseButton.y;
                for (auto& trk : studio.tracks) {
                    if (trk->muteRect.contains(mx, my)) trk->isMuted = !trk->isMuted;
                    if (trk->volRect.contains(mx, my)) trk->volume = CLAMP((float)(mx - trk->volRect.left) / trk->volRect.width, 0.0f, 1.0f);
                    for (int s = 0; s < SEQ_STEPS; s++) {
                        if (trk->stepRects[s].contains(mx, my)) trk->sequence[s].active = !trk->sequence[s].active;
                    }
                }
            }
            
            if (event.type == sf::Event::MouseWheelScrolled) {
                int mx = event.mouseWheelScroll.x, my = event.mouseWheelScroll.y;
                float delta = event.mouseWheelScroll.delta;
                for (auto& trk : studio.tracks) {
                    if (trk->trackBounds.contains(mx, my)) {
                        int margin = 10, paramsPerRow = 8, rowH = 26;
                        int colW = ((int)window.getSize().x - (margin * 2)) / paramsPerRow;
                        int localY = my - (trk->trackBounds.top + 14);
                        if (localY >= 0) {
                            int pIdx = (localY / rowH) * paramsPerRow + (mx - margin) / colW;
                            if (pIdx >= 0 && (size_t)pIdx < trk->engine->getParamCount()) {
                                std::lock_guard<std::mutex> lock(studio.audioMutex);
                                Param& p = trk->engine->getParams()[pIdx];
                                float mult = sf::Keyboard::isKeyPressed(sf::Keyboard::LShift) ? 5.0f : 1.0f;
                                uint32_t currentTick = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now().time_since_epoch()).count();
                                int scaled = encGetScaledDirection(delta, currentTick, trk->lastShiftTicks[pIdx]);
                                trk->lastShiftTicks[pIdx] = currentTick;
                                p.set(p.value + (scaled * p.step * mult));
                                trk->activeParamIdx = pIdx;
                                trk->lastEditTime = std::chrono::steady_clock::now();
                            }
                        }
                    }
                }
            }
        }

        sf::Vector2u winSize = window.getSize();
        drawUI(*drawer, winSize);

        for (unsigned int y = 0; y < winSize.y; y++) {
            for (unsigned int x = 0; x < winSize.x; x++) {
                auto& c = drawer->screenBuffer[y][x];
                size_t idx = (y * winSize.x + x) * 4;
                pixelBuffer[idx]     = c.r;
                pixelBuffer[idx + 1] = c.g;
                pixelBuffer[idx + 2] = c.b;
                pixelBuffer[idx + 3] = 255;
            }
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