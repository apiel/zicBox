#ifndef _EFFECT_GRAIN_H_
#define _EFFECT_GRAIN_H_

#include "audioBuffer.h"
#include "audioPlugin.h"
#include "mapping.h"
#include "utils/EnvelopRelative.h"

#define MAX_GRAINS 16

/*md
## EffectGrain

EffectGrain plugin is used to apply granular and scatter effect to a buffer audio.
*/
class EffectGrain : public Mapping {
protected:
    AudioBuffer<> buffer;

    float velocity = 0.0f;
    uint64_t grainDelay = 0;
    uint64_t grainDuration = 0;

    struct Grain {
        uint64_t index = 0;
        float position = 0.0f;
        float positionIncrement = 1.0f;
        EnvelopRelative env = EnvelopRelative({ { 1.0f, 0.0f }, { 1.0f, 0.0f }, { 0.0f, 1.0f } });
    } grains[MAX_GRAINS];

    void initGrain(uint8_t densityIndex)
    {
        Grain& grain = grains[densityIndex];
        grain.index = 0;
        grain.position = buffer.index + densityIndex * grainDelay;
        // grain.positionIncrement = random() * 2.0f - 1.0f;
        grain.env.reset();
    }

public:
    /*md **Values**: */
    /*md - `LENGTH` set the duration of the grain.*/
    Val& length = val(100.0f, "LENGTH", { "Length", .min = 5.0, .max = 100.0, .unit = "ms" }, [&](auto p) { setLength(p.value); });
    /*md - `DENSITY` set the density of the effect, meaning how many grains are played at the same time. */
    Val& density = val(10.0f, "DENSITY", { "Density", .min = 1.0, .max = MAX_GRAINS });
    /*md - `DENSITY_DELAY` set the delay between each grains. */
    Val& densityDelay = val(10.0f, "DENSITY_DELAY", { "Density Delay", .min = 1.0, .max = 1000, .unit = "ms" }, [&](auto p) { setDensityDelay(p.value); });
    /*md - `ENVELOP` set the envelop of the grains. */
    Val& envelop = val(0.0f, "ENVELOP", { "Envelop", .unit = "%" }, [&](auto p) { setEnvelop(p.value); });

    EffectGrain(AudioPlugin::Props& props, char* _name)
        : Mapping(props, _name)
    {
        initValues();
    }

    void setEnvelop(float value)
    {
        envelop.setFloat(value);
        for (uint8_t i = 0; i < MAX_GRAINS; i++) {
            grains[i].env.data[0].time = envelop.pct() * 0.5f;
            grains[i].env.data[1].time = 1.0f - envelop.pct() * 0.5f;
        }
    }

    void setDensityDelay(float value)
    {
        densityDelay.setFloat(value);
        grainDelay = props.sampleRate * densityDelay.get() * 0.001f;
    }

    void setLength(float value)
    {
        length.setFloat(value);
        grainDuration = props.sampleRate * length.get() * 0.001f;
    }

    void sample(float* buf)
    {
        buffer.addSample(buf[track]);

        if (velocity) {
            buf[track] = 0.0f;
            for (uint8_t i = 0; i < density.get(); i++) {
                Grain& grain = grains[i];
                if (grain.index++ < 4000) {
                    grain.position += grain.positionIncrement;
                    while (grain.position >= buffer.size) {
                        grain.position -= buffer.size;
                    }
                } else {
                    initGrain(i);
                }
                buf[track] += buffer.samples[(int)grain.position] * velocity;
            }
        }
    }

    // Might use tempo to define loop/reverse length?
    // 1, 2, 3, 6, 12, 24, 48, 96, ...
    // Could it also use ms for none rhytmic loop?
    // Is this necessary or should it just be dependent buffer size that would be customized depending on loop length...
    void onClockTick(uint64_t* clockCounter)
    {
        // clockCounterPtr = clockCounter;
        // // Clock events are sent at a rate of 24 pulses per quarter note
        // // (24/4 = 6)
        // if (*clockCounter % 6 == 0) {
        //     onStep();
        // }
    }

    void noteOn(uint8_t note, float _velocity) override
    {
        printf("effect grain noteOn: %d %f\n", note, _velocity);

        if (_velocity == 0) {
            return noteOff(note, _velocity);
        }
        velocity = _velocity;
        for (uint8_t i = 0; i < density.get(); i++) {
            initGrain(i);
        }
    }

    void noteOff(uint8_t note, float _velocity) override
    {
        velocity = _velocity;
    }
};

#endif
