#ifndef _EFFECT_DELAY_H_
#define _EFFECT_DELAY_H_

#include "EffectFilter.h"
#include "audioBuffer.h"
#include "audioPlugin.h"
#include "mapping.h"

#define MAX_DELAY_VOICES 8

// TODO load/save different kind of delay and reverb from a config file
// TODO add lfo on time ratio

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

    struct DelayVoice {
        uint64_t index;
        Val amplitude;
        Val feedback;
        Val sec;
    } voices[MAX_DELAY_VOICES] = {
        { 0,
            { 0.0, "AMPLITUDE_0", [&](float value) { setAmplitude0(value); } },
            { 0.0, "FEEDBACK_0",  [&](float value) {  setFeedback0(value); } },
            { 10.0, "SEC_0", [&](float value) { setSec0(value); } } }, // FIXME this is not seconds...
        { 1,
            { 0.0, "AMPLITUDE_1", [&](float value) { setAmplitude1(value); } },
            { 0.0, "FEEDBACK_1", [&](float value) { setFeedback1(value); } },
            { 10.0, "SEC_1", [&](float value) { setSec1(value); } } },
        { 2,
            { 0.0, "AMPLITUDE_2", [&](float value) { setAmplitude2(value); } },
            { 0.0, "FEEDBACK_2", [&](float value) { setFeedback2(value); } },
            { 10.0, "SEC_2", [&](float value) { setSec2(value); } } },
        { 3,
            { 0.0, "AMPLITUDE_3", [&](float value) { setAmplitude3(value); } },
            { 0.0, "FEEDBACK_3", [&](float value) { setFeedback3(value); } },
            { 10.0, "SEC_3", [&](float value) { setSec3(value); } } },
        { 4,
            { 0.0, "AMPLITUDE_4", [&](float value) { setAmplitude4(value); } },
            { 0.0, "FEEDBACK_4", [&](float value) { setFeedback4(value); } },
            { 10.0, "SEC_4", [&](float value) { setSec4(value); } } },
        { 5,
            { 0.0, "AMPLITUDE_5", [&](float value) { setAmplitude5(value); } },
            { 0.0, "FEEDBACK_5", [&](float value) { setFeedback5(value); } },
            { 10.0, "SEC_5", [&](float value) { setSec5(value); } } },
        { 6,
            { 0.0, "AMPLITUDE_6", [&](float value) { setAmplitude6(value); } },
            { 0.0, "FEEDBACK_6", [&](float value) { setFeedback6(value); } },
            { 10.0, "SEC_6", [&](float value) { setSec6(value); } } },
        { 7,
            { 0.0, "AMPLITUDE_7", [&](float value) { setAmplitude7(value); } },
            { 0.0, "FEEDBACK_7", [&](float value) { setFeedback7(value); } },
            { 10.0, "SEC_7", [&](float value) { setSec7(value); } } },
    };

public:
    // From 0.0 to 1.0 to apply time ratio to voice in seconds
    Val timeRatio = { 100.0f, "TIME_RATIO", [&](float value) { setTimeRatio(value); }, { "Time Ratio", .unit = "%" } };
    Val masterAmplitude = { 0.0f, "MASTER_AMPLITUDE", [&](float value) { setMasterAmplitude(value); }, { "Master Amplitude", .unit = "%" } };

    // should we inhirate filter so there is no need to defined this...
    Val cutoff = { 0.0f, "CUTOFF", [&](float value) { setCutoff(value); }, { "Cutoff" } };
    Val resonance = { 0.0f, "RESONANCE", [&](float value) { setResonance(value); }, { "Resonance", .unit = "%" } };
    Val mode = { 0.0f, "MODE", [&](float value) { setMode(value); } };

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

    EffectDelay& setSec0(float sec) { return setSec(0, sec); }
    EffectDelay& setSec1(float sec) { return setSec(1, sec); }
    EffectDelay& setSec2(float sec) { return setSec(2, sec); }
    EffectDelay& setSec3(float sec) { return setSec(3, sec); }
    EffectDelay& setSec4(float sec) { return setSec(4, sec); }
    EffectDelay& setSec5(float sec) { return setSec(5, sec); }
    EffectDelay& setSec6(float sec) { return setSec(6, sec); }
    EffectDelay& setSec7(float sec) { return setSec(7, sec); }

    EffectDelay& setSec(uint8_t voiceIndex, float sec)
    {
        voices[voiceIndex].sec.setFloat(sec);
        voices[voiceIndex].index = (buffer.index + buffer.size - (uint64_t)(sampleRate * voices[voiceIndex].sec.pct() * timeRatio.pct())) % buffer.size;
        return *this;
    }

    EffectDelay& setAmplitude0(float amplitude) { return setAmplitude(0, amplitude); }
    EffectDelay& setAmplitude1(float amplitude) { return setAmplitude(1, amplitude); }
    EffectDelay& setAmplitude2(float amplitude) { return setAmplitude(2, amplitude); }
    EffectDelay& setAmplitude3(float amplitude) { return setAmplitude(3, amplitude); }
    EffectDelay& setAmplitude4(float amplitude) { return setAmplitude(4, amplitude); }
    EffectDelay& setAmplitude5(float amplitude) { return setAmplitude(5, amplitude); }
    EffectDelay& setAmplitude6(float amplitude) { return setAmplitude(6, amplitude); }
    EffectDelay& setAmplitude7(float amplitude) { return setAmplitude(7, amplitude); }

    EffectDelay& setAmplitude(uint8_t voiceIndex, float amplitude)
    {
        voices[voiceIndex].amplitude.setFloat(amplitude);
        return *this;
    }

    EffectDelay& setFeedback0(float feedback) { return setFeedback(0, feedback); }
    EffectDelay& setFeedback1(float feedback) { return setFeedback(1, feedback); }
    EffectDelay& setFeedback2(float feedback) { return setFeedback(2, feedback); }
    EffectDelay& setFeedback3(float feedback) { return setFeedback(3, feedback); }
    EffectDelay& setFeedback4(float feedback) { return setFeedback(4, feedback); }
    EffectDelay& setFeedback5(float feedback) { return setFeedback(5, feedback); }
    EffectDelay& setFeedback6(float feedback) { return setFeedback(6, feedback); }
    EffectDelay& setFeedback7(float feedback) { return setFeedback(7, feedback); }

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