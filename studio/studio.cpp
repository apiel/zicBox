#include <SFML/Graphics.hpp>
#include <algorithm>
#include <alsa/asoundlib.h>
#include <atomic>
#include <chrono>
#include <cmath>
#include <cstring>
#include <deque>
#include <iostream>
#include <memory>
#include <mutex>
#include <thread>
#include <vector>

#include "audio/engines/DrumClap.h"
#include "audio/engines/DrumKick23.h"
#include "draw/draw.h"
#include "helpers/clamp.h"

static constexpr int MAX_TRACKS = 8;
static constexpr uint32_t SAMPLE_RATE = 44100;
static constexpr int BUFFER_SIZE = 4096;
static constexpr int WAVE_HISTORY = 60; // How many "frames" of history to show

struct Track {
    std::unique_ptr<IEngine> engine;
    std::string name;
    float volume;
    Color themeColor;
    std::atomic<float> vumeter;
    sf::IntRect vuRect;

    // Waveform history: Mutex protected because audio writes, UI reads
    std::deque<float> history;
    std::mutex historyMtx;

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
            for (auto& trk : studio.tracks) {
                float blockPeak = 0.0f;
                for (uint32_t f = 0; f < num_frames; f++) {
                    float s = trk->engine->sample();
                    float absS = std::abs(s);
                    if (absS > blockPeak) blockPeak = absS;

                    float p = s * trk->volume;
                    if (trk == studio.tracks[0]) { // Mix down logic
                        buffer_pcm[f * 2] = 0;
                        buffer_pcm[f * 2 + 1] = 0;
                    }
                    buffer_pcm[f * 2] += (int16_t)(CLAMP(p, -1.0f, 1.0f) * 32767.0f / MAX_TRACKS);
                    buffer_pcm[f * 2 + 1] += (int16_t)(CLAMP(p, -1.0f, 1.0f) * 32767.0f / MAX_TRACKS);
                }

                // Update History for Waveform
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
    int currentY = 10, margin = 10, paramsPerRow = 8, rowH = 26;
    int colW = ((int)size.x - (margin * 2)) / paramsPerRow;

    for (auto& trkPtr : studio.tracks) {
        Track& trk = *trkPtr;
        d.filledRect({ margin, currentY }, { colW / 2, 12 }, { .color = d.styles.colors.quaternary });
        d.text({ margin + 4, currentY + 1 }, trk.name, 8, { .color = trk.themeColor, .font = &PoppinsLight_8 });

        // Save larger Rect for Waveform (Higher and wider)
        trk.vuRect = sf::IntRect(margin + (colW / 2) + 10, currentY - 2, WAVE_HISTORY, 16);

        currentY += 14;
        Param* params = trk.engine->getParams();
        size_t pCount = trk.engine->getParamCount();
        for (size_t p = 0; p < pCount; p++) {
            int x = margin + ((p % paramsPerRow) * colW);
            int y = currentY + ((p / paramsPerRow) * rowH);
            d.filledRect({ x, y }, { colW - 2, rowH - 2 }, { .color = d.styles.colors.quaternary });
            d.text({ x + 4, y + 2 }, params[p].label, 12, { .color = d.styles.colors.text, .font = &PoppinsLight_12 });
            float pct = (params[p].value - params[p].min) / (params[p].max - params[p].min);
            d.filledRect({ x + 4, y + rowH - 8 }, { (int)((colW - 10) * pct), 3 }, { .color = trk.themeColor });
        }
        currentY += (((pCount + 7) / 8) * rowH) + 12;
    }
}

// Low-impact waveform renderer
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

                // Draw a symmetric waveform from the center
                if (std::abs(y - halfH) <= barHeight) {
                    pixels[idx] = trk.themeColor.r;
                    pixels[idx + 1] = trk.themeColor.g;
                    pixels[idx + 2] = trk.themeColor.b;
                } else {
                    pixels[idx] = 20;
                    pixels[idx + 1] = 20;
                    pixels[idx + 2] = 25; // Dark background
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

    sf::RenderWindow window(sf::VideoMode(1080, 850), "zicBox Rack View");
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
            if (event.type == sf::Event::KeyPressed && event.key.code >= sf::Keyboard::Num1 && event.key.code <= sf::Keyboard::Num8) {
                std::lock_guard<std::mutex> lock(studio.audioMutex);
                studio.tracks[event.key.code - sf::Keyboard::Num1]->engine->noteOn(60, 1.0f);
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