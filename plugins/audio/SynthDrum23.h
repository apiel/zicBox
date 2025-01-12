#ifndef _SYNTH_DRUM23_H_
#define _SYNTH_DRUM23_H_

#include <sstream>
#include <vector>

#include "./utils/Waveform.h"
#include "./utils/Wavetable.h"
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

    WaveformInterface* wave = nullptr;
    Waveform waveform;
    Wavetable wavetable;
    float pitchMult = 1.0f;

    unsigned int sampleCountDuration = 0;
    unsigned int sampleDurationCounter = 0;

    float velocity = 1.0f;
    float noteMult = 1.0f;

    EffectFilterData clickFilter;

    EnvelopRelative envelopAmp = EnvelopRelative({ { 0.0f, 0.0f }, { 1.0f, 0.01f }, { 0.0f, 1.0f } }, 1);
    EnvelopRelative envelopFreq = EnvelopRelative({ { 0.5f, 0.0f }, { 1.0f, 0.5f }, { 0.0f, 1.0f } });

    float addClicking(float time, float out, EffectFilterData& _clickFilter)
    {
        // Add a click at the beginning
        float duration = clickDuration.pct(); // Duration of the click in seconds
        float clickAmplitude = click.pct();

        if (clickAmplitude && time < duration) {
            float noise = props.lookupTable->getNoise(); // Get noise sample
            float clickEnv = 1.0f - (time / duration); // Linear fade-out for the click

            // Apply the envelope to the noise
            float rawClick = noise * clickAmplitude * clickEnv;

            _clickFilter.setSampleData(rawClick);
            if (clickCutoff.get() > 0.0f) {
                out += _clickFilter.hp;

            } else {
                out += _clickFilter.lp;
            }
        }

        return out;
    }

    float scaledClipping = 0.0f;

    float sample(float time, float* index, float ampModulation, float freqModulation, float _noteMult, float _velocity, EffectFilterData& _clickFilter)
    {
        float freq = freqModulation + pitchMult * _noteMult;

        float out = wave->sample(index, freq) * ampModulation;

        out = addClicking(time, out, _clickFilter);

        out = out + out * scaledClipping;
        return range(out, -1.0f, 1.0f) * _velocity;
    }

#define DRUM23_WAVEFORMS_COUNT 7
    struct WaveformType {
        std::string name;
        WaveformInterface* wave;
        uint8_t indexType = 0;
    } waveformTypes[DRUM23_WAVEFORMS_COUNT] = {
        { "Wavetable", &wavetable },
        { "Sine", &waveform, Waveform::Type::Sine },
        { "Triangle", &waveform, Waveform::Type::Triangle },
        { "Square", &waveform, Waveform::Type::Square },
        { "Pulse", &waveform, Waveform::Type::Pulse },
        { "FM", &waveform, Waveform::Type::Fm },
        { "FMsquare", &waveform, Waveform::Type::FmSquare },
    };

public:
    /*md **Values**: */
    /*md - `WAVEFORM_TYPE` Select waveform type (wavetable, sine, triangle...).*/
    Val& waveformType = val(0.0f, "WAVEFORM_TYPE", { "Waveform", VALUE_STRING, .max = DRUM23_WAVEFORMS_COUNT - 1 }, [&](auto p) {
        float current = p.val.get();
        p.val.setFloat(p.value);
        if (wave && current == p.val.get()) {
            return;
        }
        WaveformType type = waveformTypes[(int)p.val.get()];
        p.val.setString(type.name);
        wave = type.wave;
        if (p.val.get() != 0.0f) {
            waveform.setWaveformType((Waveform::Type)type.indexType);
            printf("Waveform type: %s shape: %f macro: %f\n", type.name.c_str(), waveform.shape * 100.0f, waveform.macro * 100.0f);
            shape.set(waveform.shape * 1000.0f);
            macro.set(waveform.macro * 100.0f);
        } else {
            shape.set(wavetable.fileBrowser.position);
            macro.set(wavetable.getIndex());
        }
    });
    /*md - `SHAPE` Morhp over the waveform shape.*/
    Val& shape = val(0.0f, "SHAPE", { "Shape", VALUE_STRING, .max = 1000 }, [&](auto p) {
        if (waveformType.get() != 0.0f) {
            int direction = p.value - p.val.get();
            int value = p.val.get() + direction * 10;
            // printf("val: %f, direction: %d, value: %d\n", p.value, direction, value);
            p.val.setFloat(value);
            waveform.setShape(p.val.pct());
            p.val.setString(std::to_string((int)(p.val.get() * 0.1)) + "%");
        } else {
            int value = range(p.value, 0.0f, wavetable.fileBrowser.count);
            p.val.setFloat(value);

            int position = p.val.get();
            wavetable.open(position, false);
            p.val.setString(wavetable.fileBrowser.getFileWithoutExtension(position));

            sampleDurationCounter = -1; // set counter to the maximum
            sampleDurationCounter = sampleCountDuration;
        }
    });

    /*md - `MACRO` Macro is arbitrary parameter depending of selected waveform type. */
    Val& macro = val(0.0f, "MACRO", { "Macro", VALUE_STRING }, [&](auto p) {
        if (waveformType.get() != 0.0f) {
            p.val.setFloat(p.value);
            waveform.setMacro(p.val.pct());
            p.val.setString(std::to_string((int)p.val.get()) + "%");
        } else {
            float value = range(p.value, 1.0f, ZIC_WAVETABLE_WAVEFORMS_COUNT);
            p.val.setFloat(value);
            wavetable.morph((int)p.val.get() - 1);
            p.val.setString(std::to_string((int)p.val.get()) + "/" + std::to_string(ZIC_WAVETABLE_WAVEFORMS_COUNT));
        }
    });

    /*md - `PITCH` Modulate the pitch.*/
    Val& pitch = val(0, "PITCH", { "Pitch", VALUE_CENTERED, .min = -36, .max = 36 }, [&](auto p) { setPitch(p.value); });
    /*md - `DURATION` set the duration of the envelop.*/
    Val& duration = val(100.0f, "DURATION", { "Duration", .min = 10.0, .max = 5000.0, .step = 10.0, .unit = "ms" }, [&](auto p) { setDuration(p.value); });

    /*md - `GAIN_CLIPPING` set the clipping level.*/
    Val& clipping = val(0.0, "GAIN_CLIPPING", { "Clipping", .unit = "%" }, [&](auto p) {
        p.val.setFloat(p.value);
        scaledClipping = p.val.pct() * p.val.pct() * 20;
    });

    /*md - `CLICK` set the click level.*/
    Val& click = val(0, "CLICK", { "Click" });

    /*md - `CLICK_DURATION` set the duration of the click.*/
    Val& clickDuration = val(0.1f, "CLICK_DURATION", { "Click Dur.", .step = 0.1, .floatingPoint = 1, .unit = "%" });

    /*md - `CLICK_CUTOFF` set the cutoff frequency of the click.*/
    Val& clickCutoff = val(50.0f, "CLICK_CUTOFF", { "Click LP|HP", VALUE_CENTERED, .min = -100.0, .max = 100.0, .unit = "%" }, [&](auto p) {
        p.val.setFloat(p.value);
        int value = std::abs(p.val.get());
        clickFilter.setCutoff(value * 0.01f);
        p.val.setString((p.val.get() < 0 ? "LP" : "HP") + std::to_string(value));
    });

    /*md - `CLICK_RESONANCE` set the resonance of the click.*/
    Val& clickResonance = val(75.0f, "CLICK_RESONANCE", { "Click Reso." }, [&](auto p) {
        p.val.setFloat(p.value);
        clickFilter.setResonance(p.val.pct());
    });

    SynthDrum23(AudioPlugin::Props& props, char* _name)
        : Mapping(props, _name)
        , waveform(props.lookupTable, props.sampleRate)
        , sampleRate(props.sampleRate)
    {
        initValues();

        clickFilter.setResonance(0.85);
        clickFilter.setCutoff(0.10);
    }

    void sample(float* buf)
    {
        if (sampleDurationCounter < sampleCountDuration) {
            float time = (float)sampleDurationCounter / (float)sampleCountDuration;
            float envAmp = envelopAmp.next(time);
            float envFreq = envelopFreq.next(time);
            buf[track] = sample(time, &wavetable.sampleIndex, envAmp, envFreq, noteMult, velocity, clickFilter);
            sampleDurationCounter++;
            // printf("[%d] sample: %d of %d=%f\n", track, sampleDurationCounter, sampleCountDuration, buf[track]);
        }
    }

    void setPitch(float value)
    {
        pitch.setFloat(value);
        pitchMult = pitch.pct() + 0.5f; // FIXME
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
    enum DATA_ID {
        ENV_AMP,
        ENV_AMP_EDIT,
        ENV_AMP_TIME,
        ENV_AMP_MOD,
        ENV_FREQ,
        ENV_FREQ_EDIT,
        ENV_FREQ_TIME,
        ENV_FREQ_MOD,
        WAVEFORM,
    };

    /*md **Data ID**: */
    uint8_t getDataId(std::string name) override
    {
        /*md - `ENV_AMP` update the amplitude for current step */
        if (name == "ENV_AMP")
            return ENV_AMP;
        /*md - `ENV_AMP_EDIT` set/get the amplitude edit point for current step */
        if (name == "ENV_AMP_EDIT")
            return ENV_AMP_EDIT;
        /*md - `ENV_AMP_TIME` update the amplitude time for current step */
        if (name == "ENV_AMP_TIME")
            return ENV_AMP_TIME;
        /*md - `ENV_AMP_MOD` update the amplitude modulation value for current step */
        if (name == "ENV_AMP_MOD")
            return ENV_AMP_MOD;
        /*md - `ENV_FREQ` update the frequency for current step */
        if (name == "ENV_FREQ")
            return ENV_FREQ;
        /*md - `ENV_FREQ_EDIT` set/get the frequency edit point for current step */
        if (name == "ENV_FREQ_EDIT")
            return ENV_FREQ_EDIT;
        /*md - `ENV_FREQ_TIME` update the frequency time for current step */
        if (name == "ENV_FREQ_TIME")
            return ENV_FREQ_TIME;
        /*md - `ENV_FREQ_MOD` update the frequency modulation value for current step */
        if (name == "ENV_FREQ_MOD")
            return ENV_FREQ_MOD;
        /*md - `WAVEFORM` return a representation of the selected waveform */
        if (name == "WAVEFORM")
            return WAVEFORM;
        return atoi(name.c_str());
    }

    void* data(int id, void* userdata = NULL) override
    {
        switch (id) {
        case ENV_AMP:
            return &envelopAmp.data;
        case ENV_AMP_EDIT: // set & get current amp step edit point
            return envelopAmp.updateEditPhase((int8_t*)userdata);
        case ENV_AMP_TIME: { // update amp time for current step
            float* timePct = envelopAmp.updatePhaseTime((int8_t*)userdata);
            msAmp = (uint16_t)(*timePct * duration.get());
            return &msAmp;
        }
        case ENV_AMP_MOD: // update amp modulation value for current step
            return envelopAmp.updatePhaseModulation((int8_t*)userdata);
        case ENV_FREQ:
            return &envelopFreq.data;
        case ENV_FREQ_EDIT: // set & get current freq step edit point
            return envelopFreq.updateEditPhase((int8_t*)userdata);
        case ENV_FREQ_TIME: { // update freq time for current step
            float* timePct = envelopFreq.updatePhaseTime((int8_t*)userdata);
            msAmp = (uint16_t)(*timePct * duration.get());
            return &msAmp;
        }
        case ENV_FREQ_MOD: // update freq modulation value for current step
            return envelopFreq.updatePhaseModulation((int8_t*)userdata);
        case WAVEFORM: { // pointer to waveform sample
            if (!wave) {
                return NULL;
            }
            float* index = (float*)userdata;
            return wave->sample(index);
        }
        }
        return NULL;
    }
};

#endif
