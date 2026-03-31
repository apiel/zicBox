// zic23/studio.h

#pragma once

#include <SFML/Graphics.hpp>
#include <atomic>
#include <deque>
#include <mutex>

#define AUDIO_FOLDER std::string("../data/audio")

#include "audio/Compressor.h"
#include "audio/Eq.h"
#include "audio/MMfilter.h"
#include "audio/Scatter.h"
#include "audio/engines/DrumHiClap23.h"
#include "audio/engines/DrumKick23.h"
#include "audio/engines/DrumSnare23.h"
#include "audio/engines/Sample23.h"
#include "audio/engines/Synth23.h"
#include "draw/draw.h"
#include "helpers/random.h"
#include "zic23/generator.h"
#include "zic23/step.h"

static constexpr int MAX_TRACKS = 6;
static constexpr uint32_t SAMPLE_RATE = 44100;
static constexpr int BUFFER_SIZE = 4096;
static constexpr int WAVE_HISTORY = 60;

// ================================================================
// Global layout constants  (used by both draw and event handlers)
// ================================================================
static constexpr int MARGIN = 10;
static constexpr int ROW_H = 26; // param panel row height
static constexpr int TRACK_H = 20; // each track header row height

static constexpr int MAX_CLIP_COUNT = 32;

// ================================================================
// Global UI state
// ================================================================
static bool showHelp = false;
static sf::IntRect helpCloseRect;

enum StepEditMode {
    EDIT_NOTE,
    EDIT_LEN,
    EDIT_VELO,
    EDIT_PROB,
    MODE_COUNT
};

enum TrackType {
    TRACK_TYPE_DRUM,
    TRACK_TYPE_SYNTH,
    TYPE_COUNT
};

struct Clip {
    std::vector<float> paramValues;
    std::vector<Step> sequence;
    bool saved = false;
};

struct Track {
    TrackType type;
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

    sf::IntRect editRect;

    Clip clips[MAX_CLIP_COUNT];
    int activeClipIdx = 0;
    int selectedClipIdx = 0;

    Track(TrackType t, std::unique_ptr<IEngine> e, float v, Color c, void (*gen)(std::vector<Step>& sequence) = nullptr)
        : type(t)
        , engine(std::move(e))
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
    sf::IntRect editNoteRect, editVeloRect, editProbRect, editLenRect;

    StepEditMode stepEditMode = EDIT_NOTE;

    Scatter masterScatter;
    std::atomic<int> activeScatterMode { 0 };

    MMfilter filter;

    Compressor compressor;

    int pianoRollTrack = -1;

    Studio()
    {
        Color palette[8] = { { 0, 200, 255 }, { 255, 100, 100 }, { 100, 255, 100 }, { 255, 200, 50 }, { 200, 100, 255 }, { 50, 255, 200 }, { 255, 150, 50 }, { 180, 180, 180 } };
        int i = 0;
        tracks.push_back(std::make_unique<Track>(TRACK_TYPE_DRUM, std::make_unique<DrumKick23>(SAMPLE_RATE, createFxBuffer()), 0.7f, palette[i++], Generator::generateKick));
        tracks.push_back(std::make_unique<Track>(TRACK_TYPE_DRUM, std::make_unique<DrumSnare23>(SAMPLE_RATE, createFxBuffer()), 0.7f, palette[i++], Generator::generateSnare));
        tracks.push_back(std::make_unique<Track>(TRACK_TYPE_DRUM, std::make_unique<DrumHiClap23>(SAMPLE_RATE, createFxBuffer()), 0.7f, palette[i++], Generator::generateClap));
        tracks.push_back(std::make_unique<Track>(TRACK_TYPE_SYNTH, std::make_unique<Synth23>(SAMPLE_RATE, createFxBuffer(), createFxBuffer()), 0.7f, palette[i++], Generator::generateBass));
        tracks.push_back(std::make_unique<Track>(TRACK_TYPE_SYNTH, std::make_unique<Synth23>(SAMPLE_RATE, createFxBuffer(), createFxBuffer()), 0.7f, palette[i++], Generator::generateBass));
        tracks.push_back(std::make_unique<Track>(TRACK_TYPE_SYNTH, std::make_unique<Sample23>(SAMPLE_RATE, createFxBuffer(), createFxBuffer()), 0.7f, palette[i++], Generator::generateBass));
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