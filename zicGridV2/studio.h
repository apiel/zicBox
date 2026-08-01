#pragma once

#include <atomic>
#include <chrono>
#include <deque>
#include <functional>
#include <iostream>
#include <memory>
#include <mutex>
#include <string>
#include <vector>

#ifdef IS_RPI
#define AUDIO_FOLDER std::string("data/audio")
#define PROJECT_FOLDER std::string("data/workspaces/rack")
#else
#define AUDIO_FOLDER std::string("../data/audio")
#define PROJECT_FOLDER std::string("../data/workspaces/rack")
#endif

#include "audio/Compressor.h"
#include "audio/MMfilter.h"
#include "audio/Scatter.h"
#include "audio/Tape.h"

#include "audio/engines/DrumGeneric.h"
#include "audio/engines/DrumSample.h"
#include "audio/engines/MonoSample.h"
#include "audio/engines/TribeWave.h"
#include "audio/engines/VoidBass.h"

#include "draw/draw.h"
#include "helpers/random.h"
#include "zicGridV2/generator.h"
#include "zicGridV2/gridState.h"
#include "zicGridV2/step.h"

#ifndef FX_BUFFER_SIZE
#define FX_BUFFER_SIZE 48000
#endif

static constexpr int SCREEN_W = 480;
static constexpr int SCREEN_H = 640;

static constexpr int MARGIN = 2;
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
    std::string name;
    int noteRepeat = 2;
};

struct EngineCreator {
    const char* name;
    TrackType type;
    bool showWaveform;
    void (*generate)(std::vector<Step>& sequence, float p1, float p2, float p3);
    std::function<std::unique_ptr<IEngine>(uint32_t, float**)> create;
};

static const EngineCreator engineRegistry[] = {
    { "Sample", TRACK_TYPE_SYNTH, true, Generator::generateKick, [](uint32_t sr, float** b) { return std::make_unique<MonoSample>(sr, b[0], b[1], b[2]); } },
    { "Drum", TRACK_TYPE_DRUM, false, Generator::generateKick, [](uint32_t sr, float** b) { return std::make_unique<DrumGeneric>(sr, b[0], b[1]); } },
    { "Drum Sample", TRACK_TYPE_DRUM, true, Generator::generateKick, [](uint32_t sr, float** b) { return std::make_unique<DrumSample>(sr, b[0], b[1]); } },
    { "Void Bass", TRACK_TYPE_SYNTH, false, Generator::generateBass, [](uint32_t sr, float** b) { return std::make_unique<VoidBass>(sr, b[0]); } },
    { "Tribe Wave", TRACK_TYPE_SYNTH, false, Generator::generateBass, [](uint32_t sr, float** b) { return std::make_unique<TribeWave>(sr, b[0], b[1], b[2]); } },
};

static const int ENGINE_REGISTRY_COUNT = sizeof(engineRegistry) / sizeof(EngineCreator);

struct Track {
    TrackType type;
    float* fxBuffers[FX_BUFFERS_PER_TRACK];

    std::unique_ptr<IEngine> engine;
    float volume = 0.8f;
    bool isMuted = false;
    bool isSoloed = false;
    Color themeColor;
    std::atomic<float> vumeter { 0.0f };
    std::vector<Step> sequence;
    std::deque<float> history;
    std::mutex historyMtx;
    int activeParamIdx = -1;

    void (*generate)(std::vector<Step>& sequence, float p1, float p2, float p3) = nullptr;
    int genEngine = 0; // 0 = Kick, 1 = Bass, 2 = Drum
    float genParams[3] = { 0.5f, 0.5f, 0.5f };
    uint32_t noteSamplesRemaining = 0;
    uint8_t playingNote = 0;
    uint32_t genLen = 32;
    uint8_t currentEngineIdx = 0;
    bool showWaveform = true;

    Clip clips[MAX_CLIP_COUNT];
    int activeClipIdx = 0;
    int pendingClipIdx = -1;

    Track(float v, Color c)
        : volume(v)
        , themeColor(c)
    {
        for (int i = 0; i < FX_BUFFERS_PER_TRACK; ++i) {
            fxBuffers[i] = new float[FX_BUFFER_SIZE]();
        }

        history.resize(WAVE_HISTORY, 0.0f);
        sequence.resize(SEQ_STEPS);

        setEngine(0);
    }

    ~Track()
    {
        for (int i = 0; i < FX_BUFFERS_PER_TRACK; ++i) {
            delete[] fxBuffers[i];
        }
    }

    void runGeneration()
    {
        if (genEngine == 0) {
            Generator::generateKick(sequence, genParams[0], genParams[1], genParams[2]);
        } else if (genEngine == 1) {
            Generator::generateBass(sequence, genParams[0], genParams[1], genParams[2]);
        } else if (genEngine == 2) {
            Generator::generateDrum(sequence, genParams[0], genParams[1], genParams[2]);
        }
    }

    void setEngine(int registryIdx)
    {
        if (registryIdx < 0 || registryIdx >= ENGINE_REGISTRY_COUNT) return;
        const auto& creator = engineRegistry[registryIdx];
        type = creator.type;
        showWaveform = creator.showWaveform;
        generate = creator.generate;
        currentEngineIdx = (uint8_t)registryIdx;
        engine = creator.create(SAMPLE_RATE, fxBuffers);
    }
};

struct MasterFxState {
    Compressor compressor;
    MMfilter filter;
    Scatter scatter;
    Tape tape;
    float volume = 1.0f;

    MasterFxState()
        : compressor(SAMPLE_RATE)
        , filter()
        , scatter()
        , tape()
    {
    }
};

struct Studio {
    std::vector<std::unique_ptr<Track>> tracks;
    MasterFxState masterFx;

    std::mutex audioMutex;
    float bpm = 125.0f;
    bool isPlaying = false;
    uint32_t currentStep = 0;
    uint32_t sampleCounter = 0;
    uint32_t samplesPerStep = (SAMPLE_RATE * 60) / (125.0f * 4);

    int selTrack = 0;
    int selStep = -1;
    int currentView = VIEW_INSTRUMENT;

    Studio()
    {
        Color trackColors[MAX_TRACKS] = {
            { 255, 80, 80, 255 },   // T1 Red
            { 255, 160, 40, 255 },  // T2 Orange
            { 240, 220, 40, 255 },  // T3 Yellow
            { 60, 220, 100, 255 },  // T4 Green
            { 40, 200, 255, 255 },  // T5 Cyan
            { 100, 120, 255, 255 }, // T6 Blue
            { 200, 80, 255, 255 },  // T7 Purple
            { 255, 100, 180, 255 }  // T8 Pink
        };

        for (int i = 0; i < MAX_TRACKS; ++i) {
            tracks.push_back(std::make_unique<Track>(0.8f, trackColors[i]));
            tracks.back()->setEngine(i % ENGINE_REGISTRY_COUNT);
        }

        updateBpm(125.0f);
    }

    void updateBpm(float newBpm)
    {
        bpm = newBpm;
        if (bpm < 20.0f) bpm = 20.0f;
        if (bpm > 300.0f) bpm = 300.0f;
        samplesPerStep = (uint32_t)((SAMPLE_RATE * 60.0f) / (bpm * 4.0f));
    }
};

inline Studio studio;
