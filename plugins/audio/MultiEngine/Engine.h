#pragma once

#include "helpers/midiNote.h"
#include "plugins/audio/audioPlugin.h"
#include "plugins/audio/mapping.h"
#include "plugins/audio/utils/AsrEnvelop.h"
#include "plugins/audio/utils/EnvelopDrumAmp.h"

class Engine : public Mapping {
protected:
    float baseFreq = 220.0f;
    uint8_t baseFreqNote = 60;
    void setBaseFreq(int8_t pitchSemitones, uint8_t note = 0)
    {
        if (note == 0)
            note = baseFreqNote;

        baseFreqNote = note;
        baseFreq = getMidiNoteFrequency(baseFreqNote + pitchSemitones);
    }

public:
    float attackStep = 0.0f;
    float releaseStep = 0.0f;
    AsrEnvelop envelopAmp = AsrEnvelop(&attackStep, &releaseStep);

    std::string name = "Engine";


    Val& attack = val(50.0f, "ATTACK", { "Attack", .min = 10.0, .max = 3000.0, .step = 10.0, .unit = "ms" }, [&](auto p) {
        p.val.setFloat(p.value);
        attackStep = 1.0f / (p.val.get() * 0.001f * props.sampleRate);
    });

    Val& release = val(300.0f, "RELEASE", { "Release", .min = 10.0, .max = 3000.0, .step = 10.0, .unit = "ms" }, [&](auto p) {
        p.val.setFloat(p.value);
        releaseStep = 1.0f / (p.val.get() * 0.001f * props.sampleRate);
    });

    Engine(AudioPlugin::Props& props, AudioPlugin::Config& config, std::string name)
        : Mapping(props, config)
        , name(name)
    {
    }

    void sample(float* buf) override
    {
        float envAmp = envelopAmp.next();
        sample(buf, envAmp);
    }

    uint8_t playingNote = 0;
    void noteOn(uint8_t note, float _velocity, void* userdata = NULL) override
    {
        playingNote = note;
        envelopAmp.attack();
    }

    void noteOff(uint8_t note, float _velocity, void* userdata = NULL) override
    {
        if (note == playingNote) {
            envelopAmp.release();
        }
    }

    virtual void sample(float* buf, float envAmp) = 0;

    virtual void serializeJson(nlohmann::json& json) { }
    virtual void hydrateJson(nlohmann::json& json) { }
};
