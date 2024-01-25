#ifndef _EFFECT_DELAY_H_
#define _EFFECT_DELAY_H_

#include "EffectFilter.h"
#include "audioBuffer.h"
#include "audioPlugin.h"
#include "mapping.h"

#define MAX_DELAY_VOICES 8

/*md
## EffectDelay

EffectDelay plugin is used to apply delay or reverb effect on audio buffer.

> [!NOTE]
> - `TODO` load/save different kind of delay and reverb from a config file
> - `TODO` add lfo on time ratio
*/
class EffectDelay : public Mapping {
protected:
    uint64_t sampleRate;
    AudioBuffer<> buffer;

    float sample(float in)
    {
        buffer.addSample(in);

        float delay = 0.0f;
        for (uint8_t i = 0; i < MAX_DELAY_VOICES; i++) {
            DelayVoice& voice = voices[i];
            if (voice.index++ >= buffer.size) {
                voice.index = 0;
            }
            if (masterAmplitude.pct() && voice.amplitude.pct() > 0.0f) {
                delay += buffer.samples[voice.index] * voice.amplitude.pct() * masterAmplitude.pct();
                if (voice.feedback.pct() > 0.0f) {
                    buffer.samples[buffer.index] += delay * voice.feedback.pct();
                }
            }
        }

        return in + filter.sample(delay);
    }

    /*md **Values**: */
    struct DelayVoice {
        uint64_t index;
        Val amplitude;
        Val feedback;
        Val sec;
    } voices[MAX_DELAY_VOICES] = {
        { 0,
            /*md - Delay has 8 voices */
            /*md   - `AMPLITUDE_0` to set amplitude on voice 0. */
            /*md   - `AMPLITUDE_1` to set amplitude on voice 1. */
            /*md   - ... */
            /*md   - `FEEDBACK_0` to set feedback on voice 0. */
            /*md   - `FEEDBACK_1` to set feedback on voice 1. */
            /*md   - ... */
            /*md   - `SEC_0` to set time ratio for voice 0. */
            /*md   - `SEC_1` to set time ratio for voice 1. */
            /*md   - ... */
            { 0.0, "AMPLITUDE_0", {}, [&](auto p) { setAmplitude(0, p.value); } },
            { 0.0, "FEEDBACK_0", {}, [&](auto p) { setFeedback(0, p.value); } },
            { 10.0, "SEC_0", {}, [&](auto p) { setSec(0, p.value); } } }, // FIXME this is not seconds...
        { 1,
            { 0.0, "AMPLITUDE_1", {}, [&](auto p) { setAmplitude(1, p.value); } },
            { 0.0, "FEEDBACK_1", {}, [&](auto p) { setFeedback(1, p.value); } },
            { 10.0, "SEC_1", {}, [&](auto p) { setSec(1, p.value); } } },
        { 2,
            { 0.0, "AMPLITUDE_2", {}, [&](auto p) { setAmplitude(2, p.value); } },
            { 0.0, "FEEDBACK_2", {}, [&](auto p) { setFeedback(2, p.value); } },
            { 10.0, "SEC_2", {}, [&](auto p) { setSec(2, p.value); } } },
        { 3,
            { 0.0, "AMPLITUDE_3", {}, [&](auto p) { setAmplitude(3, p.value); } },
            { 0.0, "FEEDBACK_3", {}, [&](auto p) { setFeedback(3, p.value); } },
            { 10.0, "SEC_3", {}, [&](auto p) { setSec(3, p.value); } } },
        { 4,
            { 0.0, "AMPLITUDE_4", {}, [&](auto p) { setAmplitude(4, p.value); } },
            { 0.0, "FEEDBACK_4", {}, [&](auto p) { setFeedback(4, p.value); } },
            { 10.0, "SEC_4", {}, [&](auto p) { setSec(4, p.value); } } },
        { 5,
            { 0.0, "AMPLITUDE_5", {}, [&](auto p) { setAmplitude(5, p.value); } },
            { 0.0, "FEEDBACK_5", {}, [&](auto p) { setFeedback(5, p.value); } },
            { 10.0, "SEC_5", {}, [&](auto p) { setSec(5, p.value); } } },
        { 6,
            { 0.0, "AMPLITUDE_6", {}, [&](auto p) { setAmplitude(6, p.value); } },
            { 0.0, "FEEDBACK_6", {}, [&](auto p) { setFeedback(6, p.value); } },
            { 10.0, "SEC_6", {}, [&](auto p) { setSec(6, p.value); } } },
        { 7,
            { 0.0, "AMPLITUDE_7", {}, [&](auto p) { setAmplitude(7, p.value); } },
            { 0.0, "FEEDBACK_7", {}, [&](auto p) { setFeedback(7, p.value); } },
            { 10.0, "SEC_7", {}, [&](auto p) { setSec(7, p.value); } } },
    };

public:
    /*md - `TIME_RATIO` to modulate time ratio for all voices.*/
    Val timeRatio = { 100.0f, "TIME_RATIO", { "Time Ratio", .unit = "%" }, [&](auto p) { setTimeRatio(p.value); } };
    /*md - `MASTER_AMPLITUDE` to set master amplitude.*/
    Val masterAmplitude = { 0.0f, "MASTER_AMPLITUDE", { "Master Amplitude", .unit = "%" }, [&](auto p) { setMasterAmplitude(p.value); } };

    /*md - `CUTOFF` to set cutoff on delay buffer.*/
    Val cutoff = { 0.0f, "CUTOFF", { "Cutoff" }, [&](auto p) { setCutoff(p.value); } };
    /*md - `RESONANCE` to set resonance on delay buffer.*/
    Val resonance = { 0.0f, "RESONANCE", { "Resonance", .unit = "%" }, [&](auto p) { setResonance(p.value); } };
    /*md - `MODE` to set filter mode.*/
    Val mode = { 0.0f, "MODE", {}, [&](auto p) { setMode(p.value); } };

    EffectFilter filter;

    EffectDelay(AudioPlugin::Props& props, char* _name)
        : Mapping(props, _name, { // clang-format off
            &timeRatio, &masterAmplitude, 
            &voices[0].amplitude, &voices[0].feedback, &voices[0].sec, 
            &voices[1].amplitude, &voices[1].feedback, &voices[1].sec, 
            &voices[2].amplitude, &voices[2].feedback, &voices[2].sec, 
            &voices[3].amplitude, &voices[3].feedback, &voices[3].sec, 
            &voices[4].amplitude, &voices[4].feedback, &voices[4].sec, 
            &voices[5].amplitude, &voices[5].feedback, &voices[5].sec, 
            &voices[6].amplitude, &voices[6].feedback, &voices[6].sec, 
            &voices[7].amplitude, &voices[7].feedback, &voices[7].sec,
            &cutoff, &resonance, &mode
        })
        // clang-format on
        , sampleRate(props.sampleRate)
        , filter(props, _name)
    {
        setVoice(0, 10.0f, 60.0f, 0.0f);
        setVoice(1, 20.0f, 50.0f, 0.0f);
        setVoice(2, 30.0f, 40.0f, 0.0f);
        setVoice(3, 40.0f, 30.0f, 0.0f);
        setVoice(4, 50.0f, 20.0f, 0.0f);

        filter.setResonance(0.95f).setMode(EffectFilter::Mode::HPF);

        // // make reverb
        // setVoice(0, 0.05f, 0.9f, 0.0f);
        // setVoice(1, 0.10f, 0.85f, 0.0f);
        // setVoice(2, 0.15f, 0.8f, 0.0f);
        // setVoice(3, 0.20f, 0.75f, 0.0f);
        // setVoice(4, 0.25f, 0.7f, 0.0f);
        // setVoice(5, 0.30f, 0.65f, 0.0f);
        // setVoice(6, 0.35f, 0.6f, 0.0f);
        // setVoice(7, 0.40f, 0.55f, 0.0f);
    }

    void sample(float* buf)
    {
        buf[track] = sample(buf[track]);
    }

    EffectDelay& setSec(uint8_t voiceIndex, float sec)
    {
        voices[voiceIndex].sec.setFloat(sec);
        voices[voiceIndex].index = (buffer.index + buffer.size - (uint64_t)(sampleRate * voices[voiceIndex].sec.pct() * timeRatio.pct())) % buffer.size;
        return *this;
    }

    EffectDelay& setAmplitude(uint8_t voiceIndex, float amplitude)
    {
        voices[voiceIndex].amplitude.setFloat(amplitude);
        return *this;
    }

    EffectDelay& setFeedback(uint8_t voiceIndex, float feedback)
    {
        voices[voiceIndex].feedback.setFloat(feedback);
        return *this;
    }

    EffectDelay& setVoice(uint8_t voiceIndex, float sec, float amplitude, float feedback)
    {
        setSec(voiceIndex, sec);
        setAmplitude(voiceIndex, amplitude);
        setFeedback(voiceIndex, feedback);
        return *this;
    }

    EffectDelay& setFeedback(float feedback)
    {
        for (uint8_t i = 0; i < MAX_DELAY_VOICES; i++) {
            setFeedback(i, feedback);
        }
        return *this;
    }

    EffectDelay& setMasterAmplitude(float amplitude)
    {
        masterAmplitude.setFloat(amplitude);
        return *this;
    }

    EffectDelay& setTimeRatio(float ratio)
    {
        timeRatio.setFloat(ratio);
        for (uint8_t i = 0; i < MAX_DELAY_VOICES; i++) {
            setSec(i, voices[i].sec.get());
        }
        return *this;
    }

    EffectDelay& setCutoff(float value)
    {
        cutoff.setFloat(value);
        filter.setCutoff(cutoff.get());
        return *this;
    }

    EffectDelay& setResonance(float value)
    {
        resonance.setFloat(value);
        filter.setResonance(resonance.get());
        return *this;
    }

    EffectDelay& setMode(float mode)
    {
        filter.setMode(mode);
        return *this;
    }
};

#endif