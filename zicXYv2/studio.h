// zicXYv2/studio.h

#pragma once

#include <atomic>
#include <deque>
#include <mutex>
#include <iostream> // For std::cout

#define AUDIO_FOLDER std::string("../data/audio")
#define PROJECT_FOLDER std::string("../data/workspaces/rack")

#include "audio/Compressor.h"
// #include "audio/Eq.h"
#include "audio/MMfilter.h"
#include "audio/ScatterXY.h"
#include "audio/engines/MonoSample.h"
#include "audio/engines/DrumGeneric.h"
#include "audio/engines/DrumSample.h"
#include "draw/draw.h"
#include "helpers/random.h"
#include "zicXYv2/generator.h"
#include "zicXYv2/step.h"

#ifndef FX_BUFFER_SIZE
#define FX_BUFFER_SIZE 48000
#endif

static constexpr int SCREEN_W = 320;
static constexpr int SCREEN_H = 240;

#ifndef KEY_DEF
#define KEY_DEF

#define KEY_F1 0x10
#define KEY_F2 0x11
#define KEY_F3 0x12
#define KEY_F4 0x13
#define KEY_F5 0x14

#define KEY_1 1
#define KEY_2 2
#define KEY_3 3
#define KEY_4 4
#define KEY_5 5
#define KEY_6 6
#define KEY_7 7
#define KEY_8 8

#endif

static constexpr int MARGIN = 2;

static constexpr int ENCODER_COUNT = 4;
static constexpr int MAX_TRACKS = 8;
static constexpr uint32_t SAMPLE_RATE = 44100;
static constexpr int BUFFER_SIZE = 4096;
static constexpr int FX_BUFFERS_PER_TRACK = 3;
static constexpr int WAVE_HISTORY = SCREEN_W - MARGIN * 2;


static constexpr int MAX_CLIP_COUNT = 32;

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

struct ParamValue {
    std::string key;
    float value;
    std::string string;
};

struct Clip {
    bool validated = false;
    std::vector<ParamValue> paramValues;
    std::vector<Step> sequence;
    bool saved = false;
    uint8_t engineId = 0;
};

// ================================================================
// Engine Registry
// ================================================================
struct EngineCreator {
    const char* name;
    TrackType type;
    bool showWaveform;
    void (*generate)(std::vector<Step>& sequence);
    // Lambda that takes the track's pre-allocated buffers and returns the engine
    std::function<std::unique_ptr<IEngine>(uint32_t, float**)> create;
};

static const EngineCreator engineRegistry[] = {
    { "Sample", TRACK_TYPE_SYNTH, true, Generator::generateBass, [](uint32_t sr, float** b) { return std::make_unique<MonoSample>(sr, b[0], b[1], b[2]); } },
    { "Drum", TRACK_TYPE_DRUM, false, Generator::generateClap, [](uint32_t sr, float** b) { return std::make_unique<DrumGeneric>(sr, b[0], b[1]); } },
    { "Drum Sample", TRACK_TYPE_DRUM, true, Generator::generateClap, [](uint32_t sr, float** b) { return std::make_unique<DrumSample>(sr, b[0], b[1]); } },
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
    // sf::IntRect vuRect, trackBounds, muteRect, volRect, headerRect;
    // std::vector<sf::IntRect> stepRects;
    std::vector<Step> sequence;
    std::deque<float> history;
    std::mutex historyMtx;
    int activeParamIdx = -1;
    std::chrono::steady_clock::time_point lastEditTime;
    std::vector<uint32_t> lastShiftTicks;
    uint32_t lastVolShiftTick = 0;
    // sf::IntRect genRect;
    void (*generate)(std::vector<Step>& sequence) = nullptr;
    uint32_t noteSamplesRemaining = 0;
    uint32_t genLen = 64;
    uint8_t currentEngineIdx = 0;
    bool showWaveform = true;
    int8_t encodersSelection = 0;

    // EQ eq;
    // SpectrumAnalyser spectrum;

    // sf::IntRect editRect;

    Clip clips[MAX_CLIP_COUNT];
    int activeClipIdx = 0;
    int pendingClipIdx = -1; // -1 = none, otherwise clip index to activate at loop

    // std::vector<std::pair<int, int>> scrollParamIndex;

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

        // eq.recompute(SAMPLE_RATE);
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
        currentEngineIdx = registryIdx;
        showWaveform = creator.showWaveform;

        if (engine) {
            lastShiftTicks.assign(engine->getParamCount(), 0);
        }
    }
};

enum {
    ViewMenu,
    ViewMaster,
    ViewTrack,
    ViewTrackShift,
    ViewSeq,
    ViewClips,
};

enum {
    KeyNone = 0,
    KeyView,
    KeyMute,
    KeyProject,
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

    float volume = 1.0f;

    // STEP EDITOR STATE
    int selTrack = 0;
    int selStep = 0;

    StepEditMode stepEditMode = EDIT_NOTE;

    ScatterXY masterScatter;
    std::atomic<bool> activeScatter { false };

    MMfilter filter;

    Compressor compressor;

    std::string projectPath = "";

    int currentView = ViewTrack;
    int currentCombinationKey = KeyNone;

    Studio()
    {
        Color palette[8] = { { 0, 200, 255 }, { 255, 100, 100 }, { 75, 191, 75 }, { 255, 210, 30 }, { 200, 100, 255 }, {40, 204, 160}, { 255, 150, 50 }, { 95, 115, 175 } };

        for (int i = 0; i < MAX_TRACKS; ++i) {
            auto trk = std::make_unique<Track>(0.7f, palette[i % 8]);

            int defaultEngine = 0;
            trk->setEngine(defaultEngine);

            tracks.push_back(std::move(trk));
        }

        updateClock();
    }

    void updateClock() { samplesPerStep = (SAMPLE_RATE * 60.0) / (bpm * 4.0); }
    void updateClock(float value) {
        bpm.store(value);
        updateClock();
    }

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