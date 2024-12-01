#ifndef _SYNTH_DRUM23_H_
#define _SYNTH_DRUM23_H_

#include <sstream>

#include "./utils/Wavetable.h"
#include "./utils/Waveform.h"
#include "audioPlugin.h"
#include "filter.h"
#include "mapping.h"
// #include "utils/Envelop.h"
#include "utils/EnvelopRelative.h"

#define ZIC_DRUM_ENV_AMP_STEP 4
#define ZIC_DRUM_ENV_FREQ_STEP 4
#define ZIC_DRUM_UI 1000

// Instead to have a fix envelop, should we use an envelop with customizable steps...??

/*md
## SynthDrum23

Synth engine to generate drum sounds using wavetables.
*/
class SynthDrum23 : public Mapping {
protected:
    uint8_t baseNote = 60;
    uint64_t sampleRate;

    Waveform waveform;
    Wavetable wavetable;
    float pitchMult = 1.0f;

    unsigned int sampleCountDuration = 0;
    unsigned int sampleDurationCounter = 0;

    float velocity = 1.0f;
    float noteMult = 1.0f;

    EffectFilterData filter;

    EnvelopRelative envelopAmp = EnvelopRelative({ { 0.0f, 0.0f }, { 1.0f, 0.01f }, { 0.0f, 1.0f } }, 1);
    EnvelopRelative envelopFreq = EnvelopRelative({ { 0.5f, 0.0f }, { 1.0f, 0.5f }, { 0.0f, 1.0f } });

    float index = 0;
    float wave(float amp, float freq, float pitch)
    {
        float modulatedFreq = 110.0f * (pitch + freq);
        return amp * waveform.sample(&index, modulatedFreq);
    }

    float sample(EffectFilterData& _filter, float time, float* index, float amp, float freq, float _noteMult = 1.0f, float _velocity = 1.0f)
    {
        // float out = wavetable.sample(index, amp * _velocity, freq, pitchMult * _noteMult);
        float out = wave(amp * _velocity, freq, pitchMult * _noteMult);
        if (noise.get() > 0.0f) {
            out += 0.01 * props.lookupTable->getNoise() * noise.get() * amp;
        }

        if (resEnv.get() > 0.0f) {
            _filter.setCutoff(amp * 0.85);
            _filter.setSampleData(out);
            out = _filter.lp;
        }

        out = out + out * clipping.pct() * 20;
        return range(out, -1.0f, 1.0f);
    }

public:
    /*md **Values**: */
    /*md - `BROWSER` Select wavetable.*/
    Val& browser = val(0.0f, "BROWSER", { "Browser", VALUE_STRING, .max = (float)wavetable.fileBrowser.count }, [&](auto p) { open(p.value); });
    /*md - `MORPH` Morhp over the wavetable.*/
    Val& morph = val(0.0f, "MORPH", { "Morph", .min = 1.0, .max = ZIC_WAVETABLE_WAVEFORMS_COUNT, .step = 0.1, .floatingPoint = 1 }, [&](auto p) { setMorph(p.value); });
    /*md - `PITCH` Modulate the pitch.*/
    Val& pitch = val(0, "PITCH", { "Pitch", VALUE_CENTERED, .min = -36, .max = 36 }, [&](auto p) { setPitch(p.value); });
    /*md - `DURATION` set the duration of the envelop.*/
    Val& duration = val(100.0f, "DURATION", { "Duration", .min = 10.0, .max = 5000.0, .step = 10.0, .unit = "ms" }, [&](auto p) { setDuration(p.value); });

    /*md - `GAIN_CLIPPING` set the clipping level.*/
    Val& clipping = val(0.0, "GAIN_CLIPPING", { "Gain Clipping", .unit = "%" }, [&](auto p) { setClipping(p.value); });
    /*md - `NOISE` set the noise level.*/
    Val& noise = val(0.0, "NOISE", { "Noise", .unit = "%" }, [&](auto p) { setNoise(p.value); });

    /*md - `RESONANCE_ENV` set resonance using amplitude envelope.*/
    Val& resEnv = val(0.0f, "RESONANCE_ENV", { "Resonance Env.", .unit = "%" }, [&](auto p) { setResonance(p.value); });

    SynthDrum23(AudioPlugin::Props& props, char* _name)
        : Mapping(props, _name)
        , waveform(props.lookupTable, props.sampleRate)
        , sampleRate(props.sampleRate)
    {
        initValues();
        filter.setResonance(0.95f);
    }

    void sample(float* buf)
    {
        if (sampleDurationCounter < sampleCountDuration) {
            float time = (float)sampleDurationCounter / (float)sampleCountDuration;
            float envAmp = envelopAmp.next(time);
            float envFreq = envelopFreq.next(time);
            buf[track] = sample(filter, time, &wavetable.sampleIndex, envAmp, envFreq, noteMult, velocity);
            sampleDurationCounter++;
            // printf("[%d] sample: %d of %d=%f\n", track, sampleDurationCounter, sampleCountDuration, buf[track]);
        }

        buf[track] = buf[track];
    }

    void open(float value)
    {
        browser.setFloat(value);
        int position = browser.get();
        wavetable.open(position, false);
        browser.setString(wavetable.fileBrowser.getFile(position));

        sampleDurationCounter = -1; // set counter to the maximum
        sampleDurationCounter = sampleCountDuration;
    }

    void setResonance(float value)
    {
        resEnv.setFloat(value);
        filter.setResonance(resEnv.pct());
    };

    void setClipping(float value)
    {
        clipping.setFloat(value);
    }

    void setNoise(float value)
    {
        noise.setFloat(value);
    }

    void setPitch(float value)
    {
        pitch.setFloat(value);
        pitchMult = pitch.pct() + 0.5f; // FIXME
    }

    void setMorph(float value)
    {
        morph.setFloat(value);
        wavetable.morph(morph.pct());
    }

    void setDuration(float value)
    {
        duration.setFloat(value);
        bool isOff = sampleCountDuration == sampleDurationCounter;
        sampleCountDuration = duration.get() * (sampleRate * 0.001f);
        if (isOff) {
            sampleDurationCounter = sampleCountDuration;
        }
    }

    void noteOn(uint8_t note, float _velocity) override
    {
        wavetable.sampleIndex = 0;
        sampleDurationCounter = 0;
        envelopAmp.reset();
        envelopFreq.reset();
        velocity = range(_velocity, 0.0f, 1.0f);

        noteMult = pow(2, ((note - baseNote + pitch.get()) / 12.0));
    }

    void serialize(FILE* file, std::string separator) override
    {
        Mapping::serialize(file, separator);
        // Save amp envelop
        fprintf(file, "ENV_AMP");
        for (EnvelopRelative::Data& phase : envelopAmp.data) {
            fprintf(file, " %f:%f", phase.modulation, phase.time);
        }
        fprintf(file, "%s", separator.c_str());

        // Save freq envelop
        fprintf(file, "ENV_FREQ");
        for (EnvelopRelative::Data& phase : envelopFreq.data) {
            fprintf(file, " %f:%f", phase.modulation, phase.time);
        }
        fprintf(file, "%s", separator.c_str());
    }

    void hydrateEnv(EnvelopRelative& env, std::string envData)
    {
        env.data.clear();
        std::stringstream ss(envData);
        std::string token;
        while (ss >> token) {
            float time = 0;
            float mod = 0;
            sscanf(token.c_str(), "%f:%f", &mod, &time);
            // printf("- time: %f mode: %f\n", time, mod);
            env.data.push_back({ mod, time });
        }
    }

    void hydrate(std::string value) override
    {
        if (value.find("ENV_AMP ") != std::string::npos) {
            hydrateEnv(envelopAmp, value.substr(8));
            return;
        }
        if (value.find("ENV_FREQ ") != std::string::npos) {
            hydrateEnv(envelopFreq, value.substr(9));
            return;
        }
        Mapping::hydrate(value);
    }

protected:
    uint16_t msAmp = 0;

public:
    void* data(int id, void* userdata = NULL)
    {
        switch (id) {
        case 0:
            return &envelopAmp.data;
        case 1: // set & get current amp step edit point
            return envelopAmp.updateEditPhase((int8_t*)userdata);
        case 2: { // update amp time for current step
            float* timePct = envelopAmp.updatePhaseTime((int8_t*)userdata);
            msAmp = (uint16_t)(*timePct * duration.get());
            return &msAmp;
        }
        case 3: // update amp modulation value for current step
            return envelopAmp.updatePhaseModulation((int8_t*)userdata);
        case 4:
            return &envelopFreq.data;
        case 5: // set & get current freq step edit point
            return envelopFreq.updateEditPhase((int8_t*)userdata);
        case 6: { // update freq time for current step
            float* timePct = envelopFreq.updatePhaseTime((int8_t*)userdata);
            msAmp = (uint16_t)(*timePct * duration.get());
            return &msAmp;
        }
        case 7: // update freq modulation value for current step
            return envelopFreq.updatePhaseModulation((int8_t*)userdata);
        }
        return NULL;
    }
};

#endif
