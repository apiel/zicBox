#ifndef _SYNTH_KICK23_H_
#define _SYNTH_KICK23_H_

#include <sndfile.h>

#include "audioPlugin.h"
#include "fileBrowser.h"
#include "mapping.h"
#include "utils/EnvelopRelative.h"
#include "../../helpers/random.h"

#define ZIC_WAVETABLE_WAVEFORMS_COUNT 64
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
    SF_INFO sfinfo;
    SNDFILE* file = NULL;
    uint64_t sampleRate;

    uint64_t sampleCount = 2048; // should this be configurable?
    static const uint64_t bufferSize = ZIC_WAVETABLE_WAVEFORMS_COUNT * 2048;
    uint64_t bufferSampleCount = 0;
    float bufferSamples[bufferSize];
    float bufferUi[ZIC_KICK_UI];
    int updateUiState = 0;
    std::vector<EnvelopRelative::Data>* envelopUi = NULL;

    FileBrowser fileBrowser = FileBrowser("./wavetables");

    float pitchMult = 1.0f;

    float sampleIndex = 0.0f;
    uint64_t sampleStart = 0;

    Random random;

    unsigned int sampleCountDuration = 0;
    unsigned int sampleDurationCounter = 0;

    EnvelopRelative envelopAmp = EnvelopRelative({ { 0.0f, 0.0f }, { 1.0f, 0.01f }, { 0.3f, 0.4f }, { 0.0f, 1.0f }, { 0.0f, 1.0f }, { 0.0f, 1.0f }, { 0.0f, 1.0f } });
    EnvelopRelative envelopFreq = EnvelopRelative({ { 1.0f, 0.0f }, { 0.26f, 0.03f }, { 0.24f, 0.35f }, { 0.22f, 0.4f }, { 0.0f, 1.0f }, { 0.0f, 1.0f } });

    float sample(float time, float* index, float envAmp, float envFreq)
    {
        // printf("(%f) envAmp %f envFreq %f\n", time, envAmp, envFreq);

        (*index) += pitchMult * envFreq;
        while ((*index) >= sampleCount) {
            (*index) -= sampleCount;
        }
        float out = bufferSamples[(uint16_t)(*index) + sampleStart] * envAmp;
        if (noise.get() > 0.0f) {
            out += 0.01 * random.pct() * noise.get();
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
    Val& browser = val(0.0f, "BROWSER", { "Browser", VALUE_STRING, .max = (float)fileBrowser.count }, [&](auto p) { open(p.value); });
    /*md - `MORPH` Morhp over the wavetable.*/
    Val& morph = val(0.0f, "MORPH", { "Morph", .min = 1.0, .max = ZIC_WAVETABLE_WAVEFORMS_COUNT, .step = 0.1, .floatingPoint = 1 }, [&](auto p) { setMorph(p.value); });
    /*md - `PITCH` Modulate the pitch.*/
    Val& pitch = val(0, "PITCH", { "Pitch", VALUE_CENTERED, .min = -12, .max = 12 }, [&](auto p) { setPitch(p.value); });
    /*md - `DURATION` set the duration of the envelop.*/
    Val& duration = val(100.0f, "DURATION", { "Duration", .min = 10.0, .max = 5000.0, .step = 10.0, .unit = "ms" }, [&](auto p) { setDuration(p.value); });
    // ///*md - `ENV_FREQ_START` set the frequence value when the envelop start.*/
    // Val& envFreqStart = val(1.0f, "ENVELOP_FREQ_MOD_0", { "Freq.Mod.0", .min = 0.0, .max = 2.0, .step = 0.1 }, [&](auto p) { setEnvFreqStart(p.value); });

    /*md - `GAIN_CLIPPING` set the clipping level.*/
    Val& clipping = val(0.0, "GAIN_CLIPPING", { "Gain Clipping", .unit = "%" }, [&](auto p) { setClipping(p.value); });
    /*md - `NOISE` set the noise level.*/
    Val& noise = val(0.0, "NOISE", { "Noise", .unit = "%" }, [&](auto p) { setNoise(p.value); });

    /*//md - `MIX` set mix between audio input and output.*/
    Val& mix = val(100.0f, "MIX", { "Mix in/out", .type = VALUE_CENTERED });
    /*//md - `FM_AMP_MOD` set amplitude modulation amount using audio input.*/
    Val& fmAmpMod = val(0.0f, "FM_AMP_MOD", { "FM.Amp", .unit = "%" });
    /*//md - `FM_FREQ_MOD` set frequency modulation amount using audio input.*/
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

    SynthKick23(AudioPlugin::Props& props, char* _name)
        : Mapping(props, _name, {
                                    // clang-format off
            &envAmpMod[0], &envAmpMod[1], &envAmpMod[2], &envAmpMod[3], &envAmpMod[4],
            &envAmpTime[0], &envAmpTime[1], &envAmpTime[2], &envAmpTime[3],
            &envFreqMod[0], &envFreqMod[1], &envFreqMod[2], &envFreqMod[3], &envFreqMod[4],
            &envFreqTime[0], &envFreqTime[1], &envFreqTime[2], &envFreqTime[3],
        }) // clang-format on
        , sampleRate(props.sampleRate)
    {
        memset(&sfinfo, 0, sizeof(sfinfo));

        open(browser.get(), true);

        for (int i = 0; i < ZIC_KICK_ENV_AMP_STEP; i++) {
            envAmpMod[i].setFloat(envelopAmp.data[i + 2].modulation * 100.0f);
            envAmpTime[i].setFloat(envelopAmp.data[i + 2].time * 100.0f);
        }
        for (int i = 0; i < ZIC_KICK_ENV_FREQ_STEP; i++) {
            envFreqMod[i].setFloat(envelopFreq.data[i + 1].modulation * 100.0f);
            envFreqTime[i].setFloat(envelopFreq.data[i + 1].time * 100.0f);
        }

        initValues();
    }

    void sample(float* buf)
    {
        float input = buf[track];
        if (sampleDurationCounter < sampleCountDuration) {
            float time = (float)sampleDurationCounter / (float)sampleCountDuration;
            float envAmp = envelopAmp.next(time) + input * fmAmpMod.pct();
            float envFreq = envelopFreq.next(time) + input * fmFreqMod.pct();
            buf[track] = sample(time, &sampleIndex, envAmp, envFreq);
            sampleDurationCounter++;
            // printf("[%d] sample: %d of %d=%f\n", track, sampleDurationCounter, sampleCountDuration, buf[track]);
        }

        buf[track] = input * (1.0f - mix.pct()) + buf[track] * mix.pct();
    }

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
        sampleStart = morph.pct() * bufferSampleCount;
        uint64_t max = bufferSampleCount / ZIC_WAVETABLE_WAVEFORMS_COUNT * (ZIC_WAVETABLE_WAVEFORMS_COUNT - 1); // TODO make this better :p
        if (sampleStart > max) {
            sampleStart = max;
        }
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

    SynthKick23& close()
    {
        if (file) {
            sf_close(file);
        }
        return *this;
    }

    SynthKick23& open(std::string filename)
    {
        close();

        if (!(file = sf_open(filename.c_str(), SFM_READ, &sfinfo))) {
            debug("Error: could not open file %s\n", filename);
            return *this;
        }
        debug("Audio file %s sampleCount %ld sampleRate %d\n", filename, (long)sfinfo.frames, sfinfo.samplerate);

        bufferSampleCount = sf_read_float(file, bufferSamples, bufferSize);
        sampleCount = bufferSampleCount / (float)ZIC_WAVETABLE_WAVEFORMS_COUNT;

        sampleDurationCounter = sampleCountDuration;
        updateUi(NULL);

        return *this;
    }

    SynthKick23& open(float value, bool force)
    {
        browser.setFloat(value);
        int position = browser.get();
        if (force || position != fileBrowser.position) {
            browser.setString(fileBrowser.getFile(position));
            std::string filepath = fileBrowser.getFilePath(position);
            debug("KICK23_SAMPLE_SELECTOR: %f %s\n", value, filepath.c_str());
            open(filepath);
        }
        return *this;
    }

    SynthKick23& open(float value)
    {
        open(value, false);
        return *this;
    }

    void noteOn(uint8_t note, float velocity) override
    {
        // TODO use velocity
        printf("kick noteOn: %d %f\n", note, velocity);

        if (siblingPlugin) {
            siblingPlugin->noteOn(note, velocity);
        }

        // Could change the frequency base on the note...
        // Could change the amplitude base on the velocity...
        sampleIndex = 0;
        sampleDurationCounter = 0;
        envelopAmp.reset();
        envelopFreq.reset();
    }

    AudioPlugin* siblingPlugin = NULL;

    bool config(char* key, char* value)
    {
        if (strcmp(key, "FORWARD_NOTE_ON") == 0) {
            siblingPlugin = props.audioPluginHandler->getPluginPtr(value);
            return true;
        }

        return Mapping::config(key, value);
    }

    void* data(int id, void* userdata = NULL)
    {
        switch (id) {
        case 0:
            return &updateUiState;

        case 1: {
            unsigned int ampIndex = 0;
            unsigned int freqIndex = 0;
            float index = 0;
            for (int i = 0; i < ZIC_KICK_UI; i++) {
                float time = i / (float)ZIC_KICK_UI;
                float envAmp = envelopAmp.next(time, ampIndex);
                float envFreq = envelopFreq.next(time, freqIndex);
                bufferUi[i] = sample(time, &index, envAmp, envFreq);
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

#endif
