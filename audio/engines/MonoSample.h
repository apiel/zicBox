#pragma once

#ifndef AUDIO_FOLDER
#include "host/constants.h"
#endif

#include "audio/Grains.h"
#include "audio/effects/applyDrive.h"
#include "audio/engines/EngineBase.h"
#include "audio/filterSVF.h"
#include "audio/utils/applySampleGain.h"
#include "audio/utils/math.h"
#include "helpers/clamp.h"

#include <algorithm>
#include <cmath>
#include <dirent.h>
#include <sndfile.h>
#include <string>
#include <vector>
#include <cstring>

class MonoSample : public EngineBase<MonoSample> {
public:
    // Mono-specific constraints
    static constexpr int DELAY_BUF_SIZE = 48000;
    static constexpr int REVERB_BUF_SIZE = 16384;

    struct SampleData {
        float* data = nullptr;
        uint64_t frameCount = 0;
        bool loaded = false;

        void free()
        {
            delete[] data;
            data = nullptr;
            frameCount = 0;
            loaded = false;
        }
    };

    struct Voice {
        bool active = false;
        double pos = 0.0;
        double rate = 1.0;
        float velocity = 1.0f;
        uint8_t midiNote = 0;

        bool looping = false;
        bool inLoop = false;
        bool releasing = false;
        uint64_t loopStart = 0;
        uint64_t loopEnd = 0;

        bool granular = false;
        Grains* grains = nullptr;
    };

protected:
    const float sampleRate;
    SampleData currentSample;
    Voice voice;
    Grains* grainsInstance = nullptr;
    FilterSVF svfFilter;

    // Buffers provided by host
    float* delayBuf = nullptr;
    float* reverbBuf = nullptr;
    int delayWrite = 0;
    float dlyFbSmooth = 0.0f;

    std::vector<std::string> sampleFiles;

    // Reverb internals
    static constexpr int COMB_LEN[4] = { 1559, 1617, 1685, 1751 };
    static constexpr int AP_LEN[3] = { 347, 113, 37 };
    int combOff[4], apOff[3], combIdx[4], apIdx[3];
    float combFb[4] = { 0 };

    void loadSingleSample(const std::string& filename)
    {
        currentSample.free();
        std::string path = std::string(AUDIO_FOLDER) + "/samples/" + filename;

        SF_INFO info = {};
        SNDFILE* sf = sf_open(path.c_str(), SFM_READ, &info);
        if (!sf) return;

        uint64_t frames = (uint64_t)info.frames;
        std::vector<float> tmp(frames * info.channels);
        sf_count_t read = sf_readf_float(sf, tmp.data(), frames);
        sf_close(sf);

        if (read <= 0) return;

        currentSample.data = new float[frames];
        currentSample.frameCount = frames;

        if (info.channels == 1) {
            std::memcpy(currentSample.data, tmp.data(), frames * sizeof(float));
        } else {
            float inv = 1.0f / (float)info.channels;
            for (uint64_t f = 0; f < frames; ++f) {
                float sum = 0;
                for (int c = 0; c < info.channels; ++c)
                    sum += tmp[f * info.channels + c];
                currentSample.data[f] = sum * inv;
            }
        }
        applySampleGain(currentSample.data, frames);
        currentSample.loaded = true;
    }

    void scanSamples()
    {
        sampleFiles.clear();
        std::string path = std::string(AUDIO_FOLDER) + "/samples";
        DIR* dir = opendir(path.c_str());
        if (!dir) return;
        struct dirent* entry;
        while ((entry = readdir(dir)) != nullptr) {
            std::string name = entry->d_name;
            if (name.find(".wav") != std::string::npos || name.find(".aif") != std::string::npos)
                sampleFiles.push_back(name);
        }
        closedir(dir);
        std::sort(sampleFiles.begin(), sampleFiles.end());
        sampleSelect.max = std::max(0.0f, (float)((int)sampleFiles.size() - 1));
    }

    float slotRead(double pos) const
    {
        if (!currentSample.loaded || pos < 0.0) return 0.0f;
        uint64_t i0 = (uint64_t)pos;
        if (i0 + 1 >= currentSample.frameCount) return 0.0f;
        float frac = (float)(pos - (double)i0);
        return currentSample.data[i0] + frac * (currentSample.data[i0 + 1] - currentSample.data[i0]);
    }

public:
    char fileNameDisplay[64] = "None";
    char detunModeName[12] = "Positive";
    char directionName[12] = "Forward";
    Param params[20];

    // --- Params ---
    Param& sampleSelect = addParam({ .key = "sample", .label = "Sample", .string = fileNameDisplay, .value = 0.0f, .step = 1.0f, .onUpdate = [](void* ctx, float val) {
                                        auto* s = (MonoSample*)ctx;
                                        int i = (int)val;
                                        if (i >= 0 && i < (int)s->sampleFiles.size()) {
                                            strncpy(s->fileNameDisplay, s->sampleFiles[i].c_str(), 63);
                                            s->loadSingleSample(s->sampleFiles[i]);
                                        }
                                    } });

    Param& transpose = addParam({ .key = "transpose", .label = "Transpose", .unit = "st", .value = 0.0f, .min = -24.0f, .max = 24.0f, .step = 1.0f });

    Param& loopStart = addParam({ .key = "loopStart", .label = "Loop Start", .unit = "%", .value = 0.0f });
    Param& loopLength = addParam({ .key = "loopLength", .label = "Loop Length", .unit = "ms", .value = 0.0f, .min = 0.0f, .max = 4000.0f, .step = 5.0f });

    // Granular
    Param& density = addParam({ .key = "density", .label = "Density", .value = 0.0f, .max = 16.0f });
    Param& grainSize = addParam({ .key = "grainSize", .label = "Grain Size", .unit = "ms", .value = 80.0f, .min = 5.0f, .max = 500.0f });
    Param& grainDelay = addParam({ .key = "grainDelay", .label = "Grain Delay", .unit = "ms", .value = 50.0f, .max = 500.0f });
    Param& delayRnd = addParam({ .key = "delayRnd", .label = "Delay Rnd", .unit = "%", .value = 0.0f });
    Param& pitchRnd = addParam({ .key = "pitchRnd", .label = "Pitch Rnd", .unit = "%", .value = 0.0f });
    Param& detune = addParam({ .key = "detune", .label = "Detune", .unit = "st", .value = 0.0f, .max = 12.0f });
    Param& detuneMode = addParam({ .key = "detunMode", .label = "Detune Mode", .string = detunModeName, .value = 1.0f, .max = 3.0f, .onUpdate = [](void* ctx, float val) {
                                      auto* s = (MonoSample*)ctx;
                                      Grains::DETUNE_MODE m = (int)val == 2 ? Grains::SYMMETRIC : (int)val == 3 ? Grains::NEGATIVE
                                                                                                                : Grains::POSITIVE;
                                      strcpy(s->detunModeName, (int)val == 2 ? "Symmetric" : (int)val == 3 ? "Negative"
                                                                                                           : "Positive");
                                      if (s->grainsInstance) s->grainsInstance->setDetuneMode(m);
                                  } });
    Param& direction = addParam({ .key = "dir", .label = "Direction", .string = directionName, .value = 1.0f, .max = 3.0f, .onUpdate = [](void* ctx, float val) {
                                     auto* s = (MonoSample*)ctx;
                                     Grains::DIRECTION d = (int)val == 2 ? Grains::BACKWARD : (int)val == 3 ? Grains::RANDOM
                                                                                                            : Grains::FORWARD;
                                     strcpy(s->directionName, (int)val == 2 ? "Backward" : (int)val == 3 ? "Random"
                                                                                                         : "Forward");
                                     if (s->grainsInstance) s->grainsInstance->setDirection(d);
                                 } });

    // Filter & FX
    Param& cutoff = addParam({ .key = "cutoff", .label = "Cutoff", .unit = "%", .value = 0.0f, .min = -100.0f, .max = 100.0f });
    Param& resonance = addParam({ .key = "res", .label = "Res", .unit = "%", .value = 0.0f });
    Param& drive = addParam({ .key = "drive", .label = "Drive", .unit = "%", .value = 0.0f });
    Param& reverbMix = addParam({ .key = "rvbMix", .label = "Reverb Mix", .unit = "%", .value = 0.0f });
    Param& reverbDamp = addParam({ .key = "rvbDamp", .label = "Rvb Damp", .unit = "%", .value = 50.0f });
    Param& dlyMix = addParam({ .key = "dlyMix", .label = "Dly Mix", .unit = "%", .value = 0.0f });
    Param& dlyTime = addParam({ .key = "dlyTime", .label = "Dly Time", .unit = "ms", .value = 125.0f, .min = 10.0f, .max = 1000.0f });
    Param& dlyFdbk = addParam({ .key = "dlyFdbk", .label = "Dly Fdbk", .unit = "%", .value = 0.0f });

    MonoSample(float sr, float* dlBuf, float* rvBuf)
        : EngineBase(Sampler, "MonoSample", params)
        , sampleRate(sr)
        , delayBuf(dlBuf)
        , reverbBuf(rvBuf)
    {
        // Reverb buffer offsets
        int pos = 0;
        for (int i = 0; i < 4; ++i) {
            combOff[i] = pos;
            pos += COMB_LEN[i];
            combIdx[i] = 0;
        }
        for (int i = 0; i < 3; ++i) {
            apOff[i] = pos;
            pos += AP_LEN[i];
            apIdx[i] = 0;
        }

        grainsInstance = new Grains([this](uint64_t p) -> float {
            if (!voice.active || !currentSample.loaded || p >= currentSample.frameCount) return 0.0f;
            return currentSample.data[p];
        });
        voice.grains = grainsInstance;

        scanSamples();
    }

    ~MonoSample()
    {
        currentSample.free();
        delete grainsInstance;
    }

    void noteOnImpl(uint8_t note, float vel)
    {
        if (!currentSample.loaded) return;
        voice.active = true;
        voice.midiNote = note;
        voice.velocity = vel;
        voice.pos = 0.0;
        voice.inLoop = false;
        voice.releasing = false;

        // Pitch calculation
        float interval = (float)note + transpose.value - 60.0f;
        voice.rate = std::pow(2.0f, interval / 12.0f);

        // Loop Setup
        if (loopLength.value > 0.5f) {
            voice.loopStart = (uint64_t)(loopStart.value * 0.01f * currentSample.frameCount);
            voice.loopEnd = std::min(voice.loopStart + (uint64_t)(loopLength.value * 0.001f * sampleRate), currentSample.frameCount);
            voice.looping = (voice.loopEnd > voice.loopStart);
        } else {
            voice.looping = false;
        }

        // Granular Setup
        voice.granular = (density.value >= 1.0f);
        if (voice.granular) {
            grainsInstance->setDensity((uint8_t)density.value);
            grainsInstance->setGrainDuration((uint64_t)(grainSize.value * 0.001f * sampleRate));
            grainsInstance->setGrainDelay((uint64_t)(grainDelay.value * 0.001f * sampleRate));
            grainsInstance->setDelayRandomize(delayRnd.value * 0.01f);
            grainsInstance->setPitchRandomize(pitchRnd.value * 0.01f);
            grainsInstance->setDetune(detune.value);
        }
    }

    void noteOffImpl(uint8_t note)
    {
        if (voice.active && voice.midiNote == note) {
            if (voice.looping && voice.inLoop) voice.releasing = true;
            else if (!voice.looping) { /* one-shot continues */ } else
                voice.active = false;
        }
    }

    float sampleImpl()
    {
        if (!voice.active || !currentSample.loaded) return fxChain(0.0f);

        float out = 0.0f;
        if (voice.granular) {
            out = grainsInstance->getGrainSample((float)voice.rate, (uint64_t)voice.pos, currentSample.frameCount);
            voice.pos += voice.rate;
            if (voice.pos >= currentSample.frameCount) voice.active = false;
        } else {
            if (voice.looping && !voice.inLoop && voice.pos >= voice.loopStart) voice.inLoop = true;
            if (voice.inLoop && !voice.releasing) {
                while (voice.pos >= voice.loopEnd)
                    voice.pos -= (voice.loopEnd - voice.loopStart);
            } else if (voice.pos >= currentSample.frameCount) {
                voice.active = false;
            }
            out = slotRead(voice.pos);
            voice.pos += voice.rate;
        }

        out = applyMorphFilter(out * voice.velocity, cutoff.value, resonance.value * 0.01f);
        out = applyDrive(out, drive.value * 0.01f);
        return fxChain(out);
    }

private:
    float applyMorphFilter(float sig, float cut, float res)
    {
        float absC = std::abs(cut);
        if (absC < 0.5f) return sig;
        svfFilter.setCutoff(0.01f + absC * 0.0098f);
        svfFilter.setResonance(res);
        auto o = svfFilter.process12(sig);
        return lerp(sig, (cut > 0 ? o.lp : o.hp), absC * 0.01f);
    }

    float fxChain(float sig)
    {
        // Delay
        if (dlyMix.value > 0.001f) {
            int n = (int)(dlyTime.value * 0.001f * sampleRate);
            float del = delayBuf[(delayWrite - n + DELAY_BUF_SIZE) % DELAY_BUF_SIZE];
            dlyFbSmooth += 0.001f * (dlyFdbk.value * 0.0085f - dlyFbSmooth);
            delayBuf[delayWrite] = sig + del * dlyFbSmooth;
            delayWrite = (delayWrite + 1) % DELAY_BUF_SIZE;
            sig = lerp(sig, sig + del, dlyMix.value * 0.01f);
        }
        // Reverb
        if (reverbMix.value > 0.001f) {
            float wet = 0, d = 0.2f + (reverbDamp.value * 0.007f);
            for (int i = 0; i < 4; ++i) {
                float val = reverbBuf[combOff[i] + combIdx[i]];
                combFb[i] = val * (1.0f - d) + combFb[i] * d;
                reverbBuf[combOff[i] + combIdx[i]] = sig + combFb[i] * 0.88f;
                if (++combIdx[i] >= COMB_LEN[i]) combIdx[i] = 0;
                wet += val;
            }
            sig = lerp(sig, wet * 0.25f, reverbMix.value * 0.01f);
        }
        return sig;
    }

    static float lerp(float a, float b, float t) { return a + t * (b - a); }
};