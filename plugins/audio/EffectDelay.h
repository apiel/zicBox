/** Description:
This code defines a sophisticated audio processor called `EffectDelay`, which is used to generate time-based effects, primarily delays and reverb.

**Core Mechanism:**
The system works by continually storing the incoming sound in a temporary memory bank, like a short audio loop. To create an echo, it reads back a portion of the sound that was recorded moments ago and mixes it with the current sound. The key element that determines the sound is the "feedback," where the delayed sound is itself re-recorded back into the memory bank, creating continuous, decaying echoes.

**Multi-Voice Architecture:**
This effect is highly flexible because it operates using up to eight independent "Delay Voices." Each voice is essentially a separate echo line. This allows the user to build complex soundscapes where different echoes occur at different times, volumes, and decay rates, making it suitable for both simple echoes and rich, realistic reverb.

**User Controls:**
Users have detailed control over how the effect sounds:

*   **Master Controls:** You can set the overall mix level (`MASTER_AMPLITUDE`) and uniformly stretch or compress all delay times (`TIME_RATIO`).
*   **Per-Voice Editing:** By selecting a voice via the `VOICE_EDIT` control, you can fine-tune its individual parameters:
    *   **Amplitude:** How loud that specific echo is.
    *   **Time:** How long after the original sound the echo appears.
    *   **Feedback:** How many times that specific echo repeats.
*   **Integrated Filter:** The delayed signal passes through an adjustable filter (`EffectFilter`). This lets the user sculpt the tone of the echoes, such as making them sound muffled or distant.

sha: 16d3da3e6809a835a98e33ec8f489245e89e274ee0e07ec9f4f3a4822511eda9 
*/
#pragma once

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

        return in + filter.process(delay);
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
            /*md   - `TIME_0` to set time ratio for voice 0. */
            /*md   - `TIME_1` to set time ratio for voice 1. */
            /*md   - ... */
            { 0.0, "AMPLITUDE_0" },
            { 0.0, "FEEDBACK_0" },
            { 10.0, "TIME_0", {}, [&](auto p) { setSec(&voices[0], p.value); } } }, // FIXME this is not seconds...
        { 1,
            { 0.0, "AMPLITUDE_1" },
            { 0.0, "FEEDBACK_1" },
            { 10.0, "TIME_1", {}, [&](auto p) { setSec(&voices[1], p.value); } } },
        { 2,
            { 0.0, "AMPLITUDE_2" },
            { 0.0, "FEEDBACK_2" },
            { 10.0, "TIME_2", {}, [&](auto p) { setSec(&voices[2], p.value); } } },
        { 3,
            { 0.0, "AMPLITUDE_3" },
            { 0.0, "FEEDBACK_3" },
            { 10.0, "TIME_3", {}, [&](auto p) { setSec(&voices[3], p.value); } } },
        { 4,
            { 0.0, "AMPLITUDE_4" },
            { 0.0, "FEEDBACK_4" },
            { 10.0, "TIME_4", {}, [&](auto p) { setSec(&voices[4], p.value); } } },
        { 5,
            { 0.0, "AMPLITUDE_5" },
            { 0.0, "FEEDBACK_5" },
            { 10.0, "TIME_5", {}, [&](auto p) { setSec(&voices[5], p.value); } } },
        { 6,
            { 0.0, "AMPLITUDE_6" },
            { 0.0, "FEEDBACK_6" },
            { 10.0, "TIME_6", {}, [&](auto p) { setSec(&voices[6], p.value); } } },
        { 7,
            { 0.0, "AMPLITUDE_7" },
            { 0.0, "FEEDBACK_7" },
            { 10.0, "TIME_7", {}, [&](auto p) { setSec(&voices[7], p.value); } } },
    };

    DelayVoice* selectedVoice = nullptr;

    void initVoice(uint8_t voiceIndex, float sec, float amplitude, float feedback)
    {
        setSec(&voices[voiceIndex], sec);
        voices[voiceIndex].amplitude.setFloat(amplitude);
        voices[voiceIndex].feedback.setFloat(feedback);
    }

public:
    EffectFilter filter;

    /*md - `TIME_RATIO` to modulate time ratio for all voices.*/
    Val timeRatio = { 100.0f, "TIME_RATIO", { "Time Ratio", .unit = "%" }, [&](auto p) { setTimeRatio(p.value); } };
    /*md - `MASTER_AMPLITUDE` to set master amplitude.*/
    Val masterAmplitude = { 0.0f, "MASTER_AMPLITUDE", { "Master Amplitude", .unit = "%" } };

    /*md - `VOICE_EDIT` select the step to edit */
    Val& voiceEdit = val(0.0f, "VOICE_EDIT", { "Voice edit", .min = 1, .max = MAX_DELAY_VOICES }, [&](auto p) { setVoiceEdit(p.value); });
    /*md - `VOICE_AMPLITUDE` to set amplitude on selected voice.*/
    Val& voiceAmplitude = val(0.0f, "VOICE_AMPLITUDE", { "Voice amp.", .unit = "%" }, [&](auto p) { p.val.setFloat(p.value); selectedVoice->amplitude.set(p.value); });
    /*md - `VOICE_FEEDBACK` to set feedback on selected voice.*/
    Val& voiceFeedback = val(0.0f, "VOICE_FEEDBACK", { "Voice feedback", .unit = "%" }, [&](auto p) { p.val.setFloat(p.value); selectedVoice->feedback.set(p.value); });
    /*md - `VOICE_SEC` to set sec on selected voice.*/
    Val& voiceSec = val(0.0f, "VOICE_TIME", { "Voice time", .min = 0.0f, .max = 1000.0f, .step = 5.0f, .unit = "ms" }, [&](auto p) { p.val.setFloat(p.value); setSec(selectedVoice, p.value); });

    EffectDelay(AudioPlugin::Props& props, AudioPlugin::Config& config)
        : Mapping(props, config, {
                                    // clang-format off
            &timeRatio, &masterAmplitude, 
            &voices[0].amplitude, &voices[0].feedback, &voices[0].sec, 
            &voices[1].amplitude, &voices[1].feedback, &voices[1].sec, 
            &voices[2].amplitude, &voices[2].feedback, &voices[2].sec, 
            &voices[3].amplitude, &voices[3].feedback, &voices[3].sec, 
            &voices[4].amplitude, &voices[4].feedback, &voices[4].sec, 
            &voices[5].amplitude, &voices[5].feedback, &voices[5].sec, 
            &voices[6].amplitude, &voices[6].feedback, &voices[6].sec, 
            &voices[7].amplitude, &voices[7].feedback, &voices[7].sec,
        })
        // clang-format on
        , sampleRate(props.sampleRate)
        , filter(props, config)
    {
        /*md - `CUTOFF` to set cutoff on delay buffer.*/
        val(&filter.filterCutoff);
        /*md - `RESONANCE` to set resonance on delay buffer.*/
        val(&filter.filterResonance);
        /*md - `FILTER_TYPE` to set filter mode.*/
        val(&filter.filterType);

        initValues();
        initVoice(0, 100.0f, 60.0f, 0.0f);
        initVoice(1, 200.0f, 50.0f, 0.0f);
        initVoice(2, 300.0f, 40.0f, 0.0f);
        initVoice(3, 400.0f, 30.0f, 0.0f);
        initVoice(4, 500.0f, 20.0f, 0.0f);
        setVoiceEdit(voiceEdit.get());

        filter.filterResonance.set(0.95f);
        filter.filterType.set(EffectFilter::FilterType::LP);

        // // make reverb
        // initVoice(0, 0.05f, 0.9f, 0.0f);
        // initVoice(1, 0.10f, 0.85f, 0.0f);
        // initVoice(2, 0.15f, 0.8f, 0.0f);
        // initVoice(3, 0.20f, 0.75f, 0.0f);
        // initVoice(4, 0.25f, 0.7f, 0.0f);
        // initVoice(5, 0.30f, 0.65f, 0.0f);
        // initVoice(6, 0.35f, 0.6f, 0.0f);
        // initVoice(7, 0.40f, 0.55f, 0.0f);
    }

    void sample(float* buf)
    {
        buf[track] = sample(buf[track]);
    }

    void setVoiceEdit(float value)
    {
        voiceEdit.setFloat(value);
        uint8_t index = voiceEdit.get() - 1;
        selectedVoice = &voices[index];
        printf("selected voice: %d, amplitude: %f, feedback: %f sec: %f\n", index, selectedVoice->amplitude.get(), selectedVoice->feedback.get(), selectedVoice->sec.get());
        voiceAmplitude.set(selectedVoice->amplitude.get());
        voiceFeedback.set(selectedVoice->feedback.get());
        voiceSec.set(selectedVoice->sec.get());
        printf("amp: %f, fb: %f sec: %f\n", voiceAmplitude.get(), voiceFeedback.get(), voiceSec.get());
    }

    void setSec(DelayVoice* voice, float sec)
    {
        voice->sec.setFloat(sec);
        // percentage of 1 seconds => 0ms to 1000ms, in percentage is same as 1 seconds
        voice->index = (buffer.index + buffer.size - (uint64_t)(sampleRate * voice->sec.pct() * timeRatio.pct())) % buffer.size;
    }

    void setTimeRatio(float ratio)
    {
        timeRatio.setFloat(ratio);
        for (uint8_t i = 0; i < MAX_DELAY_VOICES; i++) {
            setSec(&voices[i], voices[i].sec.get());
        }
    }
};
