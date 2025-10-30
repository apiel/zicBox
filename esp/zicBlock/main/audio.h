#pragma once

#define SKIP_SNDFILE

#include "plugins/audio/utils/EnvelopDrumAmp.h"
#include "plugins/audio/utils/KickEnvTableGenerator.h"
#include "plugins/audio/utils/KickTransientTableGenerator.h"
#include "plugins/audio/utils/WavetableGenerator2.h"
#include "plugins/audio/utils/effects/applySample.h"
#include "plugins/audio/utils/filterArray.h"
#include "plugins/audio/utils/lookupTable.h"

#include <cmath>
#include <string>
#include <vector>
class Audio {
protected:
    LookupTable lookupTable;

    EffectFilterArray<3> filter;
    float resonance = 0.0f; // 0.0 to 1.0

    float freq = 1.0f;
    float baseFreq = 440.0f;
    float velocity = 1.0f;

    // Higher base note is, lower pitch will be
    //
    // Usualy our base note is 60
    // but since we want a kick sound we want bass and we remove one octave (12 semitones)
    // So we send note 60, we will play note 48...
    uint8_t baseNote = 60 + 12;

    Audio()
        : waveform(&lookupTable, sampleRate)
    {
        waveform.setType(WavetableGenerator::Type::Saw);
        envelopAmp.morph(0.2f);
        clapEnvelopAmp.morph(0.0f);
    }

    float resonatorState = 0.0f;
    float applyResonator(float input)
    {
        // Advance "state" (acts like a time accumulator)
        resonatorState += 1.0f / sampleRate;

        // Compute output = input * e^(-decay * t) * sin(2π f t)
        float output = input * expf(-0.02f * resonatorState)
            * sinf(2.0f * M_PI * baseFreq * resonatorState * resonator);

        // Optional loudness compensation so higher freq = less drop in volume
        float compensation = sqrtf(220.0f / std::max(baseFreq, 1.0f));
        output *= compensation;

        float amount = resonator / 1.5f;
        return (output * amount) + (input * (1.0f - amount));
    }

    float applyFilter(float out, float envAmp)
    {
        if (filterCutoff < 0) {
            float amount = -filterCutoff;
            filter.setCutoff(0.85 * amount * envAmp + 0.1);
            filter.setSampleData(out, 0);
            filter.setSampleData(filter.hp[0], 1);
            filter.setSampleData(filter.hp[1], 2);
            // if (amount < 0.5f) { // Soft transition between LP and HP
            //     float ratio = amount / 0.5f;
            //     out = filter.lp[0] * (1.0f - ratio) + filter.hp[2] * ratio;
            // } else {
            //     out = filter.hp[2];
            // }
            // Let's just always mix both of them
            out = filter.lp[0] * (1.0f - amount) + filter.hp[2] * amount;
        } else {
            filter.setCutoff(0.85 * filterCutoff * envAmp + 0.1);
            filter.setSampleData(out, 0);
            filter.setSampleData(filter.lp[0], 1);
            filter.setSampleData(filter.lp[1], 2);
            out = filter.lp[2];
        }
        return out;
    }

    int totalSamples = 0;
    int sampleCounter = 0;
    float sampleIndex = 0.0f;
    float tone()
    {
        if (sampleCounter < totalSamples) {
            float t = float(sampleCounter) / totalSamples;
            sampleCounter++;

            float envAmp = envelopAmp.next();
            float envFreq = envelopFreq.next(t);

            float modulatedFreq = freq + envFreq;
            float out = waveform.sample(&sampleIndex, modulatedFreq) * 0.75f;

            if (resonator > 0.0f) {
                out = applyResonator(out);
            }

            if (timbre > 0.0f) {
                // Adjust timbre by filtering harmonics dynamically
                out *= (1.0f - timbre) + timbre * sinf(2.0f * M_PI * baseFreq * 0.5f * t);
            }

            if (t < 0.01f && transient.getMorph() > 0.0f) {
                out = out + transient.next(t);
            }

            out = applyFilter(out, envAmp);

            return out * envAmp * toneVolume;
        }
        return 0.0f;
    }

    float burstTimer = 0.f;
    int burstIndex = 0;
    float clapEnv = 0.0f;
    float pink = 0.0f;
    bool clapActive = false;
    float clap()
    {
        float out = 0.0f;
        if (clapActive) {
            float envAmp = clapEnvelopAmp.next();
            float spacing = burstSpacing * 0.03f + 0.01f;
            float decayTime = clapDecay * 0.3f + 0.02f;

            if (burstIndex < int(burstCount)) {
                burstTimer += 1.f / sampleRate;
                if (burstTimer >= spacing) {
                    burstTimer -= spacing;
                    burstIndex++;
                    clapEnv = 1.f;
                }
            }

            if (clapEnv > 0.f) {
                // Pink noise
                float white = lookupTable.getNoise() * 2.f - 1.f;
                pink = 0.98f * pink + 0.02f * white;
                float noise = pink * (1.f - clapNoiseColor) + white * clapNoiseColor;

                float burst = noise * clapEnv;
                out += burst;

                clapEnv *= expf(-1.f / (sampleRate * decayTime));
            } else if (burstIndex >= int(burstCount)) {
                clapActive = false;
            }

            if (clapPunch < 0.0f) {
                // out = CLAMP(out + out * -clapPunch * 8, -1.0f, 1.0f);
                if (burstIndex < int(burstCount * 0.5f)) {
                    out *= 1.f + -clapPunch * 2.f;
                }
            } else if (clapPunch > 0.0f) {
                float t = burstTimer / spacing;
                if (t < 0.02f) {
                    out *= 1.f + clapPunch * 2.f;
                }
            }

            out = applyBandpass(out);

            out *= clapVolume * envAmp;
        }

        return out;
    }

    float bp_x1 = 0.f, bp_x2 = 0.f;
    float bp_y1 = 0.f, bp_y2 = 0.f;
    float applyBandpass(float x)
    {
        // Biquad bandpass filter (cookbook formula)
        float f0 = 1000.f + clapFilter * 3000.f; // 1kHz to 4kHz
        float Q = 1.0f + clapResonance * 3.0f; // Q: 1 to 4

        float omega = 2.f * M_PI * f0 / sampleRate;
        float alpha = sinf(omega) / (2.f * Q);

        float b0 = alpha;
        float b1 = 0.f;
        float b2 = -alpha;
        float a0 = 1.f + alpha;
        float a1 = -2.f * cosf(omega);
        float a2 = 1.f - alpha;

        // Direct Form I
        float y = (b0 / a0) * x + (b1 / a0) * bp_x1 + (b2 / a0) * bp_x2
            - (a1 / a0) * bp_y1 - (a2 / a0) * bp_y2;

        // Shift delay line
        bp_x2 = bp_x1;
        bp_x1 = x;
        bp_y2 = bp_y1;
        bp_y1 = y;

        float gainComp = 1.f + Q;
        return y * gainComp;
    }

    // String
    std::vector<float> delayLine;
    uint32_t stringDelayLen = 0;
    uint32_t stringWritePos = 0;
    float onePoleState = 0.0f;
    float stringTone()
    {
        if (stringDelayLen == 0)
            return 0.0f;

        uint32_t rp = stringWritePos % stringDelayLen;
        uint32_t rp1 = (rp + 1) % stringDelayLen;
        float s0 = delayLine[rp];
        float s1 = delayLine[rp1];
        float out = 0.5f * (s0 + s1);

        // one-pole lowpass
        float cutoff = std::max(0.001f, stringToneLevel * (1.05f - damping) + 0.05f);
        onePoleState += cutoff * (out - onePoleState);
        float filtered = onePoleState;

        // feedback
        float fb = stringDecay;
        delayLine[stringWritePos % stringDelayLen] = filtered * fb;

        stringWritePos = (stringWritePos + 1) % stringDelayLen;

        float outputGain = 2.0f * (1.0f - damping + 0.05f);

        // // Auto stop
        // static float avgEnergy = 0.0f;
        // avgEnergy = 0.999f * avgEnergy + 0.001f * (filtered * filtered); // moving RMS
        // if (avgEnergy < 1e-6f) { // silence threshold
        //     stringDelayLen = 0;
        //     delayLine.clear();
        // }

        float lfo = nextStringLfo();
        float lfoAmp = 1.0f - stringLfoDepth + (lfo * stringLfoDepth * 2.0f);

        return applyStringFx(filtered * outputGain * stringVolume * stringVolume * lfoAmp);
    }
    // TODO fix still play even when finished...

    static constexpr uint32_t MAX_DELAY = 1 << 16; // 65536
    void stringNoteOn(uint8_t note)
    {
        if (stringVolume > 0.0f) {
            note += stringPitch - 24; // Let's remove 2 octaves
            float freq = 440.0f * powf(2.0f, (note - 69) / 12.0f);
            if (freq < 20.0f)
                freq = 20.0f;
            if (freq > sampleRate * 0.45f)
                freq = sampleRate * 0.45f;

            stringDelayLen = std::min<uint32_t>(MAX_DELAY, std::max<uint32_t>(2, (uint32_t)std::round(sampleRate / freq)));
            delayLine.assign(stringDelayLen + 4, 0.0f);

            // White noise
            for (uint32_t i = 0; i < stringDelayLen; ++i) {
                float n = (rand() / (float)RAND_MAX) * 2.0f - 1.0f;
                delayLine[i] = n * (stringPluckNoise + 0.5f);
            }

            stringWritePos = 0;
            onePoleState = 0.0f;
        }
    }

    float stringRingPhase = 0.0f;
    float applyStringFx(float out)
    {
        out = applyRingMod(out, stringRingMod, stringRingPhase, sampleRate);
        out = tinyStringReverb(out);
        return out;
    }

    float stringLfoPhase = 0.0f;
    float nextStringLfo()
    {
        stringLfoPhase += (2.0f * M_PI * stringLfoRate) / sampleRate;
        if (stringLfoPhase > 2.0f * M_PI)
            stringLfoPhase -= 2.0f * M_PI;

        // Sine wave between 0..1
        return 0.5f * (1.0f + sinf(stringLfoPhase));
    }

    float tinyStringReverb(float input)
    {
        // Simple feedback delay reverb — about 12000 samples ≈ 250ms at 48kHz
        static float delay[12000] = { 0 };
        static int pos = 0;

        // Read delayed sample
        float out = delay[pos];

        // Feedback: feed the input plus some of the old signal
        // delay[pos] = input + out * stringReverbFeedback;
        delay[pos] = input + out * (0.5f + stringReverb * 0.4f);


        // Increment and wrap buffer index
        pos = (pos + 1) % 12000;

        // Mix dry/wet with adjustable stringReverb parameter (0.0f – 1.0f)
        float dry = input * (1.0f - stringReverb);
        float wet = out * stringReverb;

        return dry + wet;
    }

public:
    // Tone
    WavetableGenerator waveform;
    EnvelopDrumAmp envelopAmp;
    KickEnvTableGenerator envelopFreq;
    KickTransientTableGenerator transient;

    float toneVolume = 1.0f; // 0.00 to 1.00
    int duration = 1000; // 50 to 3000
    int8_t pitch = -8; // -36 to 36
    float resonator = 0.0f; // 0.00 to 1.50
    float timbre = 0.0f; // 0.00 to 1.00

    // Clap
    EnvelopDrumAmp clapEnvelopAmp;

    float clapVolume = 0.2f;
    float burstSpacing = 0.5f; // 0.0 to 1.0
    float clapDecay = 1.0f; // 0.0 to 1.0
    int8_t burstCount = 4; // 1 to 10
    float clapNoiseColor = 0.5f; // 0.0 to 1.0
    float clapPunch = 0.0f; // -1.0 to 1.0
    float clapFilter = 0.0f; // 0.0 to 1.0
    float clapResonance = 0.0f; // 0.0 to 1.0

    // String

    float stringVolume = 0.5f;
    float damping = 0.5f; // 0.0 to 1.0
    float stringDecay = 0.99f; // 0.80 to 0.99
    float stringToneLevel = 0.5f; // 0.0 to 1.0
    int8_t stringPitch = 0; // -36 to 36
    float stringPluckNoise = 0.5f; // 0.0 to 1.0
    float stringRingMod = 0.0f; // 0.0 to 1.0
    float stringReverb = 0.0f; // 0.0 to 1.0
    float stringReverbFeedback = 0.8f; // 0.0 to 1.0

    float stringLfoRate = 5.0f; // Hz (LFO speed)
    float stringLfoDepth = 0.2f; // 0.0–1.0 amplitude modulation depth

    // Filter
    float filterCutoff = 0.0f; // -1.0 to 1.0

    const static int sampleRate = 48000;
    const static uint8_t channels = 2;

    static Audio* instance;
    static Audio& get()
    {
        if (!instance) {
            instance = new Audio();
        }
        return *instance;
    }

    float sample()
    {
        float sumVolume = toneVolume + clapVolume + stringVolume;
        if (sumVolume == 0.0f) {
            return 0.0f;
        }

        float out = 0.0f;
        if (toneVolume > 0.0f) {
            out += tone();
        }

        if (clapVolume > 0.0f) {
            out += clap();
        }

        if (stringVolume > 0.0f) {
            out += stringTone();
        }

        // add modulation that could turn into FM --> might use page switch on same button
        //       ----> maybe instead add a second layer that could act as FM modulation
        // add fx and f2 using page switch on same button
        //    ----> add master volume, and then could be 3 multi effect

        out = (out * velocity) / sumVolume;
        out = CLAMP(out, -1.0f, 1.0f);
        return out;
    }

    void noteOn(uint8_t note, float _velocity)
    {
        velocity = _velocity;

        // Tone
        totalSamples = static_cast<int>(sampleRate * (duration / 1000.0f));
        envelopAmp.reset(totalSamples);
        sampleCounter = 0;
        sampleIndex = 0.0f;

        // Clap
        float spacing = burstSpacing * 0.03f + 0.01f;
        float decayTime = clapDecay * 0.3f + 0.02f;
        float totalClapTime = (burstCount - 1) * spacing + decayTime * 3.0f;
        int clapTotalSamples = sampleRate * totalClapTime;
        clapEnvelopAmp.reset(clapTotalSamples);
        clapActive = true;
        burstTimer = 0.f;
        burstIndex = 0;
        clapEnv = 1.f;
        pink = 0.f;

        freq = pow(2, ((note - baseNote + pitch) / 12.0));
        baseFreq = freq * 440.0f;

        stringNoteOn(note);
    }

    void noteOff(uint8_t note) { }

    void setResonance(float value)
    {
        resonance = value;
        filter.setResonance(0.95 * (1.0 - std::pow(1.0 - value, 2)));
    }

    float getResonance() { return resonance; }
};

Audio* Audio::instance = NULL;
