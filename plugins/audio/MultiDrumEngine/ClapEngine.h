#pragma once
#include "plugins/audio/MultiDrumEngine/DrumEngine.h"
#include "plugins/audio/utils/effects/applyReverb.h"
#include "plugins/audio/utils/effects/applyBoost.h"
#include <cmath>
#include <array>

class ClapEngine : public DrumEngine {
public:
    ClapEngine(AudioPlugin::Props& p, AudioPlugin::Config& c)
        : DrumEngine(p, c, "Clap")
    {
        initValues();
    }

    // --- Parameters (10 total) ---
    Val& attack = val(0.05f, "ATTACK", {"Attack", .unit = "%"});
    Val& decay = val(25, "DECAY", {"Decay", .unit = "%"});
    Val& burstCount = val(5, "BURSTS", {"Bursts", .min = 1.f, .max = 10.f});
    Val& burstSpacing = val(50, "SPACING", {"Spacing", .unit = "%"});
    Val& noiseColor = val(15, "NOISE_COLOR", {"Noise Color", .unit = "%"});
    Val& tone = val(0.2f, "TONE", {"Tone", .unit = "%"});
    Val& tailAmount = val(0.3f, "TAIL", {"Tail", .unit = "%"});
    Val& stereo = val(50, "STEREO", {"Stereo", .unit = "%"});
    Val& boost = val(0.0f, "BOOST", {"Boost", .unit = "%"});
    Val& reverb = val(30, "REVERB", {"Reverb", .unit = "%"});

    static constexpr int REVERB_SIZE = 48000;
    float reverbBuf[REVERB_SIZE] = {};
    int rIdx = 0;

    float burstTimer = 0.f;
    int burstIndex = 0;
    float env = 0.f;
    float pink = 0.f;
    float phase = 0.f;
    bool active = false;

    void noteOn(uint8_t, float, void* = nullptr) override {
        burstTimer = 0.f;
        burstIndex = 0;
        env = 1.f;
        phase = 0.f;
        pink = 0.f;
        active = true;
    }

    void sampleOn(float* buf, float envAmp, int sc, int ts) override {
        if (!active) return;

        float t = float(sc) / ts;
        float spacing = burstSpacing.pct() * 0.03f + 0.01f;
        float decayTime = decay.pct() * 0.3f + 0.02f;

        float output = 0.0f;

        // Generate burst envelope
        if (burstIndex < int(burstCount.get())) {
            burstTimer += 1.f / props.sampleRate;
            if (burstTimer >= spacing) {
                burstTimer -= spacing;
                burstIndex++;
                env = 1.f;
            }
        }

        if (env > 0.f) {
            // Pink noise
            float white = props.lookupTable->getNoise() * 2.f - 1.f;
            pink = 0.98f * pink + 0.02f * white;
            float noise = pink * (1.f - noiseColor.pct()) + white * noiseColor.pct();

            // Filtered tone burst
            float toneFreq = 1000.f + tone.pct() * 4000.f;
            phase += 2.f * M_PI * toneFreq / props.sampleRate;
            float body = sinf(phase) * tailAmount.pct();

            float burst = (noise + body) * env;
            output += burst * (1.f + 0.5f * (props.lookupTable->getNoise() - 0.5f) * stereo.pct());

            env *= expf(-1.f / (props.sampleRate * decayTime));
        } else if (burstIndex >= int(burstCount.get())) {
            active = false;
        }

        output = applyBoost(output, boost.pct(), prevInput, prevOutput);
        output = applyReverb(output, reverb.pct(), reverbBuf, rIdx, REVERB_SIZE);

        buf[track] = output * envAmp;
    }

    void sampleOff(float* buf) override {
        buf[track] = applyReverb(buf[track], reverb.pct(), reverbBuf, rIdx, REVERB_SIZE);
    }

private:
    float prevInput = 0.f;
    float prevOutput = 0.f;
};
