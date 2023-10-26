#ifndef _EFFECT_DELAY_H_
#define _EFFECT_DELAY_H_

#include "EffectFilter.h"
#include "audioBuffer.h"
#include "audioPlugin.h"
#include "mapping.h"

#define MAX_DELAY_VOICES 8

// TODO load/save different kind of delay and reverb from a config file
// TODO add lfo on time ratio

class EffectDelay : public Mapping<EffectDelay> {
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
        Val<EffectDelay> amplitude;
        Val<EffectDelay> feedback;
        Val<EffectDelay> sec;
    } voices[MAX_DELAY_VOICES] = {
        { 0,
            { this, 0.0, "AMPLITUDE_0", &EffectDelay::setAmplitude0 },
            { this, 0.0, "FEEDBACK_0", &EffectDelay::setFeedback0 },
            { this, 10.0, "SEC_0", &EffectDelay::setSec0 } }, // FIXME this is not seconds...
        { 1,
            { this, 0.0, "AMPLITUDE_1", &EffectDelay::setAmplitude1 },
            { this, 0.0, "FEEDBACK_1", &EffectDelay::setFeedback1 },
            { this, 10.0, "SEC_1", &EffectDelay::setSec1 } },
        { 2,
            { this, 0.0, "AMPLITUDE_2", &EffectDelay::setAmplitude2 },
            { this, 0.0, "FEEDBACK_2", &EffectDelay::setFeedback2 },
            { this, 10.0, "SEC_2", &EffectDelay::setSec2 } },
        { 3,
            { this, 0.0, "AMPLITUDE_3", &EffectDelay::setAmplitude3 },
            { this, 0.0, "FEEDBACK_3", &EffectDelay::setFeedback3 },
            { this, 10.0, "SEC_3", &EffectDelay::setSec3 } },
        { 4,
            { this, 0.0, "AMPLITUDE_4", &EffectDelay::setAmplitude4 },
            { this, 0.0, "FEEDBACK_4", &EffectDelay::setFeedback4 },
            { this, 10.0, "SEC_4", &EffectDelay::setSec4 } },
        { 5,
            { this, 0.0, "AMPLITUDE_5", &EffectDelay::setAmplitude5 },
            { this, 0.0, "FEEDBACK_5", &EffectDelay::setFeedback5 },
            { this, 10.0, "SEC_5", &EffectDelay::setSec5 } },
        { 6,
            { this, 0.0, "AMPLITUDE_6", &EffectDelay::setAmplitude6 },
            { this, 0.0, "FEEDBACK_6", &EffectDelay::setFeedback6 },
            { this, 10.0, "SEC_6", &EffectDelay::setSec6 } },
        { 7,
            { this, 0.0, "AMPLITUDE_7", &EffectDelay::setAmplitude7 },
            { this, 0.0, "FEEDBACK_7", &EffectDelay::setFeedback7 },
            { this, 10.0, "SEC_7", &EffectDelay::setSec7 } },
    };

public:
    // From 0.0 to 1.0 to apply time ratio to voice in seconds
    Val<EffectDelay> timeRatio = { this, 100.0f, "TIME_RATIO", &EffectDelay::setTimeRatio, { "Time Ratio", .unit = "%" } };
    Val<EffectDelay> masterAmplitude = { this, 0.0f, "MASTER_AMPLITUDE", &EffectDelay::setMasterAmplitude, { "Master Amplitude", .unit = "%" } };

    // should we inhirate filter so there is no need to defined this...
    Val<EffectDelay> cutoff = { this, 0.0f, "CUTOFF", &EffectDelay::setCutoff, { "Cutoff" } };
    Val<EffectDelay> resonance = { this, 0.0f, "RESONANCE", &EffectDelay::setResonance, { "Resonance", .unit = "%" } };
    Val<EffectDelay> mode = { this, 0.0f, "MODE", &EffectDelay::setMode };

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
            &cutoff, &resonance, &mode})
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