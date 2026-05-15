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
#include <cstring>
#include <dirent.h>
#include <sndfile.h>
#include <string>
#include <vector>

class MonoSample : public EngineBase<MonoSample> {
public:
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

        uint64_t sampleStartFrame = 0;
        uint64_t sampleEndFrame = 0;

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
    FilterSVF svfFilter;

    float* delayBuf = nullptr;
    float* reverbBuf = nullptr;
    int delayWrite = 0;
    float dlyFbSmooth = 0.0f;

    std::vector<std::string> sampleFiles;

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

        updateSampleBounds();
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

    void updateSampleBounds()
    {
        if (!currentSample.loaded) return;
        voice.sampleStartFrame = (uint64_t)(sampleStart.value * 0.01f * currentSample.frameCount);
        voice.sampleEndFrame = (uint64_t)(sampleEnd.value * 0.01f * currentSample.frameCount);
        if (voice.sampleEndFrame <= voice.sampleStartFrame) {
            voice.sampleEndFrame = currentSample.frameCount;
        }
        updateVoiceLoop();
    }

    void updateVoiceLoop()
    {
        if (!currentSample.loaded) return;

        if (loopLength.value > 0.5f) {
            uint64_t totalCropLength = voice.sampleEndFrame - voice.sampleStartFrame;
            // Loop points are calculated relative to the cropped window
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

        voice.granular = (density.value >= 1.0f);
        if (voice.granular) {
            voice.grains->setDensity((uint8_t)density.value);
            voice.grains->setGrainDuration((uint64_t)(grainSize.value * 0.001f * sampleRate));
            voice.grains->setGrainDelay((uint64_t)(grainDelay.value * 0.001f * sampleRate));
            voice.grains->setDelayRandomize(delayRnd.value * 0.01f);
            voice.grains->setPitchRandomize(pitchRnd.value * 0.01f);
            voice.grains->setDetune(detune.value);
        }
    }

    void updatePitch()
    {
        if (!voice.active) return;
        float interval = (float)voice.midiNote + transpose.value - 60.0f;
        voice.rate = std::pow(2.0f, interval / 12.0f);
    }

public:
    char fileNameDisplay[64] = "None";
    char detunModeName[12] = "Positive";
    char directionName[12] = "Forward";
    Param params[24];

    // --- Params ---
    Param& sampleSelect = addParam({ .key = "sample", .label = "Sample", .string = fileNameDisplay, .value = 0.0f, .step = 1.0f, .onUpdate = [](void* ctx, float val) {
                                        auto* s = (MonoSample*)ctx;
                                        int i = (int)val;
                                        if (i >= 0 && i < (int)s->sampleFiles.size()) {
                                            strncpy(s->fileNameDisplay, s->sampleFiles[i].c_str(), 63);
                                            s->loadSingleSample(s->sampleFiles[i]);
                                        }
                                    } });

    Param& transpose = addParam({ .key = "transpose", .label = "Transpose", .unit = "st", .value = 0.0f, .min = -24.0f, .max = 24.0f, .step = 1.0f, .onUpdate = [](void* ctx, float val) { ((MonoSample*)ctx)->updatePitch(); } });

    // Crop Boundaries & Percentage-based Envelopes
    Param& sampleStart = addParam({ .key = "start", .label = "Start", .unit = "%", .value = 0.0f, .min = 0.0f, .max = 100.0f, .step = 0.5f, .onUpdate = [](void* ctx, float val) { ((MonoSample*)ctx)->updateSampleBounds(); } });
    Param& sampleEnd = addParam({ .key = "end", .label = "End", .unit = "%", .value = 100.0f, .min = 0.0f, .max = 100.0f, .step = 0.5f, .onUpdate = [](void* ctx, float val) { ((MonoSample*)ctx)->updateSampleBounds(); } });
    Param& envAttack = addParam({ .key = "attack", .label = "Attack", .unit = "%", .value = 0.0f, .min = 0.0f, .max = 100.0f, .step = 0.5f });
    Param& envRelease = addParam({ .key = "release", .label = "Release", .unit = "%", .value = 0.0f, .min = 0.0f, .max = 100.0f, .step = 0.5f });

    Param& loopStart = addParam({ .key = "loopStart", .label = "Loop Start", .unit = "%", .value = 0.0f, .step = 0.5f, .onUpdate = [](void* ctx, float val) { ((MonoSample*)ctx)->updateVoiceLoop(); } });
    Param& loopLength = addParam({ .key = "loopLength", .label = "Loop Length", .unit = "ms", .value = 0.0f, .min = 0.0f, .max = 4000.0f, .step = 5.0f, .onUpdate = [](void* ctx, float val) { ((MonoSample*)ctx)->updateVoiceLoop(); } });

    // Granular
    Param& density = addParam({ .key = "density", .label = "Density", .value = 0.0f, .max = 16.0f, .onUpdate = [](void* ctx, float val) { ((MonoSample*)ctx)->updateVoiceGranular(); } });
    Param& grainSize = addParam({ .key = "grainSize", .label = "Grain Size", .unit = "ms", .value = 80.0f, .min = 5.0f, .max = 500.0f, .onUpdate = [](void* ctx, float val) { ((MonoSample*)ctx)->updateVoiceGranular(); } });
    Param& grainDelay = addParam({ .key = "grainDelay", .label = "Grain Delay", .unit = "ms", .value = 50.0f, .max = 500.0f, .onUpdate = [](void* ctx, float val) { ((MonoSample*)ctx)->updateVoiceGranular(); } });
    Param& delayRnd = addParam({ .key = "delayRnd", .label = "Delay Rnd", .unit = "%", .value = 0.0f, .onUpdate = [](void* ctx, float val) { ((MonoSample*)ctx)->updateVoiceGranular(); } });
    Param& pitchRnd = addParam({ .key = "pitchRnd", .label = "Pitch Rnd", .unit = "%", .value = 0.0f, .onUpdate = [](void* ctx, float val) { ((MonoSample*)ctx)->updateVoiceGranular(); } });
    Param& detune = addParam({ .key = "detune", .label = "Detune", .unit = "st", .value = 0.0f, .max = 12.0f, .onUpdate = [](void* ctx, float val) { ((MonoSample*)ctx)->updateVoiceGranular(); } });
    Param& detuneMode = addParam({ .key = "detunMode", .label = "Detune Mode", .string = detunModeName, .value = 1.0f, .max = 3.0f, .onUpdate = [](void* ctx, float val) {
                                      auto* s = (MonoSample*)ctx;
                                      Grains::DETUNE_MODE m = (int)val == 2 ? Grains::SYMMETRIC : (int)val == 3 ? Grains::NEGATIVE
                                                                                                                : Grains::POSITIVE;
                                      strcpy(s->detunModeName, (int)val == 2 ? "Symmetric" : (int)val == 3 ? "Negative"
                                                                                                           : "Positive");
                                      if (s->voice.grains) s->voice.grains->setDetuneMode(m);
                                  } });
    Param& direction = addParam({ .key = "dir", .label = "Direction", .string = directionName, .value = 1.0f, .max = 3.0f, .onUpdate = [](void* ctx, float val) {
                                     auto* s = (MonoSample*)ctx;
                                     Grains::DIRECTION d = (int)val == 2 ? Grains::BACKWARD : (int)val == 3 ? Grains::RANDOM
                                                                                                            : Grains::FORWARD;
                                     strcpy(s->directionName, (int)val == 2 ? "Backward" : (int)val == 3 ? "Random"
                                                                                                         : "Forward");
                                     if (s->voice.grains) s->voice.grains->setDirection(d);
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

        voice.grains = new Grains([this](uint64_t p) -> float {
            if (!voice.active || !currentSample.loaded || p >= currentSample.frameCount) return 0.0f;
            return currentSample.data[p];
        });

        scanSamples();
        sampleSelect.onUpdate(this, sampleSelect.value);
    }

    ~MonoSample()
    {
        currentSample.free();
        delete voice.grains;
    }

    void noteOnImpl(uint8_t note, float vel)
    {
        if (!currentSample.loaded) return;
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
        if (!voice.active || !currentSample.loaded) return fxChain(0.0f);

        uint64_t totalCropLength = voice.sampleEndFrame - voice.sampleStartFrame;
        if (totalCropLength == 0) {
            voice.active = false;
            return fxChain(0.0f);
        }

        // Check hard termination limits
        if (voice.pos >= (double)voice.sampleEndFrame || voice.pos >= (double)currentSample.frameCount) {
            voice.active = false;
            return fxChain(0.0f);
        }

        // --- DYNAMIC AMPLITUDE ENVELOPE (ATTACK & RELEASE %) ---
        double currentOffset = voice.pos - (double)voice.sampleStartFrame;
        float ampModifier = 1.0f;

        // 1. Attack Zone
        float attackThreshold = envAttack.value * 0.01f * totalCropLength;
        if (attackThreshold > 0.0f && currentOffset < attackThreshold) {
            ampModifier = static_cast<float>(currentOffset / attackThreshold);
        }

        // 2. Release Zone
        float releaseThreshold = (100.0f - envRelease.value) * 0.01f * totalCropLength;
        if (currentOffset > releaseThreshold) {
            float totalFadingRange = voice.sampleEndFrame - voice.sampleStartFrame - releaseThreshold;
            if (totalFadingRange > 0.0f) {
                ampModifier = 1.0f - (static_cast<float>(currentOffset - releaseThreshold) / totalFadingRange);
            }
        }

        // --- LOOPING REGION RULES ---
        if (voice.looping && !voice.inLoop && voice.pos >= (double)voice.loopStart) {
            voice.inLoop = true;
        }

        if (voice.inLoop && voice.looping && !voice.releasing) {
            if (voice.pos >= (double)voice.loopEnd) {
                voice.pos -= (double)(voice.loopEnd - voice.loopStart);
            }
        }

        // --- SAMPLE GENERATION ---
        float out = 0.0f;
        if (voice.granular) {
            out = voice.grains->getGrainSample(
                (float)voice.rate,
                (uint64_t)voice.pos,
                currentSample.frameCount);
        } else {
            out = slotRead(voice.pos);
        }

        voice.pos += voice.rate;

        // Apply Envelope scaling
        out *= (voice.velocity * ampModifier);

        out = applyMorphFilter(out, cutoff.value, resonance.value * 0.01f);
        out = applyDrive(out, drive.value * 0.01f);

        return fxChain(out);
    }

    float drawImpl(float x)
    {
        if (!currentSample.loaded || currentSample.frameCount == 0) return 0.0f;

        // 1. Crop calculations
        uint64_t startFrame = (uint64_t)(sampleStart.value * 0.01f * currentSample.frameCount);
        uint64_t endFrame = (uint64_t)(sampleEnd.value * 0.01f * currentSample.frameCount);
        if (endFrame <= startFrame) endFrame = currentSample.frameCount;
        uint64_t lengthFrames = endFrame - startFrame;

        if (lengthFrames == 0) return 0.0f;

        // 2. Map X directly to the localized view window
        uint64_t frameIdx = static_cast<uint64_t>(x * (lengthFrames - 1)) + startFrame;
        if (frameIdx >= currentSample.frameCount) return 0.0f;

        float wave = currentSample.data[frameIdx];

        // 3. Apply Visual Fade Modifications
        float currentLocalOffset = static_cast<float>(frameIdx - startFrame);

        // Visual Attack Line
        float attackFrames = envAttack.value * 0.01f * lengthFrames;
        if (attackFrames > 0.0f && currentLocalOffset < attackFrames) {
            wave *= (currentLocalOffset / attackFrames);
        }

        // Visual Release Fade Line
        float releaseStartFrame = (100.0f - envRelease.value) * 0.01f * lengthFrames;
        if (currentLocalOffset > releaseStartFrame) {
            float fadeLength = lengthFrames - releaseStartFrame;
            if (fadeLength > 0.0f) {
                wave *= (1.0f - ((currentLocalOffset - releaseStartFrame) / fadeLength));
            }
        }

        return wave;
    }

    // --- GETTERS CONFIGURED RELATIVE TO THE NEW VISUAL CROP SPACE ---

    float getLoopStartImpl()
    {
        if (!currentSample.loaded || currentSample.frameCount == 0 || !voice.looping) return 0.0f;
        uint64_t totalCropLength = voice.sampleEndFrame - voice.sampleStartFrame;
        if (totalCropLength == 0) return 0.0f;

        // Map loop point relative to the UI zoom view window
        return static_cast<float>(voice.loopStart - voice.sampleStartFrame) / totalCropLength;
    }

    void setLoopStartImpl(float start)
    {
        loopStart.set(start * 100.0f);
    }

    float getLoopLengthImpl()
    {
        if (!currentSample.loaded || currentSample.frameCount == 0 || !voice.looping) return 0.0f;
        uint64_t totalCropLength = voice.sampleEndFrame - voice.sampleStartFrame;
        if (totalCropLength == 0) return 0.0f;

        uint64_t lengthFrames = voice.loopEnd - voice.loopStart;
        return static_cast<float>(lengthFrames) / totalCropLength;
    }

    int getVoiceCountImpl()
    {
        if (voice.active && voice.granular && voice.grains) {
            return voice.grains->getDensity();
        }
        return voice.active ? 1 : 0;
    }

    float getPlayheadImpl(int index)
    {
        if (!voice.active || currentSample.frameCount == 0) return -1.0f;
        uint64_t totalCropLength = voice.sampleEndFrame - voice.sampleStartFrame;
        if (totalCropLength == 0) return -1.0f;

        if (voice.granular && voice.grains) {
            uint64_t grainPos = voice.grains->getGrainPosition(index);
            if (grainPos < voice.sampleStartFrame || grainPos > voice.sampleEndFrame) return -1.0f;
            return static_cast<float>(grainPos - voice.sampleStartFrame) / totalCropLength;
        } else {
            if (index != 0) return -1.0f;
            if (voice.pos < voice.sampleStartFrame || voice.pos > voice.sampleEndFrame) return -1.0f;
            return static_cast<float>(voice.pos - (double)voice.sampleStartFrame) / totalCropLength;
        }
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
        if (dlyMix.value > 0.001f) {
            int n = (int)(dlyTime.value * 0.001f * sampleRate);
            float del = delayBuf[(delayWrite - n + DELAY_BUF_SIZE) % DELAY_BUF_SIZE];
            dlyFbSmooth += 0.001f * (dlyFdbk.value * 0.0085f - dlyFbSmooth);
            delayBuf[delayWrite] = sig + del * dlyFbSmooth;
            delayWrite = (delayWrite + 1) % DELAY_BUF_SIZE;
            sig = lerp(sig, sig + del, dlyMix.value * 0.01f);
        }
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