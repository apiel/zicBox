#ifndef _EFFECT_GRAIN_H_
#define _EFFECT_GRAIN_H_

#include "audioBuffer.h"
#include "audioPlugin.h"
#include "mapping.h"
#include "utils/EnvelopRelative.h"
#include "filter.h"

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

    uint8_t baseNote = 60;
    float positionIncrement = 0.0f;

    enum FilterMode {
        OFF,
        LPF,
        HPF,
        BPF,
        RAND,
        MODE_COUNT,
    } mode
        = OFF;

    struct Grain {
        uint64_t index = 0;
        float position = 0.0f;
        float positionIncrement = 1.0f;
        EnvelopRelative env = EnvelopRelative({ { 0.0f, 0.0f }, { 1.0f, 0.0f }, { 1.0f, 0.0f }, { 0.0f, 1.0f } });
        EffectFilterData filter;
        FilterMode filterMode = OFF;
    } grains[MAX_GRAINS];

    void initGrain(uint8_t densityIndex)
    {
        Grain& grain = grains[densityIndex];
        grain.index = 0;
        grain.position = buffer.index + densityIndex * grainDelay + grainDelay * getRand() * delayRandomize.pct();
        grain.positionIncrement = positionIncrement * direction.get() + positionIncrement * getRand() * pitchRandomize.pct();
        grain.env.reset();
        grain.filterMode = mode == RAND ? (FilterMode)((uint8_t)(getRand() * 3) % 3 + 1) : mode;
        if (grain.filterMode != OFF) {
            float _cutoff = cutoff.pct() + (getRand() - 0.5f) * 0.5f * cutoffRandomize.pct();
            grain.filter.cutoff = getCutoff(grain.filterMode, _cutoff);
            grain.filter.setResonance(resonance.pct());
        }
    }

    float getCutoff(FilterMode _mode, float _cutoff)
    {
        if (_mode == LPF) {
            return 0.85 * _cutoff + 0.1;
        } else if (_mode == BPF) {
            return 0.85 * _cutoff + 0.1;
        }
        // HPF
        return (0.20 * _cutoff) + 0.00707;
    }

    float filterSample(float inputValue, Grain& grain)
    {
        if (inputValue == 0 || grain.filterMode == OFF) {
            return inputValue;
        }

        grain.filter.setSampleData(inputValue);

        if (grain.filter.lp < -1.0f || grain.filter.lp > 1.0f || grain.filter.hp < -1.0f || grain.filter.hp > 1.0f || grain.filter.bp < -1.0f || grain.filter.bp > 1.0f) {
                printf("in %f lp %f hp %f bp %f\n", inputValue, grain.filter.lp, grain.filter.hp, grain.filter.bp);    
        }

        if (grain.filterMode == HPF) {
            return grain.filter.hp;
        } else if (grain.filterMode == BPF) {
            return grain.filter.bp;
        }
        // LPF
        return grain.filter.lp;
    }

    float getRand()
    {
        return props.lookupTable->getNoise();
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
    /*md - `PITCH` Modulate the pitch.*/
    Val& pitch = val(0, "PITCH", { "Pitch", VALUE_CENTERED, .min = -12, .max = 12 });
    /*md - `DELAY_RANDOMIZE` set the density delay randomize. If randomize is set, the density starting delay is random and while change on each sustain loop. */
    Val& delayRandomize = val(0.0f, "DELAY_RANDOMIZE", { "Delay Rand.", .unit = "%" });
    /*md - `PITCH_RANDOMIZE` set the pitch randomize. If randomize is set, the pitch is random and while change on each sustain loop.` */
    Val& pitchRandomize = val(0.0f, "PITCH_RANDOMIZE", { "Pitch Rand.", .unit = "%" });
    /*md - `DIRECTION` set the direction of the grain. */
    Val& direction = val(1, "DIRECTION", { "Direction", VALUE_CENTERED, .min = -1, .max = 1, .step = 2 });

    /*md - `FILTER_MODE` set filter mode. */
    Val& filterMode = val(0.0, "FILTER_MODE", { "Filter Mode", VALUE_STRING, .max = MODE_COUNT - 1, }, [&](auto p) { setFilterMode(p.value); });
    /*md - `CUTOFF` set cutoff. */
    Val& cutoff = val(50.0, "CUTOFF", { "Cutoff", .unit = "%" });
    /*md - `CUTOFF_RANDOMIZE` set cutoff randomize. */
    Val& cutoffRandomize = val(50.0, "CUTOFF_RANDOMIZE", { "Cutoff Rand.", .unit = "%" });
    /*md - `RESONANCE` set resonance. */
    Val& resonance = val(0.0, "RESONANCE", { "Resonance", .unit = "%" });

    EffectGrain(AudioPlugin::Props& props, char* _name)
        : Mapping(props, _name)
    {
        initValues();
    }

    std::string getMode(enum FilterMode _mode)
    {
        switch (_mode) {
        case OFF:
            return "OFF";
       case RAND:
            return "Rand";
        case LPF:
            return "LPF";
        case HPF:
            return "HPF";
        case BPF:
            return "BPF";
        default:
            return "UNKNOWN";
        }
    }

    void setFilterMode(float value)
    {
        filterMode.setFloat(value);
        mode = (FilterMode)filterMode.get();
        filterMode.setString(getMode(mode));
    };

    void setEnvelop(float value)
    {
        envelop.setFloat(value);
        for (uint8_t i = 0; i < MAX_GRAINS; i++) {
            grains[i].env.data[1].time = envelop.pct() * 0.5f;
            grains[i].env.data[2].time = 1.0f - envelop.pct() * 0.5f;
            // printf("%f %f\n", grains[i].env.data[0].time, grains[i].env.data[1].time);
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
                if (grain.index++ < grainDuration) {
                    grain.position += grain.positionIncrement;
                    while (grain.position >= buffer.size) {
                        grain.position -= buffer.size;
                    }
                    while (grain.position < 0) {
                        grain.position += buffer.size;
                    }
                } else {
                    initGrain(i);
                }
                float env = grain.env.next(grain.index / (float)grainDuration);
                // printf("time %f env %f\n", grain.index / (float)grainDuration, env);

                float out = buffer.samples[(int)grain.position] * velocity * env;
                out = filterSample(out, grain);
                buf[track] += out;
            }
        }
    }

    void noteOn(uint8_t note, float _velocity) override
    {
        if (_velocity == 0) {
            return noteOff(note, _velocity);
        }
        velocity = _velocity;
        positionIncrement = pow(2, ((note - baseNote + pitch.get()) / 12.0));
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
