#pragma once

#include "audio/engines/DrumKickSegment.h"
#include "plugins/audio/MultiEngine.h"
#include "plugins/audio/utils/valMultiFx.h"

class KickSegmentEngine : public MultiEngine {
protected:
    DrumKickSeg kick;

    // --- Main Parameters ---
    Val& duration = val("DURATION", kick.duration);
    GraphPointFn ampGraph = [&](float i) { return *kick.envelopAmp.getMorphShape(i); };
    Val& ampMorph = val("AMP_MORPH", kick.ampEnv, ampGraph);

    Val& page = val(0, "SEG_INDEX", { "Seg index", .min = 0, .max = 80, .step = 20 }, [&](auto p) {
        float current = p.val.get();
        p.val.setFloat(p.value);

        if (current != p.val.get()) {
            swapVal(current);
            swapVal(p.val.get());
        }
        needCopyValues = true;
    });
    Val& segment0 = val("SEG0", kick.seg0); // 3

    Val& baseFrequency = val("FREQ", kick.baseFrequency);
    Val& pitchRange = val("RANGE", kick.pitchRange);
    Val& punchDrive = val("PUNCH_DRIVE", kick.punchDrive);
    Val& fmDirt = val("FM_DIRT", kick.fmDirt);
    Val& driveAmount = val("DRIVE", kick.driveAmount);
    Val& compressionAmount = val("COMP", kick.compressionAmount);

    Val& fxType = val("FX_TYPE", kick.fxType);
    Val& fxAmount = val("FX_AMOUNT", kick.fxAmount);

    Val& segment20 = val("SEG20", kick.seg20); // 12
    Val& segment40 = val("SEG40", kick.seg40); // 13
    Val& segment60 = val("SEG60", kick.seg60); // 14
    Val& segment80 = val("SEG80", kick.seg80); // 15

    void swapVal(float index) {
        if (index == 20) {
            std::swap(mapping[3], mapping[12]);
        } else if (index == 40) {
            std::swap(mapping[3], mapping[13]);
        } else if (index == 60) {
            std::swap(mapping[3], mapping[14]);
        } else if (index == 80) {
            std::swap(mapping[3], mapping[15]);
        }
    }

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