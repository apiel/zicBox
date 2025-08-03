#pragma once

#include <math.h>
#include <sndfile.h>

#include "plugins/audio/MultiDrumEngine/DrumEngine.h"
#include "plugins/audio/utils/fileBrowser.h"
#include "plugins/audio/utils/utils.h"

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
    float modDepthAmount = 0.0f;

    bool initialized = false;

    std::function<float(int, int)> getPitchMod;

public:
    Val& pitchVal = val(0.0f, "PITCH", { "Pitch", VALUE_CENTERED, .min = -24.0f, .max = 24.0f, .unit = "st" }, [&](auto p) {
        pitch = pow(2, p.value / 12.0f);
    });

    Val& modDepth = val(0.0f, "MOD_DEPTH", { "Mod Depth", VALUE_CENTERED, .min = -100.0f, .unit = "%" }, [&](auto p) {
        p.val.setFloat(p.value);
        modDepthAmount = p.val.pct() * 2.0f - 1.0f;
    });
    Val& modSpeed = val(1.0f, "MOD_SPEED", { "Mod Speed", .min = 10.0f, .max = 5000.0f, .step = 10.0f, .unit = "Hz" });
    Val& modType = val(0.0f, "MOD_TYPE", { "Mod Type", VALUE_STRING, .min = 0.0f, .max = 6.0f, .skipJumpIncrements = true }, [&](auto p) {
        p.val.setFloat(p.value);
        if (p.val.get() == 0.0f) {
            p.val.setString("Sine");
            getPitchMod = [&](int, int) { return 1.0f + sinf(modPhase) * modDepthAmount; };
        } else if (p.val.get() == 1.0f) {
            p.val.setString("Saw Down"); //  The pitch will fall cyclically.
            getPitchMod = [&](int, int) {
                float phase = fmodf(modPhase / (2.0f * M_PI), 1.0f); // 0..1
                return 1.0f + (1.0f - phase * 2.0f) * modDepthAmount; // 1..-1
            };
        } else if (p.val.get() == 2.0f) {
            p.val.setString("Square"); // Two pitches will alternate cyclically.
            getPitchMod = [&](int, int) {
                float sq = sinf(modPhase) >= 0.0f ? 1.0f : -1.0f;
                return 1.0f + sq * modDepthAmount;
            };
        } else if (p.val.get() == 3.0f) {
            p.val.setString("Triangle"); // The pitch will rise and fall cyclically.
            getPitchMod = [&](int, int) {
                float phase = fmodf(modPhase / (2.0f * M_PI), 1.0f);
                float tri = phase < 0.5f ? (phase * 4.0f - 1.0f) : (3.0f - phase * 4.0f);
                return 1.0f + tri * modDepthAmount; // tri goes -1 to 1
            };
        } else if (p.val.get() == 4.0f) {
            p.val.setString("Sample & Hold"); // The pitch will change randomly.
            int samplesPerHold = std::max(1, (int)(props.sampleRate / modSpeed.get()));
            int lastHoldSample = -1;
            float heldValue = props.lookupTable->getNoise();

            getPitchMod = [&, samplesPerHold, lastHoldSample, heldValue](int sampleCounter, int totalSamples) mutable {
                if (sampleCounter / samplesPerHold != lastHoldSample) {
                    heldValue = props.lookupTable->getNoise();
                    lastHoldSample = sampleCounter / samplesPerHold;
                }
                return 1.0f + heldValue * modDepthAmount;
            };
        } else if (p.val.get() == 5.0f) {
            p.val.setString("Noise"); // A noise component will be cyclically added to the pitch. This is effective when creating snare drum sounds.
            getPitchMod = [&](int, int) {
                return 1.0f + props.lookupTable->getNoise() * modDepthAmount;
            };
        } else if (p.val.get() == 6.0f) {
            p.val.setString("Envelope"); // An envelope will be applied to the pitch. This is effective when creating kick or tom sounds.
            getPitchMod = [&](int sampleCounter, int totalSamples) {
                if (totalSamples <= 0) {
                    return 1.0f;
                }
                float envPos = (float)sampleCounter / (float)totalSamples;
                envPos = envPos > 1.0f ? 1.0f : envPos;
                float decay = 1.0f - envPos; // linear decay
                return 1.0f + decay * modDepthAmount;
            };
        }
    });

    Val& waveform = val(1.0f, "WAVEFORM", { "Waveform", VALUE_STRING, .min = 1.0f, .max = (float)fileBrowser.count }, [&](auto p) {
        // logDebug("Waveform: %f", p.value);
        // open(p.value, !initialized);
        // initialized = true;
        open(p.value);
    });

    Er1PcmEngine(AudioPlugin::Props& props, AudioPlugin::Config& config)
        : DrumEngine(props, config, "ER-1")
    {
        // open(waveform.get(), true);
        initValues();
    }

    void sampleOn(float* buf, float envAmp, int sampleCounter, int totalSamples) override
    {
        if (sampleBuffer.data == nullptr || sampleBuffer.count == 0)
            return;

        float mod = 1.0f;
        if (modDepth.get() != 0.0f) {
            mod = getPitchMod(sampleCounter, totalSamples);
            modPhase += modSpeed.pct() * 0.001f;
            if (modPhase > 2.0f * M_PI) {
                modPhase -= 2.0f * M_PI;
            }
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
