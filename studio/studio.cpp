#include <SFML/Graphics.hpp>
#include <alsa/asoundlib.h>
#include <atomic>
#include <memory>
#include <mutex>
#include <vector>
#include <thread>
#include <iostream>
#include <chrono>

// Engines and Helpers
#include "audio/engines/DrumClap.h"
#include "audio/engines/DrumKick23.h"
#include "draw/draw.h"
#include "helpers/clamp.h"

// --- Configuration ---
static constexpr int MAX_TRACKS = 8;
static constexpr uint32_t SAMPLE_RATE = 44100;
static constexpr int BUFFER_SIZE = 4096; // Matches your software renderer size

// --- Track & Studio State ---
struct Track {
    std::unique_ptr<IEngine> engine;
    std::string name;
    float volume = 0.7f;
    bool muted = false;
};

class Studio {
public:
    std::vector<Track> tracks;
    int selectedTrack = 0;
    std::mutex audioMutex;
    float* sharedReverbBuffer;

    Studio() {
        sharedReverbBuffer = new float[SAMPLE_RATE * 2](); 
        tracks.reserve(MAX_TRACKS);
        for (int i = 0; i < 4; i++) {
            tracks.push_back({ std::make_unique<DrumKick23>(SAMPLE_RATE, sharedReverbBuffer), "KICK " + std::to_string(i + 1) });
        }
        for (int i = 0; i < 4; i++) {
            tracks.push_back({ std::make_unique<DrumClap>(SAMPLE_RATE, sharedReverbBuffer), "CLAP " + std::to_string(i + 1) });
        }
    }
    ~Studio() { delete[] sharedReverbBuffer; }
};

Studio studio;
std::atomic<bool> keep_running { true };

// --- Audio Worker (ALSA) ---
void audio_worker(snd_pcm_t* pcm) {
    const size_t num_frames = 256;
    std::vector<int16_t> buffer_pcm(num_frames * 2);

    while (keep_running) {
        {
            std::lock_guard<std::mutex> lock(studio.audioMutex);
            for (uint32_t f = 0; f < num_frames; f++) {
                float mixL = 0.0f, mixR = 0.0f;
                for (auto& trk : studio.tracks) {
                    if (trk.muted) continue;
                    float s = trk.engine->sample() * trk.volume;
                    mixL += s; mixR += s;
                }
                buffer_pcm[f * 2] = (int16_t)(CLAMP(mixL, -1.0f, 1.0f) * 32767.0f);
                buffer_pcm[f * 2 + 1] = (int16_t)(CLAMP(mixR, -1.0f, 1.0f) * 32767.0f);
            }
        }
        snd_pcm_sframes_t frames = snd_pcm_writei(pcm, buffer_pcm.data(), num_frames);
        if (frames < 0) snd_pcm_recover(pcm, frames, 0);
    }
}

// --- UI Rendering Logic ---
void drawApp(Draw& d, sf::Vector2u size) {
    d.clear();
    int winW = (int)size.x;
    int winH = (int)size.y;

    int sidebarW = 200;
    int footerH = 50;
    int mainW = winW - sidebarW;
    int mainH = winH - footerH;

    // 1. Sidebar
    d.filledRect({ 0, 0 }, { sidebarW, winH }, { .color = d.styles.colors.secondary });
    for (int i = 0; i < MAX_TRACKS; i++) {
        int trackH = mainH / MAX_TRACKS;
        int y = i * trackH;
        bool isSelected = (i == studio.selectedTrack);

        if (isSelected) d.filledRect({ 5, y + 5 }, { sidebarW - 10, trackH - 10 }, { .color = d.styles.colors.quaternary });
        d.text({ 15, y + 15 }, studio.tracks[i].name, 16, { .color = isSelected ? d.styles.colors.primary : d.styles.colors.text, .font = &PoppinsLight_16 });
        d.filledRect({ 15, y + 45 }, { (int)((sidebarW - 40) * studio.tracks[i].volume), 4 }, { .color = d.styles.colors.primary });
    }

    // 2. Parameter Grid
    auto& activeTrk = studio.tracks[studio.selectedTrack];
    Param* params = activeTrk.engine->getParams();
    int cols = 4, rows = 3, gap = 15;
    int cardW = (mainW - (gap * (cols + 1))) / cols;
    int cardH = (mainH - (gap * (rows + 1))) / rows;

    for (int i = 0; i < 12; i++) {
        int c = i % cols, r = i / cols;
        Point pos = { sidebarW + gap + (c * (cardW + gap)), gap + (r * (cardH + gap)) };
        d.filledRect(pos, { cardW, cardH }, { .color = d.styles.colors.quaternary });
        d.text({ pos.x + 12, pos.y + 12 }, params[i].label, 13, { .color = d.styles.colors.text });
        float pct = (params[i].value - params[i].min) / (params[i].max - params[i].min);
        d.filledRect({ pos.x + 12, pos.y + cardH - 30 }, { (int)((cardW - 24) * pct), 10 }, { .color = d.styles.colors.primary });
    }

    // 3. Footer
    d.line({ 0, mainH }, { winW, mainH });
    d.text({ sidebarW + 20, mainH + 15 }, "ENGINE: " + std::string(activeTrk.engine->getName()), 12, { .font = &PoppinsLight_12 });
}

int main() {
    // ALSA Setup
    snd_pcm_t* pcm_h;
    if (snd_pcm_open(&pcm_h, "default", SND_PCM_STREAM_PLAYBACK, 0) < 0) return 1;
    snd_pcm_set_params(pcm_h, SND_PCM_FORMAT_S16_LE, SND_PCM_ACCESS_RW_INTERLEAVED, 2, SAMPLE_RATE, 1, 20000);

    // SFML Setup
    sf::RenderWindow window(sf::VideoMode(1080, 720), "zicBox Studio", sf::Style::Default);
    window.setFramerateLimit(60); 

    Styles appStyles = {
        .screen = { 1080, 720 },
        .margin = 10,
        .colors = { {30,30,35}, {255,255,255}, {180,180,190}, {0,200,255}, {20,20,25}, {40,40,45}, {50,50,55} }
    };

    auto drawer = std::make_unique<Draw>(appStyles);
    
    sf::Texture screenTexture;
    screenTexture.create(BUFFER_SIZE, BUFFER_SIZE);
    sf::Sprite screenSprite(screenTexture);
    
    // Pixel buffer for SFML texture updates (RGBA)
    std::vector<sf::Uint8> pixelBuffer(BUFFER_SIZE * BUFFER_SIZE * 4, 255);

    bool needs_refresh = true;
    std::thread aThread(audio_worker, pcm_h);

    while (window.isOpen() && keep_running) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) window.close();
            
            // Interaction triggers redraw
            if (event.type == sf::Event::MouseButtonPressed || event.type == sf::Event::KeyPressed) {
                needs_refresh = true;
            }

            if (event.type == sf::Event::MouseButtonPressed) {
                if (event.mouseButton.x < 200) {
                    studio.selectedTrack = CLAMP(event.mouseButton.y / (window.getSize().y / MAX_TRACKS), 0, 7);
                }
            }

            if (event.type == sf::Event::KeyPressed) {
                if (event.key.code == sf::Keyboard::Space) {
                    std::lock_guard<std::mutex> lock(studio.audioMutex);
                    studio.tracks[studio.selectedTrack].engine->noteOn(60, 1.0f);
                }
            }
        }

        if (needs_refresh) {
            drawApp(*drawer, window.getSize());

            // Blit custom software buffer to SFML pixel buffer
            unsigned int currentW = window.getSize().x;
            unsigned int currentH = window.getSize().y;

            for (unsigned int y = 0; y < currentH; y++) {
                for (unsigned int x = 0; x < currentW; x++) {
                    auto& c = drawer->screenBuffer[y][x];
                    size_t idx = (y * BUFFER_SIZE + x) * 4;
                    pixelBuffer[idx]     = c.r;
                    pixelBuffer[idx + 1] = c.g;
                    pixelBuffer[idx + 2] = c.b;
                    // pixelBuffer[idx + 3] is already 255
                }
            }
            screenTexture.update(pixelBuffer.data());
            needs_refresh = false;
        }

        window.clear();
        screenSprite.setTextureRect(sf::IntRect(0, 0, window.getSize().x, window.getSize().y));
        window.draw(screenSprite);
        window.display();

        // Small sleep to keep the thread from spinning when idle
        if (!needs_refresh) {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
    }

    keep_running = false;
    if (aThread.joinable()) aThread.join();
    snd_pcm_close(pcm_h);
    return 0;
}