#pragma once

#include "plugins/audio/MultiDrumEngine/DrumEngine.h"
#include "plugins/audio/utils/valMultiFx.h"
#include "audio/utils/math.h"

#include <cmath>
#include <vector>

class KickSegmentEngine : public DrumEngine {
protected:
    MultiFx multiFx;

    float velocity = 1.0f;
    float phase = 0.0f;
    float modPhase = 0.0f; // Phase for the "Dirt" modulator

    // 10 stages of the pitch envelope (normalized 0.0 to 1.0)
    float pitchSegments[10] = { 1.0f, 0.3f, 0.2f, 0.15f, 0.12f, 0.11f, 0.08f, 0.07f, 0.06f, 0.0f };

public:
    // --- Segment Editor ---
    Val& activeSegment = val(0, "SEG_INDEX", { .label = "Edit Segment", .min = 0, .max = 9, .step = 1 }, [&](auto p) {
        p.val.setFloat(p.value);
        int idx = (int)activeSegment.get();
        segmentPitch.setFloat(pitchSegments[idx] * 100.0f);
        needCopyValues = true;
    });

    Val& segmentPitch = val(0, "SEG_PITCH", { .label = "Seg Value", .unit = "%" }, [&](auto p) {
        p.val.setFloat(p.value);
        int idx = (int)activeSegment.get();
        pitchSegments[idx] = p.val.pct();
        p.val.props().label = "Seg " + std::to_string(idx) + " Val";
    });

    // --- Main Parameters ---
    Val& baseFrequency = val(45.0f, "FREQ", { .label = "Sub Freq", .min = 30.0, .max = 80.0, .unit = "Hz" });
    Val& pitchRange = val(600, "RANGE", { .label = "Env Range", .min = 0, .max = 2000, .unit = "Hz" });

    // Character
    Val& punchDrive = val(20, "PUNCH_DRIVE", { .label = "Punch Drive", .unit = "%" });
    
    // REPLACED TONE: FM Dirt creates aggressive sidebands
    Val& fmDirt = val(0, "FM_DIRT", { .label = "FM Dirt", .unit = "%" }); 

    Val& driveAmount = val(30, "DRIVE", { .label = "Drive", .unit = "%" });
    Val& compressionAmount = val(20, "COMP", { .label = "Compressor", .unit = "%" });

    Val& fxType = val(0, "FX_TYPE", { .label = "FX type", .type = VALUE_STRING, .max = MultiFx::FX_COUNT - 1 }, valMultiFx(multiFx));
    Val& fxAmount = val(0, "FX_AMOUNT", { .label = "FX edit", .unit = "%" });

public:
    KickSegmentEngine(AudioPlugin::Props& p, AudioPlugin::Config& c, float* fxBuffer)
        : DrumEngine(p, c, "KickSeg")
        , multiFx(props.sampleRate, fxBuffer)
    {
        initValues();
    }

    void noteOn(uint8_t note, float _velocity, void* = nullptr) override
    {
        DrumEngine::noteOn(note, _velocity);
        velocity = _velocity;
        phase = 0.0f;
        modPhase = 0.0f;
    }

    void sampleOn(float* buffer, float envelopeAmplitude, int sampleCounter, int totalSamples) override
    {
        // 1. Precise Segment Interpolation
        float timeInSeconds = (float)sampleCounter / props.sampleRate;
        float segmentDuration = 0.020f; // 20ms segments
        float currentPosition = timeInSeconds / segmentDuration;

        int index0 = (int)std::floor(currentPosition);
        int index1 = index0 + 1;
        float fraction = currentPosition - (float)index0;

        float envValue = 0.0f;
        if (index0 < 9) {
            envValue = pitchSegments[index0] * (1.0f - fraction) + pitchSegments[index1] * fraction;
        } else if (index0 == 9) {
            envValue = pitchSegments[9] * (1.0f - fraction);
        }

        // 2. Dual Oscillator FM Logic
        float rootFreq = baseFrequency.get() + (envValue * pitchRange.get());
        
        // MODULATOR: Frequency follows carrier but faster.
        // Higher fmDirt makes the ratio more harmonically complex.
        float dirtPct = fmDirt.pct();
        float modFreq = rootFreq * (3.5f + dirtPct * 5.0f); 

        modPhase += (modFreq / props.sampleRate);
        if (modPhase > 1.0f) modPhase -= 1.0f;

        // Efficient Triangle Modulator: (4.0f * abs(phase - 0.5f) - 1.0f)
        float modulator = 4.0f * (modPhase < 0.5f ? modPhase : 1.0f - modPhase) - 1.0f;

        // Apply Phase Modulation
        // Modulation depth is scaled by envValue so the "Dirt" disappears as the sub rings out.
        float modulationDepth = dirtPct * envValue * 0.25f;
        phase += (rootFreq / props.sampleRate) + (modulator * modulationDepth);
        
        // Wrap Phase
        while (phase > 1.0f) phase -= 1.0f;
        while (phase < 0.0f) phase += 1.0f;

        float out = sinf(phase * 2.0f * M_PI);

        // 3. Hardcore Drive (The 24x Saturation)
        if (punchDrive.pct() > 0.0f) {
            float intensity = punchDrive.pct() * 24.0f * (envValue + 0.1f);
            out = (out * (1.0f + intensity)) / (1.0f + intensity * fabsf(out));
        }

        // 4. Wave Folding (Bonus Grit)
        // If Dirt is cranked, we fold the peaks for a "screaming" texture
        if (dirtPct > 0.7f) {
            float foldAmt = (dirtPct - 0.7f) * 4.0f;
            out = sinf(out * (1.0f + foldAmt));
        }

        // 5. Final Processing Chain
        if (driveAmount.get() > 0.0f) {
            out = applyDrive(out, driveAmount.pct() * 2.5f);
        }

        if (compressionAmount.get() > 0.0f) {
            out = applyCompression(out, compressionAmount.pct());
        }

        out = multiFx.apply(out, fxAmount.pct());

        buffer[track] = out * envelopeAmplitude * velocity;
    }

    void sampleOff(float* buf) override
    {
        float out = buf[track];
        out = multiFx.apply(out, fxAmount.pct());
        buf[track] = out;
    }
};