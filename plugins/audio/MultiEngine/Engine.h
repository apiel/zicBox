#pragma once

#include "helpers/midiNote.h"
#include "plugins/audio/audioPlugin.h"
#include "plugins/audio/mapping.h"

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
    std::string name = "Engine";

    Engine(AudioPlugin::Props& props, AudioPlugin::Config& config, std::string name)
        : Mapping(props, config)
        , name(name)
    {
    }

    void sample(float* buf) override
    {
    }

    virtual void sample(float* buf, float envAmp) = 0;

    virtual void serializeJson(nlohmann::json& json) { }
    virtual void hydrateJson(nlohmann::json& json) { }
};
