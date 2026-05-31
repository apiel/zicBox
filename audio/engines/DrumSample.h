#pragma once

#ifndef AUDIO_FOLDER
#include "host/constants.h"
#endif

#include "audio/MultiFx.h"
#include "audio/effects/applyCompression.h"
#include "audio/effects/applyReverb.h"
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

class DrumSample : public EngineBase<DrumSample> {
public:
    MultiFx multiFx;

protected:
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
        uint8_t midiNote = 60;
        float velocity = 1.0f;
        double pos = 0.0;
        double rate = 1.0;

        uint64_t startFrame = 0;
        uint64_t endFrame = 0;

        // Pitch Mod Timers
        double elapsedSamples = 0.0;

        // Sub-oscillator phase
        double subPhase = 0.0;

        // Transient Envelope Tracker (for targeted clipping)
        float transientFollower = 0.0f;
    };

    const float sampleRate;
    SampleData currentSample;
    Voice voice;

#ifdef USE_SVF
    FilterSVF svfFilter;
#else
    EffectFilterData filterData;
#endif

    std::vector<std::string> sampleFiles;

    float* reverbBuf = nullptr;
    int reverbIndex = 0;
    float compressionState = 0.0f;

    // Internal rumble system memory state
    float rumbleLP = 0.0f;
    float rumbleDelaySample = 0.0f;

    void updateSampleBounds()
    {
        if (!currentSample.loaded) return;
        voice.startFrame = static_cast<uint64_t>(sampleStart.value * 0.01f * currentSample.frameCount);
        voice.endFrame = static_cast<uint64_t>(sampleEnd.value * 0.01f * currentSample.frameCount);
        voice.endFrame = std::min(voice.endFrame, currentSample.frameCount);
        if (voice.startFrame >= voice.endFrame) {
            if (voice.endFrame > 0) voice.startFrame = voice.endFrame - 1;
        }
    }

public:
    char fileNameDisplay[64] = "None";
    char fxName[24] = "Off";

    // STRICTLY 20 PARAMETERS total. Bound seamlessly within engine specifications.
    Param params[20];

    Param& sampleSelect = addParam({ .key = "sample", .label = "Sample", .string = fileNameDisplay, .value = 0.0f, .step = 1.0f, .onUpdate = [](void* ctx, float val) {
        auto* s = (DrumSample*)ctx;
        int i = (int)val;
        if (i >= 0 && i < (int)s->sampleFiles.size()) {
            s->loadSingleSample(s->sampleFiles[i]);
        } }, .setStringFn = [](void* ctx, float value, char* str) {
        auto* s = (DrumSample*)ctx;
        if(!s->sampleFiles.empty()) strncpy(str, s->sampleFiles[(int)value].c_str(), 63); }, .stringToFloatFn = [](void* ctx, const char* valStr) {
        auto e = (DrumSample*)ctx;
        for (int i = 0; i < (int)e->sampleFiles.size(); i++) {
            if (e->sampleFiles[i] == valStr) return (float)i;
        }
        return 0.0f; } });

    Param& transpose = addParam({ .key = "transpose", .label = "Transpose", .unit = "st", .value = 0.0f, .min = -36.0f, .max = 36.0f, .step = 1.0f });
    Param& sampleStart = addParam({ .key = "start", .label = "Start", .unit = "%", .value = 0.0f, .min = 0.0f, .max = 100.0f, .step = 0.1f, .onUpdate = [](void* ctx, float val) { ((DrumSample*)ctx)->updateSampleBounds(); } });
    Param& sampleEnd = addParam({ .key = "end", .label = "End", .unit = "%", .value = 100.0f, .min = 0.0f, .max = 100.0f, .step = 0.1f, .onUpdate = [](void* ctx, float val) { ((DrumSample*)ctx)->updateSampleBounds(); } });

    // --- Multiphasic 3-Stage Pitch Shaping ---
    Param& pitStartAmt = addParam({ .key = "pitStartAmt", .label = "Click Pitch", .unit = "st", .value = 0.0f, .min = -48.0f, .max = 48.0f, .step = 0.5f });
    Param& pitSweepAmt = addParam({ .key = "pitSweepAmt", .label = "Sweep Pitch", .unit = "st", .value = 0.0f, .min = -48.0f, .max = 48.0f, .step = 0.5f });
    Param& pitSweepDecay = addParam({ .key = "pitSweepDec", .label = "Sweep Time", .unit = "ms", .value = 25.0f, .min = 2.0f, .max = 300.0f, .step = 1.0f });
    Param& pitBendAmt = addParam({ .key = "pitBendAmt", .label = "Body Bend", .unit = "st", .value = 12.0f, .min = -48.0f, .max = 48.0f, .step = 0.5f });
    Param& pitBendDecay = addParam({ .key = "pitBendDec", .label = "Bend Time", .unit = "ms", .value = 80.0f, .min = 5.0f, .max = 1500.0f, .step = 5.0f });

    // --- Targeted Clip & Weight Shapers ---
    Param& transientClip = addParam({ .key = "transClip", .label = "Punch Clip", .unit = "%", .value = 0.0f, .min = 0.0f, .max = 100.0f });
    Param& transientRelease = addParam({ .key = "transRel", .label = "Punch Length", .unit = "ms", .value = 25.0f, .max = 1000.0f });

    // --- Rumble Shapers ---
    Param& rumbleAmt = addParam({ .key = "rumbleAmt", .label = "Rumble", .unit = "%", .value = 0.0f, .min = 0.0f, .max = 100.0f });
    Param& rumbleGap = addParam({ .key = "rumbleGap", .label = "Rum. Gap", .unit = "ms", .value = 80.0f, .min = 10.0f, .max = 400.0f });
    Param& rumbleFilter = addParam({ .key = "rumbleFilt", .label = "Rum. LP", .unit = "%", .value = 100.0f, .min = 0.0f, .max = 100.0f });

    // --- Filter & Global Processing ---
    Param& cutoff = addParam({ .key = "cutoff", .label = "Cutoff", .unit = "%", .value = 0.0f, .min = -100.0f, .max = 100.0f });
    Param& resonance = addParam({ .key = "res", .label = "Resonance", .unit = "%", .value = 0.0f });
    Param& compress = addParam({ .key = "compress", .label = "Compress", .unit = "%", .value = 0.0f });
    Param& fxType = addParam({ .key = "fxType", .label = "FX Type", .string = fxName, .value = 0.0f, .max = (float)MultiFx::FX_COUNT - 1, .step = 1.0f, // Skip Format
        .onUpdate = [](void* ctx, float v) { auto e = (DrumSample*)ctx; e->multiFx.setEffect(v); }, // Skip Format
        .setStringFn = [](void* ctx, float value, char* str) { auto e = (DrumSample*)ctx; strcpy(str, e->multiFx.getEffectName(value)); }, // Skip Format
        .stringToFloatFn = [](void* ctx, const char* valStr) { auto e = (DrumSample*)ctx; return (float)e->multiFx.getEffect(valStr); } }); // Skip Format
    Param& fxAmt = addParam({ .key = "fxAmt", .label = "FX Amount", .unit = "%", .value = 0.0f });
    Param& reverb = addParam({ .key = "rvbMix", .label = "Reverb", .unit = "%", .value = 0.0f });

    DrumSample(float sr, float* rvBuf, float* fxBuf)
        : EngineBase(Sampler, "DrumEngine", params)
        , multiFx(sr, fxBuf)
        , sampleRate(sr)
        , reverbBuf(rvBuf)
    {
        scanSamples();
        if (sampleFiles.size() > 0) {
            sampleSelect.max = (float)(sampleFiles.size() - 1);
            sampleSelect.onUpdate(this, 0.0f);
        }
    }

    ~DrumSample() { currentSample.free(); }

    void noteOnImpl(uint8_t note, float vel)
    {
        if (!currentSample.loaded) return;

        updateSampleBounds();
        voice.active = true;
        voice.midiNote = note;
        voice.velocity = vel;
        voice.pos = static_cast<double>(voice.startFrame);

        // Reset tracking states
        voice.elapsedSamples = 0.0;
        voice.subPhase = 0.0;
        voice.transientFollower = 0.0f;

        rumbleLP = 0.0f;
        rumbleDelaySample = 0.0f;
    }

    float sampleImpl()
    {
        if (!voice.active || !currentSample.loaded) return applyBufferedFx(0.0f);

        if (voice.pos >= static_cast<double>(voice.endFrame) || voice.pos >= static_cast<double>(currentSample.frameCount)) {
            voice.active = false;
            return 0.0f;
        }

        // --- MULTIPHASIC 3-STAGE PITCH GENERATOR ---
        // Stage 1: Absolute Initial Sharp Click Env (Locked at tight 12ms target execution window)
        double clickSamples = 0.012 * sampleRate;
        float clickEnv = std::exp(-5.0 * (voice.elapsedSamples / clickSamples));
        if (voice.elapsedSamples > clickSamples * 2.0) clickEnv = 0.0f;

        // Stage 2: Mid-Transient Pitch Sweep Scooper Env (Variable control)
        double sweepSamplesTotal = (pitSweepDecay.value * 0.001f) * sampleRate;
        float sweepEnv = std::exp(-5.0 * (voice.elapsedSamples / sweepSamplesTotal));
        if (voice.elapsedSamples > sweepSamplesTotal * 3.0) sweepEnv = 0.0f;

        // Stage 3: Low-End Body Bend/Fundamental Drop Env (Variable control)
        double bendSamplesTotal = (pitBendDecay.value * 0.001f) * sampleRate;
        float bendEnv = std::exp(-5.0 * (voice.elapsedSamples / bendSamplesTotal));
        if (voice.elapsedSamples > bendSamplesTotal * 3.0) bendEnv = 0.0f;

        // Compute cumulative summation of active pitch curves
        float totalPitchMod = (clickEnv * pitStartAmt.value) + (sweepEnv * pitSweepAmt.value) + (bendEnv * pitBendAmt.value);

        float targetInterval = (float)voice.midiNote + transpose.value + totalPitchMod - 60.0f;
        voice.rate = std::pow(2.0f, targetInterval / 12.0f);

        // Frame buffer access
        float out = slotRead(voice.pos);
        voice.pos += voice.rate;

        out *= voice.velocity;

        // --- Surgical Attack Clipping Engine ---
        if (transientClip.value > 0.01f) {
            float absSig = std::abs(out);
            float attCoef = std::exp(-1.0f / (0.002f * sampleRate));
            float relCoef = std::exp(-1.0f / (transientRelease.value * 0.001f * sampleRate));
            float coef = (absSig > voice.transientFollower) ? attCoef : relCoef;
            voice.transientFollower = coef * voice.transientFollower + (1.0f - coef) * absSig;

            float clipThreshold = 1.0f - (transientClip.value * 0.007f);
            clipThreshold = CLAMP(clipThreshold, 0.1f, 1.0f);

            float drivenValue = out / clipThreshold;
            float clippedOut = CLAMP(drivenValue, -1.0f, 1.0f);

            out = a_lerp(out, clippedOut, voice.transientFollower);
        }

        // --- Dynamic Techno Rumble Module ---
        if (rumbleAmt.value > 0.01f) {
            double targetGapSamples = (rumbleGap.value * 0.001f) * sampleRate;

            if (voice.elapsedSamples >= targetGapSamples) {
                float timeSinceGap = static_cast<float>(voice.elapsedSamples - targetGapSamples) / sampleRate;

                float riseEnv = 1.0f - std::exp(-timeSinceGap / 0.030f);
                float decayEnv = std::exp(-timeSinceGap / 0.350f);

                float targetHz = 40.0f + (std::pow(1.0f - rumbleFilter.value * 0.01f, 2.0f) * 960.0f);

                float lowPassCoeff = 1.0f - std::exp(-2.0f * M_PI * targetHz / sampleRate);
                lowPassCoeff = CLAMP(lowPassCoeff, 0.001f, 1.0f);

                rumbleLP += lowPassCoeff * (rumbleDelaySample - rumbleLP);
                float dirtySub = std::tanh(rumbleLP * 2.5f);

                out += dirtySub * riseEnv * decayEnv * (rumbleAmt.value * 0.015f);
            }

            rumbleDelaySample = out;
        }

        voice.elapsedSamples += 1.0;

        // --- Morph Filtering and Global Processor Output ---
        out = applyMorphFilter(out, cutoff.value, resonance.value * 0.01f);
        return applyBufferedFx(out);
    }

    float drawImpl(float x)
    {
        if (!currentSample.loaded || currentSample.frameCount == 0) return 0.0f;
        updateSampleBounds();
        uint64_t totalFrames = voice.endFrame - voice.startFrame;
        if (totalFrames == 0) return 0.0f;
        uint64_t targetFrame = voice.startFrame + static_cast<uint64_t>(x * (totalFrames - 1));
        if (targetFrame >= currentSample.frameCount) return 0.0f;
        return currentSample.data[targetFrame];
    }

    const char* getNameXYImpl() { return "Filter"; }
    void setXYImpl(XY xy)
    {
        cutoff.set(xy.x * 200.0f - 100.0f);
        resonance.set(xy.y * 100.0f);
    }
    XY getXYImpl() { return { (cutoff.value + 100.0f) / 200.0f, resonance.value * 0.01f }; }
    int getVoiceCountImpl() { return voice.active ? 1 : 0; }

    float getPlayheadImpl(int index)
    {
        if (!voice.active || index != 0 || currentSample.frameCount == 0) return -1.0f;
        uint64_t totalFrames = voice.endFrame - voice.startFrame;
        if (totalFrames == 0) return -1.0f;
        return static_cast<float>(voice.pos - static_cast<double>(voice.startFrame)) / totalFrames;
    }

protected:
    float applyBufferedFx(float out)
    {
        out = multiFx.apply(out, fxAmt.value * 0.01f);
        if (compress.value > 0.0f) {
            out = applyCompression2(out, compress.value * 0.01f, compressionState);
        }
        return applyReverb(out, reverb.value * 0.01f, reverbBuf, reverbIndex);
    }

    float slotRead(double pos) const
    {
        if (!currentSample.loaded || pos < 0.0) return 0.0f;
        uint64_t i0 = static_cast<uint64_t>(pos);
        if (i0 + 1 >= currentSample.frameCount) return 0.0f;
        float frac = static_cast<float>(pos - static_cast<double>(i0));
        return currentSample.data[i0] + frac * (currentSample.data[i0 + 1] - currentSample.data[i0]);
    }

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
        float filtered = a_lerp(out.lp, out.hp, t);
#else
        filterData.set(normCutoff, normRes);
        filterData.setSampleData(sig);
        float filtered = a_lerp(filterData.lp, filterData.hp, t);
#endif
        return a_lerp(sig, filtered, absC * 0.01f);
    }

    static float a_lerp(float a, float b, float t) { return a + t * (b - a); }

    void loadSingleSample(const std::string& filename)
    {
        currentSample.free();
        std::string path = std::string(AUDIO_FOLDER) + "/samples/" + filename;

        SF_INFO info = {};
        SNDFILE* sf = sf_open(path.c_str(), SFM_READ, &info);
        if (!sf) return;

        uint64_t frames = (uint64_t)info.frames;
        std::vector<float> tmp(frames * info.channels);
        sf_readf_float(sf, tmp.data(), frames);
        sf_close(sf);

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
    }
};