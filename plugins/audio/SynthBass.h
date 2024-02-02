#ifndef _SYNTH_BASS_H_
#define _SYNTH_BASS_H_

#include <sndfile.h>

#include "audioPlugin.h"
#include "fileBrowser.h"
#include "filter.h"
#include "mapping.h"
#include "utils/Envelop.h"

#define ZIC_BASS_UI 1000

/*md
## SynthBass

Synth engine to generate bass sounds.
*/
class SynthBass : public Mapping {
protected:
    EffectFilterData filter;

    // could be use make sample representation for a note duration
    float bufferUi[ZIC_BASS_UI];

    // Envelop might need a bit of curve??
    Envelop envelop = Envelop({ { 0.0f, 0.0f }, { 1.0f, 0.01f }, { 0.3f, 0.4f }, { 0.0f, 1.0f } });

    unsigned int sampleCountDuration = 0;
    unsigned int sampleIndex = 0;
    float sampleValue = 0.0f;
    float stepIncrement = 0.001f;
    float velocity = 1.0f;

    int updateUiState = 0;

public:
    /*md **Values**: */
    /*md - `PITCH` set the pitch.*/
    Val& pitch = val(0, "PITCH", { "Pitch", .min = -24, .max = 24 }, [&](auto p) { setPitch(p.value); });
    /*md - `DURATION` set the duration of the envelop.*/
    Val& duration = val(100.0f, "DURATION", { "Duration", .min = 100.0, .max = 5000.0, .step = 100.0, .unit = "ms" }, [&](auto p) { setDuration(p.value); });
    /*md - `DECAY_LEVEL` set the decay level.*/
    Val& decayLevel = val(0.5f, "DECAY_LEVEL", { "Decay Level", .unit = "%" }, [&](auto p) { setDecayLevel(p.value); });
    /*md - `DECAY_TIME` set the decay time percentage base on the total duration.*/
    Val& decayTime = val(100.0f, "DECAY_TIME", { "Decay Time", .unit = "%" }, [&](auto p) { setDecayTime(p.value); });
    /*md - `CUTOFF` to set cutoff frequency and switch between low and high pass filter. */
    Val& cutoff = val(50.0, "CUTOFF", { "Cutoff", .unit = "%" }, [&](auto p) { setCutoff(p.value); });
    /*md - `RESONANCE` to set resonance. */
    Val& resonance = val(0.0, "RESONANCE", { "Resonance", .unit = "%" }, [&](auto p) { setResonance(p.value); });
    /*md - `DRIVE` to set drive. */
    Val& drive = val(50, "DRIVE", { "F.Drive" }, [&](auto p) { setDrive(p.value); });

    SynthBass(AudioPlugin::Props& props, char* _name)
        : Mapping(props, _name) // clang-format on
    {
        initValues();
    }

    void sample(float* buf)
    {
        if (sampleIndex < sampleCountDuration) {
            sampleIndex++;
            sampleValue += stepIncrement;
            if (sampleValue >= 1.0) {
                sampleValue = -1.0;
            }
            float time = (float)sampleIndex / (float)sampleCountDuration;
            float env = envelop.next(time);
            filter.setCutoff(0.85 * cutoff.pct() * env + 0.1);
            filter.setSampleData(sampleValue * velocity * env);
            buf[track] = filter.buf0;

            // buf[track] = sampleValue * velocity * env;
        }
    }

    void setDecayLevel(float value)
    {
        decayLevel.setFloat(value);
        envelop.data[2].modulation = decayLevel.pct();
        updateUiState++;
    }

    void setDecayTime(float value)
    {
        decayTime.setFloat(value);
        envelop.data[2].time = decayTime.pct();
        updateUiState++;
    }

    void setPitch(float value)
    {
        pitch.setFloat(value);
        // pitchMult = pitch.pct() + 0.5f; // FIXME
        updateUiState++;
    }

    void setDuration(float value)
    {
        duration.setFloat(value);
        bool isOff = sampleCountDuration == sampleIndex;
        sampleCountDuration = duration.get() * (props.sampleRate * 0.001f);
        if (isOff) {
            sampleIndex = sampleCountDuration;
        }
        // printf(">>>>>>>>>>>>>>.... sampleCountDuration: %d (%d)\n", sampleCountDuration, duration.getAsInt());
        updateUiState++;
    }

    void setDrive(float value)
    {
        drive.setFloat(value);
        filter.setDrive(drive.pct());
    }

    void setCutoff(float value)
    {
        cutoff.setFloat(value);
        filter.setCutoff(0.85 * cutoff.pct() + 0.1);
    }

    void setResonance(float value)
    {
        resonance.setFloat(value);
        filter.setResonance(resonance.pct());
    };

    void noteOn(uint8_t note, float _velocity) override
    {
        velocity = _velocity;
        printf("bass noteOn: %d %f\n", note, velocity);

        sampleIndex = 0;
        sampleValue = 0.0f;
        envelop.reset();
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
            for (int i = 0; i < ZIC_BASS_UI; i++) {
                // float time = i / (float)ZIC_BASS_UI;
                // float envAmp = envelop.next(time, &ampIndex);
                // float envFreq = envelopFreq.next(time, &freqIndex);
                // bufferUi[i] = sample(time, &index, envAmp, envFreq);
                bufferUi[i] = 0.0f;
            }
            return (void*)&bufferUi;
        }
        case 2:
            return &envelop.data;
        }
        return NULL;
    }
};

#endif
