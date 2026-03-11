// tek23/studio.h
#pragma once

#include <SFML/Graphics.hpp>
#include <atomic>
#include <deque>
#include <mutex>

#include "audio/engines/DrumClap.h"
#include "audio/engines/DrumKick23.h"
#include "audio/engines/DrumSnare.h"
#include "audio/engines/DrumSnare23.h"
#include "audio/engines/DrumHiHat23.h"
#include "draw/draw.h"
#include "helpers/random.h"
#include "tek23/step.h"
#include "tek23/generator.h"

static constexpr int MAX_TRACKS = 8;
static constexpr uint32_t SAMPLE_RATE = 44100;
static constexpr int BUFFER_SIZE = 4096;
static constexpr int WAVE_HISTORY = 60;

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
    sf::IntRect genRect;
    void (*generate)(std::vector<Step>& sequence) = nullptr;

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
    }
};

class Studio {
public:
    std::vector<std::unique_ptr<Track>> tracks;
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
    sf::IntRect editNoteRect, editVeloRect, editProbRect;

    Studio()
    {
        Color palette[8] = { { 0, 200, 255 }, { 255, 100, 100 }, { 100, 255, 100 }, { 255, 200, 50 }, { 200, 100, 255 }, { 50, 255, 200 }, { 255, 150, 50 }, { 180, 180, 180 } };
        int i = 0;
        tracks.push_back(std::make_unique<Track>(std::make_unique<DrumKick23>(SAMPLE_RATE, createFxBuffer()), 0.7f, palette[i++], Generator::generateKick));
        tracks.push_back(std::make_unique<Track>(std::make_unique<DrumSnare23>(SAMPLE_RATE, createFxBuffer()), 0.7f, palette[i++], Generator::generateSnare));
        tracks.push_back(std::make_unique<Track>(std::make_unique<DrumHiHat23>(SAMPLE_RATE, createFxBuffer()), 0.7f, palette[i++], Generator::generateHat));
        tracks.push_back(std::make_unique<Track>(std::make_unique<DrumClap>(SAMPLE_RATE, createFxBuffer()), 0.7f, palette[i++], Generator::generateClap));
        tracks.push_back(std::make_unique<Track>(std::make_unique<DrumSnare>(SAMPLE_RATE), 0.7f, palette[i++]));
        tracks.push_back(std::make_unique<Track>(std::make_unique<DrumSnare>(SAMPLE_RATE), 0.7f, palette[i++]));
        tracks.push_back(std::make_unique<Track>(std::make_unique<DrumSnare>(SAMPLE_RATE), 0.7f, palette[i++]));
        tracks.push_back(std::make_unique<Track>(std::make_unique<DrumSnare>(SAMPLE_RATE), 0.7f, palette[i++]));
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