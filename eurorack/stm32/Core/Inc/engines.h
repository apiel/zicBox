#pragma once

#define FX_BUFFER_SIZE 2048
// #define FX_BUFFER_SIZE 4096
// #define FX_BUFFER_SIZE 8192
// #define FX_BUFFER_SIZE 16384
// #define FX_BUFFER_SIZE 32768

#include "audio/effects/applyReverb.h"
#include "audio/engines/DrumClap.h"
#include "audio/engines/DrumEdge.h"
#include "audio/engines/DrumImpact.h"
#include "audio/engines/DrumKick2.h"
#include "audio/engines/DrumKickFM.h"
#include "audio/engines/DrumSnare.h"
#include "audio/engines/DrumMetalic.h"
#include "audio/engines/DrumPercussion.h"

#define SAMPLE_RATE 44104.0f

FX_BUFFER

enum EngineType { KICK2,
    KICKFM,
    EDGE,
    IMPACT,
    CLAP,
    SNARE,
    METALIC,
    PERC,
    ENGINE_COUNT,
};

DrumKick2 kick2(SAMPLE_RATE);
DrumKickFM kickfm(SAMPLE_RATE);
DrumEdge edge(SAMPLE_RATE, buffer);
DrumImpact impact(SAMPLE_RATE, buffer);
DrumSnare snare(SAMPLE_RATE);
DrumClap clap(SAMPLE_RATE, buffer);
DrumMetalic metalic(SAMPLE_RATE, buffer);
DrumPercussion percussion(SAMPLE_RATE, buffer);

IEngine* engines[ENGINE_COUNT] = {
    &kick2,
    &kickfm,
    &edge,
    &impact,
    &clap,
    &snare,
    &metalic,
    &percussion,
};