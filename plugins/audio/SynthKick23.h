#pragma once

#include "./utils/Wavetable.h"
#include "audioPlugin.h"
#include "filter.h"
#include "mapping.h"
#include "utils/EnvelopRelative.h"

#define ZIC_KICK_ENV_AMP_STEP 4
#define ZIC_KICK_ENV_FREQ_STEP 4
#define ZIC_KICK_UI 1000

// Instead to have a fix envelop, should we use an envelop with customizable steps...??

/*md
## SynthKick23

Synth engine to generate drum sounds using wavetables.
*/
class SynthKick23 : public Mapping {
protected:
    uint8_t baseNote = 60;
    uint64_t sampleRate;

    Wavetable wavetable;
    float pitchMult = 1.0f;

    float bufferUi[ZIC_KICK_UI];
    int updateUiState = 0;
    std::vector<EnvelopRelative::Data>* envelopUi = NULL;

    unsigned int sampleCountDuration = 0;
    unsigned int sampleDurationCounter = 0;

    float velocity = 1.0f;
    float noteMult = 1.0f;

    EffectFilterData filter;

    EnvelopRelative envelopAmp = EnvelopRelative({ { 0.0f, 0.0f }, { 1.0f, 0.01f }, { 0.3f, 0.4f }, { 0.0f, 1.0f }, { 0.0f, 1.0f }, { 0.0f, 1.0f }, { 0.0f, 1.0f } });
    EnvelopRelative envelopFreq = EnvelopRelative({ { 1.0f, 0.0f }, { 0.26f, 0.03f }, { 0.24f, 0.35f }, { 0.22f, 0.4f }, { 0.0f, 1.0f }, { 0.0f, 1.0f } });

    float sample(EffectFilterData& _filter, float time, float* index, float amp, float freq, float _noteMult = 1.0f, float _velocity = 1.0f)
    {
        float out = wavetable.sample(index, freq + pitchMult * _noteMult) * amp * _velocity;

        if (noise.get() > 0.0f) {
            out += 0.01 * props.lookupTable->getNoise() * noise.get();
        }

        if (resEnv.get() > 0.0f) {
            _filter.setCutoff(amp * 0.85);
            _filter.setSampleData(out);
            out = _filter.lp;
        }

        out = out + out * clipping.pct() * 20;
        return range(out, -1.0f, 1.0f);
    }

    void updateUi(std::vector<EnvelopRelative::Data>* envData)
    {
        envelopUi = envData;
        updateUiState++;
    }

public:
    /*md **Values**: */
    /*md - `BROWSER` Select wavetable.*/
    Val& browser = val(1.0f, "BROWSER", { "Browser", VALUE_STRING, .min = 1.0f, .max = (float)wavetable.fileBrowser.count }, [&](auto p) { open(p.value); });
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

    /*#md - `MIX` set mix between audio input and output.*/
    Val& mix = val(100.0f, "MIX", { "Mix in/out", .type = VALUE_CENTERED });
    /*#md - `FM_AMP_MOD` set amplitude modulation amount using audio input.*/
    Val& fmAmpMod = val(0.0f, "FM_AMP_MOD", { "FM.Amp", .unit = "%" });
    /*#md - `FM_FREQ_MOD` set frequency modulation amount using audio input.*/
    Val& fmFreqMod = val(0.0f, "FM_FREQ_MOD", { "FM.Freq", .unit = "%" });

    Val envAmpMod[ZIC_KICK_ENV_AMP_STEP + 1] = {
        { 100.0f, "ENVELOP_AMP_MOD_0", { "Amp.Mod.0", .unit = "%" }, [&](auto p) { setEnvAmpMod(p.value, 0); } },
        { 50.0f, "ENVELOP_AMP_MOD_1", { "Amp.Mod.1", .unit = "%" }, [&](auto p) { setEnvAmpMod(p.value, 1); } },
        { 50.0f, "ENVELOP_AMP_MOD_2", { "Amp.Mod.2", .unit = "%" }, [&](auto p) { setEnvAmpMod(p.value, 2); } },
        { 50.0f, "ENVELOP_AMP_MOD_3", { "Amp.Mod.3", .unit = "%" }, [&](auto p) { setEnvAmpMod(p.value, 3); } },
        { 50.0f, "ENVELOP_AMP_MOD_4", { "Amp.Mod.4", .unit = "%" }, [&](auto p) { setEnvAmpMod(p.value, 4); } },
    };
    Val envAmpTime[ZIC_KICK_ENV_AMP_STEP] = {
        { 50.0f, "ENVELOP_AMP_TIME_1", { "Amp.Time 1", .unit = "%" }, [&](auto p) { setEnvAmpTime(p.value, 0); } },
        { 50.0f, "ENVELOP_AMP_TIME_2", { "Amp.Time 2", .unit = "%" }, [&](auto p) { setEnvAmpTime(p.value, 1); } },
        { 50.0f, "ENVELOP_AMP_TIME_3", { "Amp.Time 3", .unit = "%" }, [&](auto p) { setEnvAmpTime(p.value, 2); } },
        { 50.0f, "ENVELOP_AMP_TIME_4", { "Amp.Time 4", .unit = "%" }, [&](auto p) { setEnvAmpTime(p.value, 3); } },
    };
    Val envFreqMod[ZIC_KICK_ENV_FREQ_STEP + 1] = {
        { 100.0f, "ENVELOP_FREQ_MOD_0", { "Freq.Mod.0", .unit = "%" }, [&](auto p) { setEnvFreqMod(p.value, 0); } },
        { 50.0f, "ENVELOP_FREQ_MOD_1", { "Freq.Mod.1", .unit = "%" }, [&](auto p) { setEnvFreqMod(p.value, 1); } },
        { 50.0f, "ENVELOP_FREQ_MOD_2", { "Freq.Mod.2", .unit = "%" }, [&](auto p) { setEnvFreqMod(p.value, 2); } },
        { 50.0f, "ENVELOP_FREQ_MOD_3", { "Freq.Mod.3", .unit = "%" }, [&](auto p) { setEnvFreqMod(p.value, 3); } },
        { 50.0f, "ENVELOP_FREQ_MOD_4", { "Freq.Mod.4", .unit = "%" }, [&](auto p) { setEnvFreqMod(p.value, 4); } },
    };
    Val envFreqTime[ZIC_KICK_ENV_FREQ_STEP] = {
        { 50.0f, "ENVELOP_FREQ_TIME_1", { "Freq.Time 1", .unit = "%" }, [&](auto p) { setEnvFreqTime(p.value, 0); } },
        { 50.0f, "ENVELOP_FREQ_TIME_2", { "Freq.Time 2", .unit = "%" }, [&](auto p) { setEnvFreqTime(p.value, 1); } },
        { 50.0f, "ENVELOP_FREQ_TIME_3", { "Freq.Time 3", .unit = "%" }, [&](auto p) { setEnvFreqTime(p.value, 2); } },
        { 50.0f, "ENVELOP_FREQ_TIME_4", { "Freq.Time 4", .unit = "%" }, [&](auto p) { setEnvFreqTime(p.value, 3); } },
    };

    SynthKick23(AudioPlugin::Props& props, AudioPlugin::Config& config)
        : Mapping(props, config, {
                                    // clang-format off
            &envAmpMod[0], &envAmpMod[1], &envAmpMod[2], &envAmpMod[3], &envAmpMod[4],
            &envAmpTime[0], &envAmpTime[1], &envAmpTime[2], &envAmpTime[3],
            &envFreqMod[0], &envFreqMod[1], &envFreqMod[2], &envFreqMod[3], &envFreqMod[4],
            &envFreqTime[0], &envFreqTime[1], &envFreqTime[2], &envFreqTime[3],
        }) // clang-format on
        , sampleRate(props.sampleRate)
    {
        initValues();
        filter.setResonance(0.95f);
    }

    void sample(float* buf)
    {
        float input = buf[track];
        if (sampleDurationCounter < sampleCountDuration) {
            float time = (float)sampleDurationCounter / (float)sampleCountDuration;
            float envAmp = envelopAmp.next(time) + input * fmAmpMod.pct();
            float envFreq = envelopFreq.next(time) + input * fmFreqMod.pct();
            buf[track] = sample(filter, time, &wavetable.sampleIndex, envAmp, envFreq, noteMult, velocity);
            sampleDurationCounter++;
            // printf("[%d] sample: %d of %d=%f\n", track, sampleDurationCounter, sampleCountDuration, buf[track]);
        }

        buf[track] = input * (1.0f - mix.pct()) + buf[track] * mix.pct();
    }

    void open(float value)
    {
        browser.setFloat(value);
        int position = browser.get();
        wavetable.open(position, false);
        browser.setString(wavetable.fileBrowser.getFile(position));

        sampleDurationCounter = -1; // set counter to the maximum
        sampleDurationCounter = sampleCountDuration;
        updateUi(NULL);
    }

    void setResonance(float value)
    {
        resEnv.setFloat(value);
        filter.setResonance(resEnv.pct());
        updateUi(NULL);
    };

    void setEnvAmpMod(float value, uint8_t index)
    {
        envAmpMod[index].setFloat(value);
        envelopAmp.data[index + 1].modulation = envAmpMod[index].pct();
        updateUi(&envelopAmp.data);
    }

    void setClipping(float value)
    {
        clipping.setFloat(value);
        updateUiState++;
    }

    void setNoise(float value)
    {
        noise.setFloat(value);
        updateUiState++;
    }

    void setEnvAmpTime(float value, uint8_t index)
    {
        if (value <= 0.0f) {
            return;
        }
        if (index > 0 && envAmpTime[index - 1].get() >= value) {
            return;
        }
        if (index < ZIC_KICK_ENV_AMP_STEP - 1 && envAmpTime[index + 1].get() <= value) {
            return;
        }
        envAmpTime[index].setFloat(value);
        envelopAmp.data[index + 2].time = envAmpTime[index].pct();
        updateUi(&envelopAmp.data);
    }

    void setEnvFreqMod(float value, uint8_t index)
    {
        envFreqMod[index].setFloat(value);
        envelopFreq.data[index].modulation = envFreqMod[index].pct();
        updateUi(&envelopFreq.data);
    }

    void setEnvFreqTime(float value, uint8_t index)
    {
        if (value <= 0.0f) {
            return;
        }
        if (index > 0 && envFreqTime[index - 1].get() > value) {
            return;
        }
        if (index < ZIC_KICK_ENV_FREQ_STEP - 1 && envFreqTime[index + 1].get() < value) {
            return;
        }
        envFreqTime[index].setFloat(value);
        envelopFreq.data[index + 1].time = envFreqTime[index].pct();
        updateUi(&envelopFreq.data);
    }

    void setPitch(float value)
    {
        pitch.setFloat(value);
        pitchMult = pitch.pct() + 0.5f; // FIXME
        updateUi(NULL);
    }

    void setMorph(float value)
    {
        morph.setFloat(value);
        wavetable.morph(morph.pct());
        updateUi(NULL);
        // printf(">>>>>>>>>>>>>>.... sampleStart: %ld (%f bufferSampleCount %ld)\n", sampleStart, morph.get(), bufferSampleCount);
    }

    void setDuration(float value)
    {
        duration.setFloat(value);
        bool isOff = sampleCountDuration == sampleDurationCounter;
        sampleCountDuration = duration.get() * (sampleRate * 0.001f);
        if (isOff) {
            sampleDurationCounter = sampleCountDuration;
        }
        updateUi(NULL);
        // printf(">>>>>>>>>>>>>>.... sampleCountDuration: %d (%d)\n", sampleCountDuration, duration.getAsInt());
    }

    void noteOn(uint8_t note, float _velocity) override
    {
        // printf("kick noteOn: %d %f\n", note, _velocity);

        wavetable.sampleIndex = 0;
        sampleDurationCounter = 0;
        envelopAmp.reset();
        envelopFreq.reset();
        velocity = _velocity;

        noteMult = pow(2, ((note - baseNote + pitch.get()) / 12.0));
    }

    void* data(int id, void* userdata = NULL)
    {
        switch (id) {
        case 0:
            return &updateUiState;

        case 1: {
            unsigned int ampIndex = 0;
            unsigned int freqIndex = 0;
            EffectFilterData _filter;
            _filter.setResonance(resEnv.pct());
            float index = 0;
            for (int i = 0; i < ZIC_KICK_UI; i++) {
                float time = i / (float)ZIC_KICK_UI;
                float envAmp = envelopAmp.next(time, ampIndex);
                float envFreq = envelopFreq.next(time, freqIndex);
                bufferUi[i] = sample(_filter, time, &index, envAmp, envFreq);
            }
            return (void*)&bufferUi;
        }
        case 2:
            return envelopUi;
        case 3:
            return &envelopAmp.data;
        case 4:
            return &envelopFreq.data;
        }
        return NULL;
    }
};
