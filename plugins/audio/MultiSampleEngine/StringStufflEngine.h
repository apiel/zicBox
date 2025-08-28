#pragma once

#include "plugins/audio/MultiSampleEngine/SampleEngine.h"
#include "plugins/audio/utils/MultiFx.h"
#include <vector>
#include <cmath>


// TODO should convert this to synth or drum engine...

class PhysicalModelEngine : public SampleEngine {
protected:
    MultiFx multiFx;

    // Parameters
    Val& exciteAmount; // 0..100% -> how much sample excites the resonator vs direct
    Val& decay;        // feedback decay (0..1)
    Val& tone;         // low-pass cutoff (0..1)
    Val& pitchOffset;  // semitone offset applied to resonator base pitch
    Val& fxType;
    Val& fxAmount;

    // Resonator state
    std::vector<float> delayLine;
    uint32_t delayLen = 0;
    uint32_t writePos = 0;
    uint32_t readPos = 0;
    float onePoleState = 0.0f;

    float sampleRate = 48000.0f;
    bool resonatorActive = false;

    // excitation buffer length (in samples)
    uint32_t exciteLen = 0;

    // defaults
    static constexpr uint32_t MAX_DELAY = 1 << 16; // 65536

    // Helper: convert MIDI note to frequency
    inline float noteToFreq(int note) const {
        return 440.0f * powf(2.0f, (note - 69) / 12.0f);
    }

    // Fill the delay line with excitation: prefer using first portion of sampleBuffer if available
    void exciteFromSample(float velocity) {
        if (sampleBuffer.count == 0) {
            // fallback to noise
            for (uint32_t i = 0; i < delayLen; ++i) {
                float n = (rand() / (float)RAND_MAX) * 2.0f - 1.0f;
                delayLine[i] = n * velocity;
            }
        } else {
            // Copy exciteLen samples from start of sampleBuffer (or wrap) into delay
            uint64_t avail = sampleBuffer.count;
            for (uint32_t i = 0; i < delayLen; ++i) {
                uint64_t idx = i % avail;
                // simple linear taper to avoid clicks
                float env = 1.0f;
                if (i < exciteLen) env = (float)i / (float)std::max<uint32_t>(1, exciteLen);
                float s = sampleBuffer.data[idx] * env * velocity;
                delayLine[i] = s;
            }
        }
        writePos = 0;
        readPos = 0;
        onePoleState = 0.0f;
        resonatorActive = true;
    }

public:
    PhysicalModelEngine(AudioPlugin::Props& props, AudioPlugin::Config& config, SampleBuffer& sampleBuffer)
        : SampleEngine(props, config, sampleBuffer, "PhysModel")
        , multiFx(props.sampleRate, props.lookupTable)
        , exciteAmount(val(50.0f, "EXCITE", { "Excite", .unit = "%" }, [&](auto p) { p.val.setFloat(p.value); }))
        , decay(val(0.98f, "DECAY", { "Decay", .min = 0.80f, .max = 0.999f }, [&](auto p) { p.val.setFloat(p.value); }))
        , tone(val(50.0f, "TONE", { "Tone", .unit = "%" }, [&](auto p) { p.val.setFloat(p.value); }))
        , pitchOffset(val(0.0f, "PITCH", { "Pitch", .min = -12.0f, .max = 12.0f, .step = 1.0f }, [&](auto p) { p.val.setFloat(p.value); }))
        , fxType(val(0, "FX_TYPE", { "FX type", VALUE_STRING, .max = MultiFx::FXType::FX_COUNT - 1 }, [&](auto p) { multiFx.setFxType(p); }))
        , fxAmount(val(0, "FX_AMOUNT", { "FX edit", .unit = "%" }))
    {
        sampleRate = props.sampleRate;
        // Initialize delay line with a reasonable size
        delayLen = (uint32_t)std::min<uint64_t>((uint64_t)(sampleRate * 0.02f), (uint64_t)MAX_DELAY); // default 20ms
        delayLine.assign(delayLen + 4, 0.0f);
        exciteLen = (uint32_t)std::max<int>(4, (int)(sampleRate * 0.002f)); // 2ms excitation copy
    }

    void noteOn(uint8_t note, float velocity, void *) override {
        // Determine delay length from pitch (note + pitchOffset)
        float targetNote = note + pitchOffset.get();
        float freq = noteToFreq((int)roundf(targetNote));
        if (freq < 20.0f) freq = 20.0f;
        if (freq > sampleRate * 0.45f) freq = sampleRate * 0.45f;
        uint32_t len = (uint32_t)std::max<int>(2, (int)std::round(sampleRate / freq));
        len = std::min<uint32_t>(len, MAX_DELAY);

        if (len != delayLen) {
            delayLen = len;
            delayLine.assign(delayLen + 4, 0.0f);
        }

        // Excite the delay line: blend between sample-based excitation and noise depending on exciteAmount
        float excitePct = exciteAmount.get() * 0.01f;
        if (excitePct > 0.5f && sampleBuffer.count > 0) {
            // use sample content to excite (copy from buffer start)
            for (uint32_t i = 0; i < delayLen; ++i) {
                uint64_t idx = i % std::max<uint64_t>(1, sampleBuffer.count);
                float s = sampleBuffer.data[idx];
                float n = (rand() / (float)RAND_MAX) * 2.0f - 1.0f;
                delayLine[i] = s * excitePct + n * (1.0f - excitePct);
            }
        } else {
            // use noise
            for (uint32_t i = 0; i < delayLen; ++i) {
                float n = (rand() / (float)RAND_MAX) * 2.0f - 1.0f;
                delayLine[i] = n * velocity;
            }
        }
        writePos = 0;
        readPos = 0;
        onePoleState = 0.0f;
        resonatorActive = true;
    }

    // Produce one resonator sample (Karplus-Strong like)
    float resonatorTick() {
        if (!resonatorActive || delayLen == 0) return 0.0f;

        // read two samples for simple fractional interpolation
        uint32_t rp = readPos % delayLen;
        uint32_t rp1 = (rp + 1) % delayLen;
        float s0 = delayLine[rp];
        float s1 = delayLine[rp1];
        // simple linear interpolation
        float out = 0.5f * (s0 + s1);

        // one-pole lowpass to model tone
        float cutoff = std::max(0.001f, tone.get() * 0.01f);
        onePoleState = onePoleState + cutoff * (out - onePoleState);
        float filtered = onePoleState;

        // write back with decay (feedback)
        float fb = decay.get();
        delayLine[writePos % delayLen] = filtered * fb;

        // advance pointers
        writePos = (writePos + 1) % delayLen;
        readPos = (readPos + 1) % delayLen;

        // if values become very small, consider inactive
        if (fabs(filtered) < 1e-6f) {
            resonatorActive = false;
        }

        return filtered;
    }

    // Get one sample: combine direct sample playback and resonator output
    float getSample(int index, float stepIncrement) override {
        // Direct sample playback (linear interp)
        float direct = 0.0f;
        if (sampleBuffer.count > 0) {
            int64_t idx = index;
            if (idx < 0 || (uint64_t)idx >= sampleBuffer.count) {
                direct = 0.0f;
            } else {
                uint64_t i0 = (uint64_t)idx;
                uint64_t i1 = (i0 + 1 < sampleBuffer.count) ? i0 + 1 : i0;
                float frac = index - (float)i0;
                float s0 = sampleBuffer.data[i0];
                float s1 = sampleBuffer.data[i1];
                direct = s0 + (s1 - s0) * frac;
            }
        }

        float modelOut = resonatorTick();

        // Mix according to exciteAmount: higher excite -> more modeled sound
        float excitePct = exciteAmount.get() * 0.01f;
        float out = modelOut * excitePct + direct * (1.0f - excitePct);

        return multiFx.apply(out, fxAmount.pct());
    }

    void sample(float* buf, int index) override {
        float out = buf[track];
        out = multiFx.apply(out, fxAmount.pct());
        buf[track] = out;
    }
};
