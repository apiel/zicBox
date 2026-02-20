#pragma once

#include "audio/MultiFx.h"
#include "plugins/audio/MultiDrumEngine/DrumEngine.h"

#include <cmath>
#include <vector>

class KickSegmentEngine : public DrumEngine {
protected:
    MultiFx multiFx;

    float velocity = 1.0f;
    float phase = 0.0f;

    // We store 10 stages of the pitch envelope (normalized 0.0 to 1.0)
    // These represent the 'extra' frequency added to the base
    float pitchSegments[10] = { 1.0f, 0.8f, 0.6f, 0.4f, 0.2f, 0.1f, 0.05f, 0.02f, 0.01f, 0.0f };

    // Internal states
    float lowPassState = 0.0f;

public:
    // --- Segment Editor ---
    Val& activeSegment = val(0, "SEG_INDEX", { .label = "Edit Segment", .min = 0, .max = 9, .step = 1 }, [&](auto p) {
        p.val.setFloat(p.value);
        int idx = (int)activeSegment.get();
        segmentPitch.setFloat(pitchSegments[idx] * 100.0f);
        // We update the labels and values of the focus knobs below
        needCopyValues = true;
    });

    // This knob changes the pitch of the segment selected above
    Val& segmentPitch = val(0, "SEG_PITCH", { .label = "Seg Value", .unit = "%" }, [&](auto p) {
        p.val.setFloat(p.value);
        int idx = (int)activeSegment.get();
        pitchSegments[idx] = p.val.pct();
        // Update the name dynamically to show which one we are editing
        p.val.props().label = "Seg " + std::to_string(idx) + " Val";
    });

    // --- Main Parameters ---
    Val& baseFrequency = val(45.0f, "FREQ", { .label = "Sub Freq", .min = 30.0, .max = 80.0, .unit = "Hz" });
    Val& pitchRange = val(600, "RANGE", { .label = "Env Range", .min = 0, .max = 2000, .unit = "Hz" });

    // Character
    Val& punchDrive = val(20, "PUNCH_DRIVE", { .label = "Punch Drive", .unit = "%" });
    Val& tone = val(80, "TONE", { .label = "Tone", .unit = "%" });

    Val& driveAmount = val(30, "DRIVE", { .label = "Drive", .unit = "%" });
    Val& compressionAmount = val(20, "COMP", { .label = "Compressor", .unit = "%" });

    Val& fxType = val(0, "FX_TYPE", { .label = "FX type", .type = VALUE_STRING, .max = MultiFx::FXType::FX_COUNT - 1 }, multiFx.setFxType);
    Val& fxAmount = val(0, "FX_AMOUNT", { .label = "FX edit", .unit = "%" });

public:
    KickSegmentEngine(AudioPlugin::Props& p, AudioPlugin::Config& c)
        : DrumEngine(p, c, "KickSeg")
        , multiFx(props.sampleRate)
    {
        initValues();
    }

    void noteOn(uint8_t note, float _velocity, void* = nullptr) override
    {
        DrumEngine::noteOn(note, _velocity);
        velocity = _velocity;
        phase = 0.0f;
        lowPassState = 0.0f;
    }

    void sampleOn(float* buffer, float envelopeAmplitude, int sampleCounter, int totalSamples) override
    {
        // 1. Precise Segment Interpolation
        // We divide the first 200ms of the kick into 10 segments.
        // Anything after that stays at the base frequency.
        float timeInSeconds = (float)sampleCounter / props.sampleRate;
        float segmentDuration = 0.020f; // 20ms per segment = 200ms total area
        float currentPosition = timeInSeconds / segmentDuration;

        int index0 = (int)std::floor(currentPosition);
        int index1 = index0 + 1;
        float fraction = currentPosition - (float)index0;

        float envValue = 0.0f;
        if (index0 < 9) {
            // Linear interpolation between the points you set manually
            envValue = pitchSegments[index0] * (1.0f - fraction) + pitchSegments[index1] * fraction;
        } else if (index0 == 9) {
            envValue = pitchSegments[9] * (1.0f - fraction);
        }

        // 2. Oscillator Logic (FM style)
        float currentFreq = baseFrequency.get() + (envValue * pitchRange.get());

        // FM Modulator for texture
        float modFreq = currentFreq * 1.0f;

        phase += (currentFreq / props.sampleRate);
        if (phase > 1.0f) phase -= 1.0f;

        float out = sinf(phase * 2.0f * M_PI);

        // 3. Hardcore Drive (Using the 24x Logic)
        if (punchDrive.pct() > 0.0f) {
            float intensity = punchDrive.pct() * 24.0f * (envValue + 0.1f);
            out = (out * (1.0f + intensity)) / (1.0f + intensity * fabsf(out));
        }

        // 4. Tone LPF
        float cutoff = 0.05f + tone.pct() * 0.8f;
        lowPassState += cutoff * (out - lowPassState);
        out = lowPassState;

        if (driveAmount.get() > 0.0f) {
            out = applyDrive(out, driveAmount.pct() * 2.5f);
        }

        // Glue (Compression) to bring the tail and the punch together
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