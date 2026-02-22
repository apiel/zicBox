#pragma once

#include "audio/engines/DrumKickSegment.WIP.h"
#include "plugins/audio/MultiEngine.h"
#include "plugins/audio/utils/valMultiFx.h"

class KickSegmentEngine : public MultiEngine {
protected:
    DrumKickSeg kick;

    // --- Main Parameters ---
    Val& duration = val("DURATION", kick.duration);
    GraphPointFn ampGraph = [&](float i) { return *kick.envelopAmp.getMorphShape(i); };
    Val& ampMorph = val("AMP_MORPH", kick.ampEnv, ampGraph);

    //TODO
    // FIXME
    // 
    // Find how to update the active segment in val when changed....
    Val& activeSegment = val("SEG_INDEX", kick.activeSegIdx, NULL, [&]() { needCopyValues = true; });
    Val& segmentPitch = val("SEG_PITCH", kick.activeSegVal);

    Val& baseFrequency = val("FREQ", kick.baseFrequency);
    Val& pitchRange = val("RANGE", kick.pitchRange);
    Val& punchDrive = val("PUNCH_DRIVE", kick.punchDrive);
    Val& fmDirt = val("FM_DIRT", kick.fmDirt);
    Val& driveAmount = val("DRIVE", kick.driveAmount);
    Val& compressionAmount = val("COMP", kick.compressionAmount);

    Val& fxType = val("FX_TYPE", kick.fxType);
    Val& fxAmount = val("FX_AMOUNT", kick.fxAmount);

public:
    KickSegmentEngine(AudioPlugin::Props& p, AudioPlugin::Config& c, float* fxBuffer)
        : MultiEngine(p, c, "KickSeg")
        , kick(p.sampleRate, fxBuffer)
    {
        initValues();
    }

    void noteOn(uint8_t note, float velocity, void* = nullptr) override
    {
        kick.noteOn(note, velocity);
    }

    void sample(float* buf) override
    {
        buf[track] = kick.sample();
    }
};