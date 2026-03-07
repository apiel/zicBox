#include <SFML/Graphics.hpp>
#include <alsa/asoundlib.h>
#include <atomic>
#include <chrono>
#include <cstring>
#include <iostream>
#include <memory>
#include <mutex>
#include <thread>
#include <vector>

// Engines and Helpers
#include "audio/engines/DrumClap.h"
#include "audio/engines/DrumKick23.h"
#include "draw/draw.h"
#include "helpers/clamp.h"

static constexpr int MAX_TRACKS = 8;
static constexpr uint32_t SAMPLE_RATE = 44100;
static constexpr int BUFFER_SIZE = 4096;

struct Track {
    std::unique_ptr<IEngine> engine;
    std::string name;
    float volume = 0.7f;
    Color themeColor; // Custom color per track
};

class Studio {
public:
    std::vector<Track> tracks;
    std::mutex audioMutex;
    float* sharedReverbBuffer;

    Studio()
    {
        sharedReverbBuffer = new float[SAMPLE_RATE * 2]();
        tracks.reserve(MAX_TRACKS);

        // Simple color palette for the 8 tracks
        Color palette[8] = {
            { 0, 200, 255 }, { 255, 100, 100 }, { 100, 255, 100 }, { 255, 200, 50 },
            { 200, 100, 255 }, { 50, 255, 200 }, { 255, 150, 50 }, { 180, 180, 180 }
        };

        for (int i = 0; i < 4; i++) {
            tracks.push_back({ std::make_unique<DrumKick23>(SAMPLE_RATE, sharedReverbBuffer),
                "KICK " + std::to_string(i + 1), 0.7f, palette[i] });
        }
        for (int i = 4; i < 8; i++) {
            tracks.push_back({ std::make_unique<DrumClap>(SAMPLE_RATE, sharedReverbBuffer),
                "CLAP " + std::to_string(i - 3), 0.7f, palette[i] });
        }
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
            for (uint32_t f = 0; f < num_frames; f++) {
                float mixL = 0.0f, mixR = 0.0f;
                for (auto& trk : studio.tracks) {
                    float s = trk.engine->sample() * trk.volume;
                    mixL += s;
                    mixR += s;
                }
                buffer_pcm[f * 2] = (int16_t)(CLAMP(mixL, -1.0f, 1.0f) * 32767.0f);
                buffer_pcm[f * 2 + 1] = (int16_t)(CLAMP(mixR, -1.0f, 1.0f) * 32767.0f);
            }
        }
        snd_pcm_sframes_t frames = snd_pcm_writei(pcm, buffer_pcm.data(), num_frames);
        if (frames < 0) snd_pcm_recover(pcm, frames, 0);
    }
}

// --- Horizontal "Rack" Rendering ---
void drawApp(Draw& d, sf::Vector2u size)
{
    d.clear();
    int winW = (int)size.x;

    int currentY = 5;
    int margin = 5;
    int trackPadding = 8;
    int paramsPerRow = 8;
    int colW = (winW - (margin * 2)) / paramsPerRow;
    int rowH = 28; // Height for one row of params

    for (int t = 0; t < MAX_TRACKS; t++) {
        auto& trk = studio.tracks[t];
        Param* params = trk.engine->getParams();
        size_t pCount = trk.engine->getParamCount();
        int numRows = (pCount + paramsPerRow - 1) / paramsPerRow;

        // 1. Small Track Header
        d.filledRect({ margin, currentY }, { winW - (margin * 2), 18 }, { .color = d.styles.colors.secondary });
        d.text({ margin + 4, currentY + 1 }, trk.name, 12, { .color = trk.themeColor, .font = &PoppinsLight_12 });
        currentY += 16;

        // 2. Parameter Grid for this track
        for (size_t p = 0; p < pCount; p++) {
            int localCol = p % paramsPerRow;
            int localRow = p / paramsPerRow;

            int x = margin + (localCol * colW);
            int y = currentY + (localRow * rowH);

            // Parameter Box
            d.filledRect({ x, y }, { colW - 2, rowH - 2 }, { .color = d.styles.colors.quaternary });

            // Label (Small)
            d.text({ x + 4, y + 2 }, params[p].label, 12, { .color = d.styles.colors.text, .font = &PoppinsLight_12 });

            // Visual Value Bar (using the track's theme color)
            float pct = (params[p].value - params[p].min) / (params[p].max - params[p].min);
            d.filledRect({ x + 4, y + rowH - 8 }, { (int)((colW - 10) * pct), 3 }, { .color = trk.themeColor });
        }

        // Advance Y for the next track
        currentY += (numRows * rowH) + trackPadding;
    }
}

int main()
{
    snd_pcm_t* pcm_h;
    if (snd_pcm_open(&pcm_h, "default", SND_PCM_STREAM_PLAYBACK, 0) < 0) return 1;
    snd_pcm_set_params(pcm_h, SND_PCM_FORMAT_S16_LE, SND_PCM_ACCESS_RW_INTERLEAVED, 2, SAMPLE_RATE, 1, 20000);

    sf::RenderWindow window(sf::VideoMode(1080, 850), "zicBox Rack View", sf::Style::Default);
    window.setFramerateLimit(60);

    Styles appStyles = {
        .screen = { 1080, 850 },
        .margin = 2,
        .colors = { { 15, 15, 18 }, { 255, 255, 255 }, { 120, 120, 130 }, { 0, 180, 255 }, { 10, 10, 12 }, { 28, 28, 32 }, { 35, 35, 40 } }
    };

    auto drawer = std::make_unique<Draw>(appStyles);
    sf::Texture screenTexture;
    screenTexture.create(BUFFER_SIZE, BUFFER_SIZE);
    sf::Sprite screenSprite(screenTexture);
    std::vector<sf::Uint8> pixelBuffer(BUFFER_SIZE * BUFFER_SIZE * 4, 255);

    bool needs_refresh = true;
    std::thread aThread(audio_worker, pcm_h);

    while (window.isOpen() && keep_running) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) window.close();
            if (event.type == sf::Event::MouseButtonPressed || event.type == sf::Event::KeyPressed) needs_refresh = true;

            // Trigger sound on Keys 1-8
            if (event.type == sf::Event::KeyPressed && event.key.code >= sf::Keyboard::Num1 && event.key.code <= sf::Keyboard::Num8) {
                int idx = event.key.code - sf::Keyboard::Num1;
                std::lock_guard<std::mutex> lock(studio.audioMutex);
                studio.tracks[idx].engine->noteOn(60, 1.0f);
            }
        }

        if (needs_refresh) {
            drawApp(*drawer, window.getSize());
            unsigned int curW = window.getSize().x, curH = window.getSize().y;
            for (unsigned int y = 0; y < curH; y++) {
                for (unsigned int x = 0; x < curW; x++) {
                    auto& c = drawer->screenBuffer[y][x];
                    size_t idx = (y * BUFFER_SIZE + x) * 4;
                    pixelBuffer[idx] = c.r;
                    pixelBuffer[idx + 1] = c.g;
                    pixelBuffer[idx + 2] = c.b;
                }
            }
            screenTexture.update(pixelBuffer.data());
            needs_refresh = false;
        }

        window.clear();
        screenSprite.setTextureRect(sf::IntRect(0, 0, window.getSize().x, window.getSize().y));
        window.draw(screenSprite);
        window.display();
        if (!needs_refresh) std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }

    keep_running = false;
    if (aThread.joinable()) aThread.join();
    snd_pcm_close(pcm_h);
    return 0;
}