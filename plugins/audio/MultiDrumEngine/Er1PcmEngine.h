#pragma once

#include <math.h>
#include <sndfile.h>

#include "plugins/audio/utils/utils.h"
#include "plugins/audio/MultiDrumEngine/DrumEngine.h"
#include "plugins/audio/utils/fileBrowser.h"

class Er1PcmEngine : public DrumEngine {
    static const uint64_t bufferSize = 48000 * 3; // max 3 seconds
    float sampleData[bufferSize];

    struct SampleBuffer {
        uint64_t count = 0;
        float* data = nullptr;
    } sampleBuffer;

    float pitch = 1.0f;
    FileBrowser fileBrowser = FileBrowser("./data/audio/samples/er1");

    float modPhase = 0.0f;

    float index = 0.0f;
    float step = 1.0f;
    float stepMultiplier = 1.0;

public:
    Val& pitchVal = val(0.0f, "PITCH", { "Pitch", VALUE_CENTERED, .min = -24.0f, .max = 24.0f, .unit = "st" }, [&](auto p) {
        pitch = pow(2, p.value / 12.0f);
    });

    Val& modDepth = val(0.0f, "MOD_DEPTH", { "Mod Depth", .unit = "%" });
    Val& modSpeed = val(1.0f, "MOD_SPEED", { "Mod Speed", .min = 0.1f, .max = 20.0f, .step = 0.1f });
    Val& modType = val(0.0f, "MOD_TYPE", { "Mod Type", VALUE_STRING });

    Val& waveform = val(1.0f, "WAVEFORM", { "Waveform", VALUE_STRING, .min = 1.0f, .max = (float)fileBrowser.count }, [&](auto p) { open(p.value); });

    Er1PcmEngine(AudioPlugin::Props& props, AudioPlugin::Config& config)
        : DrumEngine(props, config, "ER-1")
    {
        open(waveform.get(), true);
    }

    void sampleOn(float* buf, float envAmp, int sampleCounter, int totalSamples) override
    {
        if (sampleBuffer.data == nullptr || sampleBuffer.count == 0)
            return;

        float mod = 1.0f;
        if (modDepth.get() > 0.0f) {
            if (modType.get() == 0.0f) {
                mod = 1.0f + sinf(modPhase) * modDepth.pct();
            } else {
                // mod = 1.0f + (randomFloat() * 2.0f - 1.0f) * modDepth.pct();
            }
            modPhase += modSpeed.get() * 0.001f;
            if (modPhase > 2.0f * M_PI)
                modPhase -= 2.0f * M_PI;
        }

        float out = 0.0f;
        if ((uint64_t)index < sampleBuffer.count) {
            out = sampleBuffer.data[(uint64_t)index] * mod * envAmp;
            // let s use linearInterpolation(float index, uint16_t lutSize, float* lut)
            // out = linearInterpolationAbsolute(index, sampleBuffer.count, sampleBuffer.data) * mod * envAmp;
            index += step * pitch;
        }

        buf[track] = out;
    }

    void sampleOff(float* buf) override
    {
        // Not used for now (no reverb/tail)
        buf[track] = 0.0f;
    }

    void noteOn(uint8_t note, float velocity, void* userdata = NULL) override
    {
        index = 0.0f;
        modPhase = 0.0f;
    }

    void open(float value, bool force = false)
    {
        waveform.setFloat(value);
        int position = waveform.get();
        if (force || position != fileBrowser.position) {
            waveform.setString(fileBrowser.getFile(position));
            std::string filepath = fileBrowser.getFilePath(position);
            logTrace("SAMPLE_SELECTOR: %f %s", value, filepath.c_str());
            open(filepath);
            initValues();
        }
    }

    void open(std::string filename)
    {
        SF_INFO sfinfo;
        SNDFILE* file = sf_open(filename.c_str(), SFM_READ, &sfinfo);
        if (!file) {
            logDebug("Error: could not open file %s [%s]\n", filename.c_str(), sf_strerror(file));
            return;
        }
        logTrace("Audio file %s sampleCount %ld sampleRate %d\n", filename, (long)sfinfo.frames, sfinfo.samplerate);
        // printf(".................Audio file chan %d vs prop chan %d\n", sfinfo.channels, props.channels);

        sampleBuffer.count = sf_read_float(file, sampleData, bufferSize);
        sampleBuffer.data = sampleData;

        sf_close(file);

        if (sfinfo.channels < props.channels) {
            stepMultiplier = 0.5f;
        } else if (sfinfo.channels > props.channels) {
            stepMultiplier = 2.0f;
        } else {
            stepMultiplier = 1.0f;
        }

        index = sampleBuffer.count;
        applyGain(sampleBuffer.data, sampleBuffer.count);
    }
};
