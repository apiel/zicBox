#pragma once

#include <math.h>
#include <sndfile.h>

#include "host/constants.h"
#include "plugins/audio/MultiDrumEngine/DrumEngine.h"
#include "plugins/audio/utils/MultiFx.h"
#include "plugins/audio/utils/TransientGenerator.h"
#include "plugins/audio/utils/fileBrowser.h"
#include "plugins/audio/utils/utils.h"

class Er1PcmEngine : public DrumEngine {
    float velocity = 1.0f;

    static const uint64_t bufferSize = 48000 * 3; // max 3 seconds
    float sampleData[bufferSize];

    struct SampleBuffer {
        uint64_t count = 0;
        float* data = nullptr;
    } sampleBuffer;

    float pitch = 1.0f;
    FileBrowser fileBrowser = FileBrowser(AUDIO_FOLDER + "/er1");

    MultiFx multiFx;
    MultiFx multiFx2;
    TransientGenerator transient;

    float modPhase = 0.0f;

    float index = 0.0f;
    float step = 1.0f;
    float stepMultiplier = 1.0;
    float modDepthAmount = 0.0f;

    bool initialized = false;

    std::function<float(int, int)> getPitchMod;
    int samplesPerHold = 0;
    int lastHoldSample = -1;
    float heldValue = 0.0f;
    void setSamplePerHold()
    {
        samplesPerHold = std::max(1, (int)(props.sampleRate / modSpeed.get() * 10.0f));
    }

public:
    Val& pitchVal = val(0.0f, "PITCH", { "Pitch", VALUE_CENTERED, .min = -24.0f, .max = 24.0f, .unit = "st" }, [&](auto p) {
        pitch = pow(2, p.value / 12.0f);
    });

    GraphPointFn waveGraph = [&](float index) { int idx = index * sampleBuffer.count; return sampleBuffer.data[idx]; };
    Val& waveform = val(1.0f, "WAVEFORM", { "Waveform", VALUE_STRING, .min = 1.0f, .max = (float)fileBrowser.count, .graph = waveGraph }, [&](auto p) {
        open(p.value);
    });

    Val& modDepth = val(0.0f, "MOD_DEPTH", { "Mod Depth", VALUE_CENTERED, .min = -100.0f, .unit = "%" }, [&](auto p) {
        p.val.setFloat(p.value);
        modDepthAmount = p.val.pct() * 2.0f - 1.0f;
    });
    Val& modSpeed = val(10.0f, "MOD_SPEED", { "Mod Speed", .min = 10.0f, .max = 5000.0f, .step = 10.0f, .unit = "Hz" }, [&](auto p) {
        p.val.setFloat(p.value);
        setSamplePerHold();
    });
    Val& modType = val(0.0f, "MOD_TYPE", { "Mod Type", VALUE_STRING, .min = 0.0f, .max = 8.0f, .incType = INC_ONE_BY_ONE }, [&](auto p) {
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
            heldValue = props.lookupTable->getNoise();
            setSamplePerHold();
            getPitchMod = [&](int sampleCounter, int totalSamples) {
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
            p.val.setString("Env. Expo."); // An envelope will be applied to the pitch. This is effective when creating kick or tom sounds.
            getPitchMod = [&](int sampleCounter, int totalSamples) {
                float morph = modSpeed.pct(); // 0 = fast decay, 1 = slow decay
                float amount = 1.0f - morph;
                // Curve: higher `amount` = faster decay
                float a = 10.0f + 100.0f * amount * amount * amount * amount;
                float x = (float)sampleCounter / totalSamples; // time in seconds
                float decay = expf(-a * x); // fast drop, then flatten out
                return 1.0f + decay * modDepthAmount;
            };
        } else if (p.val.get() == 7.0f) {
            p.val.setString("Env. Bounce");
            getPitchMod = [&](int sampleCounter, int totalSamples) {
                if (totalSamples <= 0)
                    return 1.0f;

                float x = (float)sampleCounter / totalSamples;
                float morph = modSpeed.pct(); // 0 = bouncier, 1 = flatter
                float freq = 2.0f + 10.0f * (1.0f - morph); // bounce rate
                float decayRate = 4.0f + 8.0f * morph;

                float bounce = fabs(sinf(3.1415f * x * freq)) * expf(-decayRate * x);
                return 1.0f + bounce * modDepthAmount;
            };
        } else if (p.val.get() == 8.0f) {
            p.val.setString("Env. Asym"); // Slow rise, fast drop. Great for reverse or tension FX.

            getPitchMod = [&](int sampleCounter, int totalSamples) {
                if (totalSamples <= 0)
                    return 1.0f;

                float x = (float)sampleCounter / totalSamples;
                float morph = modSpeed.pct(); // 0 = more extreme, 1 = more subtle

                // You can adjust curvature with morph
                float riseCurve = 2.0f + 4.0f * (1.0f - morph); // 2 to 6
                float dropCurve = 10.0f + 50.0f * (1.0f - morph); // 10 to 60

                float value;
                if (x < 0.5f) {
                    // Logarithmic-style rise (gentle up)
                    float t = x * 2.0f;
                    value = powf(t, 1.0f / riseCurve); // slow rise
                } else {
                    // Fast exponential drop
                    float t = (x - 0.5f) * 2.0f;
                    value = 1.0f - expf(-dropCurve * t);
                }

                return 1.0f + value * modDepthAmount;
            };
        }
    });

    GraphPointFn transientGraph = [&](float index) { return *transient.sample(&index); };
    Val& transientMorph = val(100.0, "TRANSIENT", { "Transient", VALUE_STRING, .step = 0.1f, .floatingPoint = 1, .graph = transientGraph }, [&](auto p) {
        p.val.setFloat(p.value);
        transient.morphType(p.val.pct());
        p.val.setString(std::to_string((int)(transient.getMorph() * 100)) + "%");
        p.val.props().unit = transient.getTypeName();
    });

    Val& fxType = val(0, "FX_TYPE", { "FX type", VALUE_STRING, .max = MultiFx::FXType::FX_COUNT - 1 }, multiFx.setFxType);

    Val& fxAmount = val(0, "FX_AMOUNT", { "FX edit", .unit = "%" });

    Val& fxType2 = val(0, "FX2_TYPE", { "FX2 type", VALUE_STRING, .max = MultiFx::FXType::FX_COUNT - 1 }, multiFx2.setFxType);

    Val& fxAmount2 = val(0, "FX2_AMOUNT", { "FX2 edit", .unit = "%" });

    Er1PcmEngine(AudioPlugin::Props& props, AudioPlugin::Config& config)
        : DrumEngine(props, config, "ER-1")
        , multiFx(props.sampleRate, props.lookupTable)
        , multiFx2(props.sampleRate, props.lookupTable)
        , transient(props.sampleRate, 50)
    {
        // open(waveform.get(), true);
        initValues();
    }

    float transientIndex = 0.0f;

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

        float t = (float)sampleCounter / totalSamples; // Time normalized to [0, 1]
        if (t < 0.01f && transientMorph.get() > 0.0f) {
            out = out + transient.next(&transientIndex) * envAmp;
        }

        out = out * velocity;
        out = multiFx.apply(out, fxAmount.pct());
        out = multiFx2.apply(out, fxAmount2.pct());
        buf[track] = out;
    }

    void sampleOff(float* buf) override
    {
        float out = buf[track];
        out = multiFx.apply(out, fxAmount.pct());
        out = multiFx2.apply(out, fxAmount2.pct());
        buf[track] = out;
    }

    void noteOn(uint8_t note, float _velocity, void* userdata = NULL) override
    {
        DrumEngine::noteOn(note, _velocity);
        velocity = _velocity;
        index = 0.0f;
        modPhase = 0.0f;
        transientIndex = 0.0f;
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

    void serializeJson(nlohmann::json& json) override
    {
        json["sampleFile"] = fileBrowser.getFile(waveform.get());
    }

    void hydrateJson(nlohmann::json& json) override
    {
        if (json.contains("sampleFile")) {
            int position = fileBrowser.find(json["sampleFile"]);
            if (position != 0) {
                // waveform.set(position);
                open(position, true);
            }
        }
    }
};
