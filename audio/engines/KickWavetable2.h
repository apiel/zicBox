#pragma once

#include <algorithm>
#include <cmath>
#include <cstring>
#include <filesystem>
#include <sndfile.h>
#include <vector>

#ifndef AUDIO_FOLDER
#include "host/constants.h"
#endif

#include "audio/EnvelopDrumAmp.h"
#include "audio/Wavetable.h"
#include "audio/effects/applyCompression.h"
#include "audio/effects/applyDrive.h"
#include "audio/engines/EngineBase.h"
#include "audio/utils/math.h"
#include <atomic>
#include <cstdint>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

class KickWavetable2 : public EngineBase<KickWavetable2> {
public:
    EnvelopDrumAmp envelopAmp;
    std::atomic<bool> isBodyMuted { false };
    float currentMorphVal = 1.0f;

protected:
    const float sampleRate;
    float velocity = 1.0f;

    float carrierPhase = 0.0f;
    float modulatorPhase = 0.0f;
    float modulationEnvelope = 0.0f;
    float compressionEnv = 0.0f;

    float lerp(float a, float b, float t) { return a + t * (b - a); }

public:
    Wavetable wavetable;
    char wtName[64] = "---";

    Param params[12];

    bool isSingleCycleFile()
    {
        return (wavetable.sampleCount <= 0.0f) || (wavetable.sampleCount * 64.0f < 64.0f * 2048.0f * 0.9f);
    }

    float getActiveCycleSampleCount()
    {
        if (wavetable.sampleCount <= 0.0f) return 2048.0f;
        if (isSingleCycleFile()) {
            return wavetable.sampleCount * 64.0f;
        }
        return wavetable.sampleCount;
    }

    float readWaveformSample(float morphVal, float pos)
    {
        if (wavetable.sampleCount <= 0.0f) return 0.0f;
        if (isSingleCycleFile()) {
            float cycleLen = getActiveCycleSampleCount();
            pos = pos - std::floor(pos / cycleLen) * cycleLen;
            return linearInterpolationAbsolute(pos, cycleLen, wavetable.samples());
        }
        return wavetable.readMorph(morphVal, pos);
    }

    // Option 2: Apply current pot parameter values directly to modulate the loaded WAV waveform
    float synthesizeParametricSample(float phase)
    {
        // 1. Phase Offset
        float pOffset = phaseOffset.value * 0.01f;
        float p = phase + pOffset;
        p = p - std::floor(p);

        // 2. Phase Skewing
        float skewNorm = std::clamp(skew.value * 0.01f, 0.05f, 0.95f);
        float pWarped = 0.0f;
        if (p < skewNorm) {
            pWarped = 0.5f * (p / skewNorm);
        } else {
            pWarped = 0.5f + 0.5f * ((p - skewNorm) / (1.0f - skewNorm));
        }

        // 3. Read base sample from current loaded wavetable preset
        float cycleLen = getActiveCycleSampleCount();
        float rawSample = readWaveformSample(currentMorphVal, pWarped * cycleLen);

        // 4. Wave Shape Morphing (blend raw sample towards Tri / Saw / Square)
        float shapeVal = waveShape.value * 0.01f;
        float s = rawSample;

        if (shapeVal > 0.001f) {
            float sinVal = std::sin(pWarped * 2.0f * (float)M_PI);
            float triVal = (pWarped < 0.5f) ? (4.0f * pWarped - 1.0f) : (3.0f - 4.0f * pWarped);
            float sawVal = 1.0f - 2.0f * pWarped;
            float sqVal = (pWarped < 0.5f) ? 1.0f : -1.0f;

            float targetVal = 0.0f;
            if (shapeVal <= 0.3333f) {
                float t = shapeVal / 0.3333f;
                targetVal = lerp(sinVal, triVal, t);
            } else if (shapeVal <= 0.6666f) {
                float t = (shapeVal - 0.3333f) / 0.3333f;
                targetVal = lerp(triVal, sawVal, t);
            } else {
                float t = (shapeVal - 0.6666f) / 0.3333f;
                targetVal = lerp(sawVal, sqVal, t);
            }
            s = lerp(rawSample, targetVal, std::min(1.0f, shapeVal * 1.2f));
        }

        // 5. Additive Harmonics (2nd and 3rd harmonics)
        float h2Val = harmonic2.value * 0.01f;
        if (std::abs(h2Val) > 0.001f) {
            s += h2Val * std::sin(pWarped * 4.0f * (float)M_PI);
        }

        float h3Val = harmonic3.value * 0.01f;
        if (std::abs(h3Val) > 0.001f) {
            s += h3Val * std::sin(pWarped * 6.0f * (float)M_PI);
        }

        // 6. Sine Wavefolding / Drive Saturation
        float foldVal = fold.value * 0.01f;
        if (foldVal > 0.001f) {
            float foldDrive = 1.0f + foldVal * 3.5f;
            s = std::sin(s * foldDrive * ((float)M_PI * 0.5f));
        }

        return std::clamp(s, -1.0f, 1.0f);
    }

    bool isInitialized = false;

    Param& wavetableParam = addParam({
        .key = "wavetable",
        .label = "Wavetable",
        .string = wtName,
        .value = 0.0f,
        .min = 0.0f,
        .max = 0.0f,
        .step = 1.0f,
        .onUpdate = [](void* ctx, float val) {
            auto* self = static_cast<KickWavetable2*>(ctx);
            int fileCount = self->wavetable.fileBrowser.count;
            if (fileCount <= 0) return;

            if (self->isSingleCycleFile()) {
                int fileIdx = (int)val;
                fileIdx = (fileIdx % fileCount + fileCount) % fileCount;
                self->wavetable.open(fileIdx + 1, true);
                self->currentMorphVal = 1.0f;
                std::string fname = self->wavetable.fileBrowser.getFileWithoutExtension(fileIdx + 1);
                snprintf(self->wtName, sizeof(self->wtName), "%s", fname.c_str());
            } else {
                int totalVal = (int)val;
                int totalMax = fileCount * 64;
                totalVal = (totalVal % totalMax + totalMax) % totalMax;

                int fileIdx = totalVal / 64;
                int morphIdx = (totalVal % 64) + 1;

                self->wavetable.open(fileIdx + 1, true);

                int validMorph = std::clamp(morphIdx, 1, 64);
                self->currentMorphVal = (float)validMorph;
                std::string fname = self->wavetable.fileBrowser.getFileWithoutExtension(fileIdx + 1);
                snprintf(self->wtName, sizeof(self->wtName), "%s #%d", fname.c_str(), validMorph);
            }
            // Option 2: Potentiometer values remain at their physical dial positions; no auto-overwriting!
        },
        .graph = [](void* ctx, float phase) {
            auto* self = static_cast<KickWavetable2*>(ctx);
            return self->synthesizeParametricSample(phase);
        }
    });

    Param& pitchModShape = addParam({
        .key = "pitchModShape",
        .label = "Pitch Shape",
        .unit = "%",
        .value = 50.0f,
        .min = 0.0f,
        .max = 100.0f,
        .step = 1.0f,
        .graph = [](void* ctx, float phase) {
            auto* self = static_cast<KickWavetable2*>(ctx);
            float shapeNorm = self->pitchModShape.value * 0.01f;
            float rawEnv = std::exp(-phase * 4.0f);
            float pitchEnv = std::pow(rawEnv, 1.0f + shapeNorm * 5.0f);
            return pitchEnv * 2.0f - 1.0f;
        }
    });

    Param& baseFreq = addParam({ .key = "baseFreq", .label = "Sub Freq", .unit = "Hz", .value = 52.0f, .min = 30.0f, .max = 100.0f, .step = 1.0f });
    Param& duration = addParam({ .key = "duration", .label = "Duration", .unit = "ms", .value = 350.0f, .min = 50.0f, .max = 1500.0f, .step = 10.0f });
    Param& fmDepth = addParam({ .key = "fmDepth", .label = "FM Depth", .unit = "%", .value = 35.0f, .min = 0.0f, .max = 100.0f, .step = 1.0f });
    Param& drive = addParam({ .key = "drive", .label = "Drive", .unit = "%", .value = 35.0f, .min = 0.0f, .max = 100.0f, .step = 1.0f });

    Param& waveShape = addParam({
        .key = "waveShape",
        .label = "Wave Shape",
        .unit = "%",
        .value = 0.0f,
        .min = 0.0f,
        .max = 100.0f,
        .step = 1.0f
    });

    Param& harmonic2 = addParam({
        .key = "harmonic2",
        .label = "Harmonic 2",
        .unit = "%",
        .value = 0.0f,
        .min = -100.0f,
        .max = 100.0f,
        .step = 1.0f,
        .type = VALUE_CENTERED
    });

    Param& harmonic3 = addParam({
        .key = "harmonic3",
        .label = "Harmonic 3",
        .unit = "%",
        .value = 0.0f,
        .min = -100.0f,
        .max = 100.0f,
        .step = 1.0f,
        .type = VALUE_CENTERED
    });

    Param& skew = addParam({
        .key = "skew",
        .label = "Wave Skew",
        .unit = "%",
        .value = 50.0f,
        .min = 5.0f,
        .max = 95.0f,
        .step = 1.0f
    });

    Param& fold = addParam({
        .key = "fold",
        .label = "Wave Fold",
        .unit = "%",
        .value = 0.0f,
        .min = 0.0f,
        .max = 100.0f,
        .step = 1.0f
    });

    Param& phaseOffset = addParam({
        .key = "phaseOffset",
        .label = "Phase Shift",
        .unit = "%",
        .value = 0.0f,
        .min = 0.0f,
        .max = 100.0f,
        .step = 1.0f
    });

    std::string findKickWavetableFolder()
    {
        std::vector<std::string> candidates = {
#ifdef AUDIO_FOLDER
            AUDIO_FOLDER + "/wavetables_kick",
#endif
            "data/audio/wavetables_kick",
            "../data/audio/wavetables_kick",
            "../../data/audio/wavetables_kick"
        };
        std::error_code ec;
        for (const auto& path : candidates) {
            if (std::filesystem::exists(path, ec) && std::filesystem::is_directory(path, ec)) {
                return path;
            }
        }
        return "data/audio/wavetables_kick";
    }

    KickWavetable2(const float sampleRate = 44100.0f)
        : EngineBase(Drum, "KickWavetable2", params)
        , sampleRate(sampleRate)
    {
        std::string kickFolder = findKickWavetableFolder();
        wavetable.fileBrowser.openFolder(kickFolder);
        if (wavetable.fileBrowser.count > 0) {
            wavetable.open(1, true);
        }

        int fileCount = wavetable.fileBrowser.count;
        if (isSingleCycleFile()) {
            wavetableParam.max = std::max(0.0f, (float)(fileCount - 1));
        } else {
            wavetableParam.max = std::max(0.0f, (float)(fileCount * 64 - 1));
        }
        if (fileCount > 0) {
            wavetableParam.set(0.0f);
        }
        isInitialized = true;
    }

    bool saveCurrentWavetableFrame(std::string folderPath = "")
    {
        if (folderPath.empty()) {
#ifdef AUDIO_FOLDER
            folderPath = AUDIO_FOLDER + "/wavetables_kick";
#else
            folderPath = "data/audio/wavetables_kick";
#endif
        }

        if (wavetable.fileBrowser.count <= 0) {
            logError("No wavetable file available to save.");
            return false;
        }

        std::error_code ec;
        std::filesystem::create_directories(folderPath, ec);

        int totalVal = (int)wavetableParam.value;
        int fileCount = wavetable.fileBrowser.count;
        int fileIdx = std::clamp(totalVal / 64, 0, fileCount - 1);
        int morphIdx = (totalVal % 64) + 1;

        std::string rawName = wavetable.fileBrowser.getFileWithoutExtension(fileIdx + 1);
        char outFilename[512];
        snprintf(outFilename, sizeof(outFilename), "%s/%s_morph%d.wav", folderPath.c_str(), rawName.c_str(), morphIdx);

        float cycleLen = getActiveCycleSampleCount();
        int numSamples = static_cast<int>(cycleLen > 0.0f ? cycleLen : 2048.0f);
        std::vector<float> frameBuffer(numSamples);

        for (int i = 0; i < numSamples; ++i) {
            float phase = (float)i / (float)numSamples;
            frameBuffer[i] = synthesizeParametricSample(phase);
        }

        SF_INFO sfinfo;
        memset(&sfinfo, 0, sizeof(sfinfo));
        sfinfo.samplerate = 44100;
        sfinfo.channels = 1;
        sfinfo.format = SF_FORMAT_WAV | SF_FORMAT_PCM_16;

        SNDFILE* outfile = sf_open(outFilename, SFM_WRITE, &sfinfo);
        if (outfile) {
            sf_write_float(outfile, frameBuffer.data(), numSamples);
            sf_close(outfile);
            logInfo("Saved single morph wavetable frame to %s", outFilename);
            return true;
        } else {
            logError("Failed to open wavetable file for writing: %s", outFilename);
            return false;
        }
    }

    void trigger(float vel = 1.0f)
    {
        noteOnImpl(60, vel);
    }

    void noteOnImpl(uint8_t note, float _velocity)
    {
        (void)note;
        velocity = _velocity;

        if (!isBodyMuted) {
            carrierPhase = 0.0f;
            modulatorPhase = 0.0f;
            modulationEnvelope = 1.0f;
            compressionEnv = 0.0f;

            int totalSamples = static_cast<int>(sampleRate * (duration.value * 0.001f));
            envelopAmp.reset(totalSamples);
        }
    }

    void noteOffImpl(uint8_t note)
    {
        (void)note;
    }

    float sampleImpl()
    {
        float envAmp = envelopAmp.next();
        float kickOut = 0.0f;

        // 1. Generate Main Kick Body Sample
        if (envAmp > 0.0001f) {
            modulationEnvelope *= Math::exp(-1.0f / (sampleRate * 0.025f));

            // Pitch mod shape curve
            float shapeNorm = pitchModShape.value * 0.01f;
            float pitchEnv = std::pow(modulationEnvelope, 1.0f + shapeNorm * 5.0f);

            float rootFreq = baseFreq.value;
            float currentFreq = rootFreq * (1.0f + pitchEnv * 2.5f);

            float modulatorFreq = currentFreq * 1.5f;
            float modulatorSignal = Math::fastSin2(PI_X2 * modulatorPhase);
            modulatorPhase += modulatorFreq / sampleRate;
            if (modulatorPhase > 1.0f) modulatorPhase -= 1.0f;

            float fmIntensity = pct(fmDepth) * 0.75f * pitchEnv;
            float cycleLen = getActiveCycleSampleCount();
            float phaseInc = (currentFreq / sampleRate) * cycleLen;
            float fmScale = (cycleLen / 2048.0f);
            carrierPhase += phaseInc + (modulatorSignal * fmIntensity * 20.0f * fmScale);

            while (carrierPhase >= cycleLen) carrierPhase -= cycleLen;
            while (carrierPhase < 0.0f) carrierPhase += cycleLen;

            float phaseNorm = carrierPhase / cycleLen;
            float sig = synthesizeParametricSample(phaseNorm);

            kickOut = sig * envAmp;
        }

        // 2. Apply Drive & Internal Glue Compressor
        float out = kickOut;
        if (drive.value > 0.0f) {
            out = applyDrive(out, pct(drive) * 3.0f);
        }
        out = applyCompression2(out, 0.65f, compressionEnv);

        return out * velocity;
    }
};
