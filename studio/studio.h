// studio/studio.h
#pragma once

#include <SFML/Graphics.hpp>
#include <atomic>
#include <mutex>
#include <deque>

#include "draw/draw.h"
#include "audio/engines/DrumClap.h"
#include "audio/engines/DrumKick23.h"
#include "helpers/random.h"

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

    Track(std::unique_ptr<IEngine> e, float v, Color c)
        : engine(std::move(e))
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

    // STEP EDITOR STATE
    int selTrack = -1;
    int selStep = -1;
    sf::IntRect editNoteRect, editVeloRect, editProbRect;

    Studio()
    {
        sharedReverbBuffer = new float[SAMPLE_RATE * 2]();
        Color palette[8] = { { 0, 200, 255 }, { 255, 100, 100 }, { 100, 255, 100 }, { 255, 200, 50 }, { 200, 100, 255 }, { 50, 255, 200 }, { 255, 150, 50 }, { 180, 180, 180 } };
        for (int i = 0; i < 4; i++)
            tracks.push_back(std::make_unique<Track>(std::make_unique<DrumKick23>(SAMPLE_RATE, sharedReverbBuffer), 0.7f, palette[i]));
        for (int i = 4; i < 8; i++)
            tracks.push_back(std::make_unique<Track>(std::make_unique<DrumClap>(SAMPLE_RATE, sharedReverbBuffer), 0.7f, palette[i]));
        updateClock();
    }
    void updateClock() { samplesPerStep = (SAMPLE_RATE * 60.0) / (bpm * 4.0); }
    ~Studio() { delete[] sharedReverbBuffer; }
};

Studio studio;
std::atomic<bool> keep_running { true };
uint32_t lastBpmTick = 0;
Random rnd;