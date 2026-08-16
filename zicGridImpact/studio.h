#pragma once

#include "audio/engines/DriftKick.h"
#include "audio/engines/DriftSynth1.h"
#include "audio/engines/DriftSynth2.h"
#include "audio/Scatter.h"
#include "audio/Compressor.h"
#include "mixer.h"
#include "sequencer.h"
#include <mutex>

struct StudioState {
    std::mutex audioMutex;

    DriftKick kick { 44100.0f };
    DriftSynth1 synth1 { 44100.0f };
    DriftSynth2 synth2 { 44100.0f };
    Sequencer seq { 44100.0f };
    Mixer mixer { 44100.0f };
    Scatter scatter;
    Compressor compressor { 44100.0f };

    int selTrack = 0; // 0=Kick, 1=Synth1, 2=Synth2
};

inline StudioState studio;
