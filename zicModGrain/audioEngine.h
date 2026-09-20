#pragma once

#ifndef AUDIO_FOLDER
#include <filesystem>
inline std::string getGrainAudioFolder() {
    if (std::filesystem::exists("../../data/audio/wavetables")) return "../../data/audio";
    if (std::filesystem::exists("../data/audio/wavetables")) return "../data/audio";
    if (std::filesystem::exists("data/audio/wavetables")) return "data/audio";
    return "../data/audio";
}
#define AUDIO_FOLDER getGrainAudioFolder()
#endif

#include "audio/Grains.h"
#include "audio/engines/EngineBase.h"
#ifdef USE_SVF
#include "audio/filterSVF.h"
#else
#include "audio/filter.h"
#endif
#include "audio/utils/applySampleGain.h"
#include "audio/utils/math.h"
#include "helpers/clamp.h"

#include <algorithm>
#include <cmath>
#include <cstring>
#include <dirent.h>
#include <sndfile.h>
#include <string>
#include <vector>

class GranularEngine : public EngineBase<GranularEngine> {
public:
    struct SampleData {
        float* data = nullptr;
        uint64_t frameCount = 0;
        bool loaded = false;

        void free()
        {
            float* oldData = data;
            data = nullptr;
            frameCount = 0;
            loaded = false;
            if (oldData) {
                delete[] oldData;
            }
        }
    };

    struct Voice {
        bool active = true;
        double pos = 0.0;
        double rate = 1.0;
        float velocity = 1.0f;
        uint8_t midiNote = 60;

        uint64_t sampleStartFrame = 0;
        uint64_t sampleEndFrame = 0;

        bool looping = true;
        bool inLoop = false;
        bool releasing = false;
        uint64_t loopStart = 0;
        uint64_t loopEnd = 0;

        Grains* grains = nullptr;
    };

protected:
    const float sampleRate;
    SampleData currentSample;
    Voice voice;
#ifdef USE_SVF
    FilterSVF svfFilter;
#else
    EffectFilterData filterData;
#endif

    std::vector<std::string> sampleFiles;
    std::vector<float> defaultBuffer;

    void generateDefaultWaveform()
    {
        const size_t len = (size_t)(sampleRate * 1.0f);
        defaultBuffer.resize(len);
        for (size_t i = 0; i < len; ++i) {
            float phase = (float)i / (float)len;
            float sig = std::sin(phase * 2.0f * M_PI * 110.0f) * 0.5f +
                        std::sin(phase * 2.0f * M_PI * 220.0f) * 0.25f +
                        std::sin(phase * 2.0f * M_PI * 330.0f) * 0.15f +
                        std::sin(phase * 2.0f * M_PI * 440.0f) * 0.1f;
            defaultBuffer[i] = sig;
        }

        float* newData = new float[len];
        std::memcpy(newData, defaultBuffer.data(), len * sizeof(float));

        float* oldData = currentSample.data;
        currentSample.data = newData;
        currentSample.frameCount = len;
        currentSample.loaded = true;
        if (oldData) {
            delete[] oldData;
        }

        voice.pos = 0.0;
        updateSampleBounds();
    }

    void loadSingleSample(const std::string& filename)
    {
        std::string basePath = std::string(AUDIO_FOLDER);
        std::vector<std::string> tryPaths = {
            basePath + "/" + filename,
            basePath + "/samples/" + filename,
            basePath + "/er1/" + filename,
            basePath + "/packs/test1/" + filename
        };

        SNDFILE* sf = nullptr;
        SF_INFO info = {};

        for (const auto& path : tryPaths) {
            sf = sf_open(path.c_str(), SFM_READ, &info);
            if (sf) break;
        }

        if (!sf) {
            generateDefaultWaveform();
            return;
        }

        uint64_t frames = (uint64_t)info.frames;
        if (frames <= 1) {
            sf_close(sf);
            generateDefaultWaveform();
            return;
        }

        std::vector<float> tmp(frames * info.channels);
        sf_count_t read = sf_readf_float(sf, tmp.data(), frames);
        sf_close(sf);

        if (read <= 0) {
            generateDefaultWaveform();
            return;
        }

        float* newData = new float[frames];

        if (info.channels == 1) {
            std::memcpy(newData, tmp.data(), frames * sizeof(float));
        } else {
            float inv = 1.0f / (float)info.channels;
            for (uint64_t f = 0; f < frames; ++f) {
                float sum = 0;
                for (int c = 0; c < info.channels; ++c)
                    sum += tmp[f * info.channels + c];
                newData[f] = sum * inv;
            }
        }
        applySampleGain(newData, frames);

        float* oldData = currentSample.data;
        currentSample.data = newData;
        currentSample.frameCount = frames;
        currentSample.loaded = true;
        if (oldData) {
            delete[] oldData;
        }

        voice.pos = 0.0;
        updateSampleBounds();
    }

    void scanSamples()
    {
        sampleFiles.clear();
        std::string basePath = std::string(AUDIO_FOLDER);
        std::vector<std::string> searchDirs = { basePath, basePath + "/samples", basePath + "/er1", basePath + "/packs/test1" };

        for (const auto& p : searchDirs) {
            DIR* dir = opendir(p.c_str());
            if (dir) {
                struct dirent* entry;
                while ((entry = readdir(dir)) != nullptr) {
                    std::string name = entry->d_name;
                    if (name.find(".wav") != std::string::npos || name.find(".aif") != std::string::npos) {
                        sampleFiles.push_back(name);
                    }
                }
                closedir(dir);
            }
        }

        std::sort(sampleFiles.begin(), sampleFiles.end());
        sampleFiles.erase(std::unique(sampleFiles.begin(), sampleFiles.end()), sampleFiles.end());

        if (sampleFiles.empty()) {
            sampleFiles.push_back("Synthesizer.wav");
        }

        sampleSelect.max = std::max(0.0f, (float)((int)sampleFiles.size() - 1));
    }

    void updateSampleBounds()
    {
        if (!currentSample.loaded || !currentSample.data || currentSample.frameCount <= 1) return;
        voice.sampleStartFrame = (uint64_t)(sampleStart.value * 0.01f * currentSample.frameCount);
        voice.sampleEndFrame = (uint64_t)(sampleEnd.value * 0.01f * currentSample.frameCount);
        if (voice.sampleEndFrame <= voice.sampleStartFrame) {
            voice.sampleEndFrame = currentSample.frameCount;
        }
        updateVoiceLoop();
    }

    void updateVoiceLoop()
    {
        if (!currentSample.loaded || !currentSample.data || currentSample.frameCount <= 1) return;

        uint64_t totalCropLength = (voice.sampleEndFrame > voice.sampleStartFrame) ? (voice.sampleEndFrame - voice.sampleStartFrame) : 0;
        if (totalCropLength <= 1) return;

        if (loopLength.value > 0.5f) {
            voice.loopStart = voice.sampleStartFrame + (uint64_t)(loopStart.value * 0.01f * totalCropLength);
            voice.loopEnd = std::min(voice.loopStart + (uint64_t)(loopLength.value * 0.001f * sampleRate), voice.sampleEndFrame);
            voice.looping = (voice.loopEnd > voice.loopStart);
        } else {
            voice.looping = false;
        }
    }

    void updateVoiceGranular()
    {
        if (!voice.grains) return;
        uint8_t d = CLAMP((uint8_t)density.value, (uint8_t)1, (uint8_t)MAX_GRAINS);
        voice.grains->setDensity(d);
        voice.grains->setGrainDuration((uint64_t)(grainDelay.value * 0.001f * sampleRate));
        voice.grains->setGrainDelay((uint64_t)(grainDelay.value * 0.5f * 0.001f * sampleRate));
        voice.grains->setDelayRandomize(delayRnd.value * 0.01f);
        voice.grains->setPitchRandomize(pitchRnd.value * 0.01f);
        voice.grains->setDetune(grainDetune.value);
    }

    void updatePitch(float transposeVal)
    {
        float interval = (float)voice.midiNote + transposeVal - 60.0f;
        voice.rate = std::pow(2.0f, interval / 12.0f);
    }

    void updatePitch()
    {
        updatePitch(transpose.value);
    }

public:
    char fileNameDisplay[64] = "Synthesizer";
    char detunModeName[16] = "Positive";
    char directionName[16] = "Forward";

    // Parameter Array Size EXACTLY matching count of addParam calls (19 params)
    Param params[19];

    // --- Potentiometer Parameters (8 Params) ---
    Param& density = addParam({ .key = "density", .label = "Density", .value = 4.0f, .min = 1.0f, .max = 16.0f, .step = 1.0f, .onUpdate = [](void* ctx, float val) { ((GranularEngine*)ctx)->updateVoiceGranular(); } });
    Param& loopStart = addParam({ .key = "loopStart", .label = "Loop Start", .unit = "%", .value = 0.0f, .min = 0.0f, .max = 100.0f, .step = 0.5f, .onUpdate = [](void* ctx, float val) { ((GranularEngine*)ctx)->updateVoiceLoop(); } });
    Param& loopLength = addParam({ .key = "loopLength", .label = "Loop Len", .unit = "ms", .value = 1000.0f, .min = 0.0f, .max = 4000.0f, .step = 5.0f, .onUpdate = [](void* ctx, float val) { ((GranularEngine*)ctx)->updateVoiceLoop(); } });
    Param& grainDetune = addParam({ .key = "grainDetune", .label = "Grain Detune", .unit = "st", .value = 0.0f, .min = 0.0f, .max = 12.0f, .step = 0.1f, .onUpdate = [](void* ctx, float val) { ((GranularEngine*)ctx)->updateVoiceGranular(); } });
    Param& grainDelay = addParam({ .key = "grainDelay", .label = "Grain Delay", .unit = "ms", .value = 80.0f, .min = 5.0f, .max = 500.0f, .step = 1.0f, .onUpdate = [](void* ctx, float val) { ((GranularEngine*)ctx)->updateVoiceGranular(); } });
    Param& pitchRnd = addParam({ .key = "pitchRnd", .label = "Rand Detune", .unit = "%", .value = 0.0f, .min = 0.0f, .max = 100.0f, .step = 1.0f, .onUpdate = [](void* ctx, float val) { ((GranularEngine*)ctx)->updateVoiceGranular(); } });
    Param& delayRnd = addParam({ .key = "delayRnd", .label = "Rand Delay", .unit = "%", .value = 0.0f, .min = 0.0f, .max = 100.0f, .step = 1.0f, .onUpdate = [](void* ctx, float val) { ((GranularEngine*)ctx)->updateVoiceGranular(); } });
    Param& cutoff = addParam({ .key = "cutoff", .label = "Cutoff", .unit = "%", .value = 0.0f, .min = -100.0f, .max = 100.0f, .step = 1.0f });

    // --- Encoder Menu Parameters (11 Params) ---
    Param& sampleSelect = addParam({ .key = "sample", .label = "File", .string = fileNameDisplay, .value = 0.0f, .min = 0.0f, .max = 0.0f, .step = 1.0f, .onUpdate = [](void* ctx, float val) {
                                        auto* s = (GranularEngine*)ctx;
                                        int i = (int)val;
                                        if (i >= 0 && i < (int)s->sampleFiles.size()) {
                                            s->loadSingleSample(s->sampleFiles[i]);
                                        } }, .setStringFn = [](void* ctx, float value, char* str) {
                                            auto* s = (GranularEngine*)ctx;
                                            int idx = CLAMP((int)value, 0, (int)s->sampleFiles.size() - 1);
                                            if (!s->sampleFiles.empty()) {
                                                strncpy(str, s->sampleFiles[idx].c_str(), 63);
                                            } else {
                                                strcpy(str, "Synthesizer");
                                            } } });
    Param& sampleStart = addParam({ .key = "start", .label = "Start", .unit = "%", .value = 0.0f, .min = 0.0f, .max = 100.0f, .step = 0.5f, .onUpdate = [](void* ctx, float val) { ((GranularEngine*)ctx)->updateSampleBounds(); } });
    Param& sampleEnd = addParam({ .key = "end", .label = "End", .unit = "%", .value = 100.0f, .min = 0.0f, .max = 100.0f, .step = 0.5f, .onUpdate = [](void* ctx, float val) { ((GranularEngine*)ctx)->updateSampleBounds(); } });
    Param& envAttack = addParam({ .key = "attack", .label = "Attack", .unit = "%", .value = 5.0f, .min = 0.0f, .max = 100.0f, .step = 0.5f });
    Param& envRelease = addParam({ .key = "release", .label = "Release", .unit = "%", .value = 10.0f, .min = 0.0f, .max = 100.0f, .step = 0.5f });
    Param& detuneMode = addParam({ .key = "detunMode", .label = "Detune Mode", .string = detunModeName, .value = 1.0f, .min = 1.0f, .max = 3.0f, .step = 1.0f, .onUpdate = [](void* ctx, float val) {
                                      auto* s = (GranularEngine*)ctx;
                                      Grains::DETUNE_MODE m = (int)val == 2 ? Grains::SYMMETRIC : ((int)val == 3 ? Grains::NEGATIVE : Grains::POSITIVE);
                                      strcpy(s->detunModeName, (int)val == 2 ? "Symmetric" : ((int)val == 3 ? "Negative" : "Positive"));
                                      if (s->voice.grains) s->voice.grains->setDetuneMode(m);
                                  } });
    Param& direction = addParam({ .key = "dir", .label = "Direction", .string = directionName, .value = 1.0f, .min = 1.0f, .max = 3.0f, .step = 1.0f, .onUpdate = [](void* ctx, float val) {
                                     auto* s = (GranularEngine*)ctx;
                                     Grains::DIRECTION d = (int)val == 2 ? Grains::BACKWARD : ((int)val == 3 ? Grains::RANDOM : Grains::FORWARD);
                                     strcpy(s->directionName, (int)val == 2 ? "Backward" : ((int)val == 3 ? "Random" : "Forward"));
                                     if (s->voice.grains) s->voice.grains->setDirection(d);
                                 } });
    Param& resonance = addParam({ .key = "res", .label = "Resonance", .unit = "%", .value = 0.0f, .min = 0.0f, .max = 100.0f, .step = 1.0f });
    Param& transpose = addParam({ .key = "transpose", .label = "Transpose", .unit = "st", .value = 0.0f, .min = -24.0f, .max = 24.0f, .step = 1.0f, .onUpdate = [](void* ctx, float val) { ((GranularEngine*)ctx)->updatePitch(val); } });
    Param& masterVol = addParam({ .key = "vol", .label = "Volume", .unit = "%", .value = 80.0f, .min = 0.0f, .max = 100.0f, .step = 1.0f });
    Param& bpmParam = addParam({ .key = "bpm", .label = "BPM", .value = 120.0f, .min = 40.0f, .max = 240.0f, .step = 1.0f });

    GranularEngine(float sr = 44100.0f)
        : EngineBase(Synth, "zicModGrain", params)
        , sampleRate(sr)
    {
        voice.grains = new Grains([this](uint64_t p) -> float {
            if (!voice.active || !currentSample.loaded || !currentSample.data || currentSample.frameCount <= 1) return 0.0f;
            uint64_t frameIdx = p % currentSample.frameCount;
            if (frameIdx >= currentSample.frameCount) return 0.0f;
            return currentSample.data[frameIdx];
        });

        scanSamples();
        if (!sampleFiles.empty()) {
            loadSingleSample(sampleFiles[0]);
        } else {
            generateDefaultWaveform();
        }
        updateVoiceGranular();
    }

    ~GranularEngine()
    {
        currentSample.free();
        delete voice.grains;
    }

    void noteOnImpl(uint8_t note, float vel)
    {
        voice.active = true;
        voice.midiNote = note;
        voice.velocity = vel;
        voice.pos = (double)voice.sampleStartFrame;
        voice.inLoop = false;
        voice.releasing = false;
        updatePitch();
    }

    void noteOffImpl(uint8_t note)
    {
        if (voice.active && voice.midiNote == note) {
            voice.releasing = true;
        }
    }

    float sampleImpl()
    {
        if (!voice.active || !currentSample.loaded || !currentSample.data || currentSample.frameCount <= 1) return 0.0f;

        uint64_t totalCropLength = (voice.sampleEndFrame > voice.sampleStartFrame) ? (voice.sampleEndFrame - voice.sampleStartFrame) : 0;
        if (totalCropLength <= 1) return 0.0f;

        if (voice.pos >= (double)voice.sampleEndFrame || voice.pos >= (double)currentSample.frameCount) {
            if (voice.looping) {
                voice.pos = (double)voice.sampleStartFrame;
            } else {
                voice.active = false;
                return 0.0f;
            }
        }

        double currentOffset = voice.pos - (double)voice.sampleStartFrame;
        float ampModifier = 1.0f;
        float attackThreshold = envAttack.value * 0.01f * totalCropLength;
        if (attackThreshold > 0.0f && currentOffset < attackThreshold) {
            ampModifier = static_cast<float>(currentOffset / attackThreshold);
        }

        float releaseThreshold = (100.0f - envRelease.value) * 0.01f * totalCropLength;
        if (currentOffset > releaseThreshold) {
            float totalFadingRange = totalCropLength - releaseThreshold;
            if (totalFadingRange > 0.0f) {
                ampModifier = 1.0f - (static_cast<float>(currentOffset - releaseThreshold) / totalFadingRange);
            }
        }

        if (voice.looping && voice.pos >= (double)voice.loopEnd && voice.loopEnd > voice.loopStart) {
            voice.pos -= (double)(voice.loopEnd - voice.loopStart);
        }

        float out = voice.grains->getGrainSample(
            (float)voice.rate,
            (uint64_t)voice.pos,
            currentSample.frameCount);

        voice.pos += voice.rate;

        out *= (voice.velocity * ampModifier * (masterVol.value * 0.01f));
        out = applyMorphFilter(out, cutoff.value, resonance.value * 0.01f);

        return out;
    }

    float drawImpl(float x)
    {
        if (!currentSample.loaded || !currentSample.data || currentSample.frameCount <= 1) return 0.0f;
        uint64_t frameIdx = static_cast<uint64_t>(x * (currentSample.frameCount - 1));
        if (frameIdx >= currentSample.frameCount) return 0.0f;
        return currentSample.data[frameIdx];
    }

    int getVoiceCountImpl()
    {
        if (voice.active && voice.grains) {
            return voice.grains->getDensity();
        }
        return voice.active ? 1 : 0;
    }

    float getPlayheadImpl(int index)
    {
        if (!voice.active || !currentSample.loaded || !currentSample.data || currentSample.frameCount <= 1) return -1.0f;
        if (voice.grains) {
            uint64_t grainPos = voice.grains->getGrainPosition(index);
            if (currentSample.frameCount <= 1) return -1.0f;
            return static_cast<float>(grainPos % currentSample.frameCount) / (float)currentSample.frameCount;
        }
        if (currentSample.frameCount <= 1) return -1.0f;
        return static_cast<float>((uint64_t)voice.pos % currentSample.frameCount) / (float)currentSample.frameCount;
    }

private:
    float applyMorphFilter(float sig, float cutoffParam, float res)
    {
        float absC = std::abs(cutoffParam);
        if (absC < 0.5f) return sig;

        float normCutoff = 0.01f + absC * 0.0098f;
        float normRes = CLAMP(res, 0.0f, 0.98f);

        float t = CLAMP((cutoffParam + 100.0f) * 0.005f, 0.0f, 1.0f);
#ifdef USE_SVF
        svfFilter.setCutoff(normCutoff);
        svfFilter.setResonance(normRes);
        auto out = svfFilter.process12(sig);
        float filtered = lerp(out.lp, out.hp, t);
#else
        filterData.set(normCutoff, normRes);
        filterData.setSampleData(sig);
        float filtered = lerp(filterData.lp, filterData.hp, t);
#endif
        return lerp(sig, filtered, absC * 0.01f);
    }

    static float lerp(float a, float b, float t) { return a + t * (b - a); }
};
