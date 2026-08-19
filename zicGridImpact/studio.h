#pragma once

#include "audio/engines/ImpactKick.h"
#include "audio/engines/DriftSynth1.h"
#include "audio/engines/ImpactSuper.h"
#include "audio/engines/DriftWavetable.h"
#include "audio/engines/ImpactChaos.h"
#include "audio/Scatter.h"
#include "audio/Compressor.h"
#include "mixer.h"
#include "sequencer.h"
#include <mutex>

struct StudioState {
    std::mutex audioMutex;

    ImpactKick kick { 44100.0f };
    DriftSynth1 synth1 { 44100.0f };
    ImpactSuper impactSuper { 44100.0f };
    int synth1EngineIdx = 0; // 0=DriftSynth1, 1=ImpactSuper

    DriftWavetable synth2 { 44100.0f };
    ImpactChaos chaos { 44100.0f };
    Sequencer seq { 44100.0f };
    Mixer mixer { 44100.0f };
    Scatter scatter;
    Compressor compressor { 44100.0f };

    int selTrack = 0; // 0=Kick, 1=Synth1, 2=Synth2, 3=Chaos
};

inline StudioState studio;

