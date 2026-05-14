// zicRack/studio.h

#pragma once

#include <SFML/Graphics.hpp>
#include <atomic>
#include <deque>
#include <mutex>
#include <iostream> // For std::cout

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
#include "audio/engines/Noise23.h"
#include "draw/draw.h"
#include "helpers/random.h"
#include "zicRack/generator.h"
#include "zicRack/step.h"

static constexpr int MAX_TRACKS = 6;
static constexpr uint32_t SAMPLE_RATE = 44100;
static constexpr int BUFFER_SIZE = 4096;
static constexpr int WAVE_HISTORY = 60;
static constexpr int FX_BUFFERS_PER_TRACK = 3;

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

// ================================================================
// Engine Registry
// ================================================================
struct EngineCreator {
    const char* name;
    TrackType type;
    void (*generate)(std::vector<Step>& sequence);
    // Lambda that takes the track's pre-allocated buffers and returns the engine
    std::function<std::unique_ptr<IEngine>(uint32_t, float**)> create;
};

static const EngineCreator engineRegistry[] = {
    { "Kick", TRACK_TYPE_DRUM, Generator::generateKick, [](uint32_t sr, float** b) { return std::make_unique<DrumKick23>(sr, b[0]); } },
    { "Snare", TRACK_TYPE_DRUM, Generator::generateSnare, [](uint32_t sr, float** b) { return std::make_unique<DrumSnare23>(sr, b[0]); } },
    { "Clap", TRACK_TYPE_DRUM, Generator::generateClap, [](uint32_t sr, float** b) { return std::make_unique<DrumHiClap23>(sr, b[0]); } },
    { "Synth", TRACK_TYPE_SYNTH, Generator::generateBass, [](uint32_t sr, float** b) { return std::make_unique<Synth23>(sr, b[0], b[1], b[2]); } },
    { "Sample", TRACK_TYPE_SYNTH, Generator::generateBass, [](uint32_t sr, float** b) { return std::make_unique<Sample23>(sr, b[0], b[1]); } },
    { "Noise", TRACK_TYPE_SYNTH, Generator::generateBass, [](uint32_t sr, float** b) { return std::make_unique<Noise23>(sr, b[0], b[1], b[2]); } },
};

static const int ENGINE_REGISTRY_COUNT = sizeof(engineRegistry) / sizeof(EngineCreator);

struct Track {
    TrackType type;
    float* fxBuffers[FX_BUFFERS_PER_TRACK];

    std::unique_ptr<IEngine> engine;
    float volume = 0.8f;
    bool isMuted = false;
    Color themeColor;
    std::atomic<float> vumeter;
    sf::IntRect vuRect, trackBounds, muteRect, volRect, headerRect;
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
    uint32_t noteSamplesRemaining = 0;
    uint32_t genLen = 64;

    EQ eq;
    SpectrumAnalyser spectrum;

    sf::IntRect editRect;

    Clip clips[MAX_CLIP_COUNT];
    int activeClipIdx = 0;
    int selectedClipIdx = 0;

    std::vector<std::pair<int, int>> scrollParamIndex;

    Track(float v, Color c)
        : volume(v)
        , themeColor(c)
        , vumeter(0.0f)
    {
        for (int i = 0; i < FX_BUFFERS_PER_TRACK; ++i) {
            fxBuffers[i] = new float[FX_BUFFER_SIZE]();
        }

        history.resize(WAVE_HISTORY, 0.0f);
        sequence.resize(SEQ_STEPS);
        stepRects.resize(SEQ_STEPS);

        // maybe pass sample rate as props
        eq.recompute(SAMPLE_RATE);
        setEngine(0);
    }

    ~Track()
    {
        for (int i = 0; i < FX_BUFFERS_PER_TRACK; ++i) {
            delete[] fxBuffers[i];
        }
    }

    void setEngine(int registryIdx)
    {
        if (registryIdx < 0 || registryIdx >= ENGINE_REGISTRY_COUNT) return;

        const auto& creator = engineRegistry[registryIdx];

        // unique_ptr handles deletion of the old engine automatically
        engine = creator.create(SAMPLE_RATE, fxBuffers);
        type = creator.type;
        generate = creator.generate;

        if (engine) {
            lastShiftTicks.assign(engine->getParamCount(), 0);
        }
    }
};

enum {
    ViewMenu,
    ViewMaster,
    ViewTrack,
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

    // STEP EDITOR STATE
    int selTrack = -1;
    int selStep = -1;
    sf::IntRect editNoteRect, editVeloRect, editProbRect, editLenRect;

    sf::IntRect overlayRect;

    StepEditMode stepEditMode = EDIT_NOTE;

    Scatter masterScatter;
    std::atomic<int> activeScatterMode { 0 };

    MMfilter filter;

    Compressor compressor;

    int pianoRollTrack = -1;

    std::string projectPath = "";

    int currentView = ViewMaster;

    Studio()
    {
        Color palette[8] = { { 0, 200, 255 }, { 255, 100, 100 }, { 100, 255, 100 }, { 255, 200, 50 }, { 200, 100, 255 }, { 50, 255, 200 }, { 255, 150, 50 }, { 180, 180, 180 } };

        for (int i = 0; i < MAX_TRACKS; ++i) {
            auto trk = std::make_unique<Track>(0.7f, palette[i % 8]);

            // Default layout: 0:Kick, 1:Snare, 2:Clap, 3:Synth, 4:Synth, 5:Sample
            // int defaultEngine = (i < 3) ? i : (i < 5 ? 3 : 4);
            int defaultEngine = (i < 3) ? i : (i < 5 ? 3 : 5);
            trk->setEngine(defaultEngine);

            tracks.push_back(std::move(trk));
        }

        updateClock();
    }

    void updateClock() { samplesPerStep = (SAMPLE_RATE * 60.0) / (bpm * 4.0); }

    void changeTrackEngine(int trackIdx, int registryIdx)
    {
        if (trackIdx < 0 || trackIdx >= tracks.size()) return;

        std::lock_guard<std::mutex> lock(audioMutex);
        tracks[trackIdx]->setEngine(registryIdx);
    }
};

Studio studio;
std::atomic<bool> keep_running { true };
Random rnd;