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

class KickWavetable : public EngineBase<KickWavetable> {
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

    float synthesizeParametricSample(float phase)
    {
        float pOffset = phaseOffset.value * 0.01f;
        float p = phase + pOffset;
        p = p - std::floor(p);

        float skewNorm = std::clamp(skew.value * 0.01f, 0.05f, 0.95f);
        float pWarped = 0.0f;
        if (p < skewNorm) {
            pWarped = 0.5f * (p / skewNorm);
        } else {
            pWarped = 0.5f + 0.5f * ((p - skewNorm) / (1.0f - skewNorm));
        }

        float shapeVal = waveShape.value * 0.01f;
        float s = 0.0f;

        float sinVal = std::sin(pWarped * 2.0f * (float)M_PI);
        float triVal = (pWarped < 0.5f) ? (4.0f * pWarped - 1.0f) : (3.0f - 4.0f * pWarped);
        float sawVal = 1.0f - 2.0f * pWarped;
        float sqVal = (pWarped < 0.5f) ? 1.0f : -1.0f;

        if (shapeVal <= 0.3333f) {
            float t = shapeVal / 0.3333f;
            s = lerp(sinVal, triVal, t);
        } else if (shapeVal <= 0.6666f) {
            float t = (shapeVal - 0.3333f) / 0.3333f;
            s = lerp(triVal, sawVal, t);
        } else {
            float t = (shapeVal - 0.6666f) / 0.3333f;
            s = lerp(sawVal, sqVal, t);
        }

        float h2Val = harmonic2.value * 0.01f;
        if (std::abs(h2Val) > 0.001f) {
            s += h2Val * std::sin(pWarped * 4.0f * (float)M_PI);
        }

        float h3Val = harmonic3.value * 0.01f;
        if (std::abs(h3Val) > 0.001f) {
            s += h3Val * std::sin(pWarped * 6.0f * (float)M_PI);
        }

        float foldVal = fold.value * 0.01f;
        if (foldVal > 0.001f) {
            float foldDrive = 1.0f + foldVal * 3.5f;
            s = std::sin(s * foldDrive * ((float)M_PI * 0.5f));
        }

        return std::clamp(s, -1.0f, 1.0f);
    }

    void analyzeAndFitWaveform()
    {
        float cycleLen = getActiveCycleSampleCount();
        if (cycleLen <= 0.0f) return;

        int numSamples = 256;
        std::vector<float> wavSamples(numSamples);
        float maxAbs = 1e-5f;

        for (int i = 0; i < numSamples; ++i) {
            float pos = ((float)i / (float)numSamples) * cycleLen;
            wavSamples[i] = readWaveformSample(currentMorphVal, pos);
            if (std::abs(wavSamples[i]) > maxAbs) {
                maxAbs = std::abs(wavSamples[i]);
            }
        }

        for (int i = 0; i < numSamples; ++i) {
            wavSamples[i] /= maxAbs;
        }

        float a1 = 0.0f, b1 = 0.0f;
        float a2 = 0.0f, b2 = 0.0f;
        float a3 = 0.0f, b3 = 0.0f;

        for (int i = 0; i < numSamples; ++i) {
            float phase = (float)i / (float)numSamples * 2.0f * (float)M_PI;
            a1 += wavSamples[i] * std::cos(phase);
            b1 += wavSamples[i] * std::sin(phase);
            a2 += wavSamples[i] * std::cos(2.0f * phase);
            b2 += wavSamples[i] * std::sin(2.0f * phase);
            a3 += wavSamples[i] * std::cos(3.0f * phase);
            b3 += wavSamples[i] * std::sin(3.0f * phase);
        }
        a1 *= (2.0f / numSamples);
        b1 *= (2.0f / numSamples);
        a2 *= (2.0f / numSamples);
        b2 *= (2.0f / numSamples);
        a3 *= (2.0f / numSamples);
        b3 *= (2.0f / numSamples);

        float mag1 = std::sqrt(a1 * a1 + b1 * b1);
        float mag2 = std::sqrt(a2 * a2 + b2 * b2);
        float mag3 = std::sqrt(a3 * a3 + b3 * b3);

        float phaseRad = std::atan2(a1, b1);
        float pOffsetNorm = std::fmod(phaseRad / (2.0f * (float)M_PI) + 1.0f, 1.0f);
        phaseOffset.value = std::round(pOffsetNorm * 100.0f);

        int peakIdx = 0;
        float maxVal = -2.0f;
        for (int i = 0; i < numSamples; ++i) {
            if (wavSamples[i] > maxVal) {
                maxVal = wavSamples[i];
                peakIdx = i;
            }
        }
        float peakPosNorm = (float)peakIdx / (float)numSamples;
        float relativePeak = peakPosNorm - pOffsetNorm;
        relativePeak = relativePeak - std::floor(relativePeak);
        float skewVal = std::clamp(relativePeak * 100.0f, 5.0f, 95.0f);
        skew.value = std::round(skewVal);

        float h2Ratio = (mag1 > 1e-4f) ? (mag2 / mag1) : 0.0f;
        float h2Sign = (b2 * b1 + a2 * a1 >= 0.0f) ? 1.0f : -1.0f;
        harmonic2.value = std::round(std::clamp(h2Sign * h2Ratio * 100.0f, -100.0f, 100.0f));

        float h3Ratio = (mag1 > 1e-4f) ? (mag3 / mag1) : 0.0f;
        float h3Sign = (b3 * b1 + a3 * a1 >= 0.0f) ? 1.0f : -1.0f;
        harmonic3.value = std::round(std::clamp(h3Sign * h3Ratio * 100.0f, -100.0f, 100.0f));

        int zeroCrossings = 0;
        for (int i = 0; i < numSamples - 1; ++i) {
            if ((wavSamples[i] >= 0.0f && wavSamples[i + 1] < 0.0f) || (wavSamples[i] < 0.0f && wavSamples[i + 1] >= 0.0f)) {
                zeroCrossings++;
            }
        }
        float foldEst = 0.0f;
        if (zeroCrossings > 2) {
            foldEst = std::clamp((float)(zeroCrossings - 2) * 25.0f, 0.0f, 100.0f);
        }
        fold.value = std::round(foldEst);

        float rms = 0.0f;
        for (int i = 0; i < numSamples; ++i) {
            rms += wavSamples[i] * wavSamples[i];
        }
        rms = std::sqrt(rms / numSamples);

        float shapeEst = 0.0f;
        if (rms < 0.62f) {
            shapeEst = std::clamp((1.0f - (rms - 0.577f) / (0.707f - 0.577f)) * 33.0f, 0.0f, 33.0f);
        } else if (rms < 0.8f) {
            shapeEst = std::clamp(33.0f + ((rms - 0.62f) / 0.18f) * 33.0f, 33.0f, 66.0f);
        } else {
            shapeEst = std::clamp(66.0f + ((rms - 0.8f) / 0.2f) * 34.0f, 66.0f, 100.0f);
        }

        float bestShape = shapeEst;
        float minError = 1e9f;
        for (float testShape = 0.0f; testShape <= 100.0f; testShape += 5.0f) {
            waveShape.value = testShape;
            float err = 0.0f;
            for (int i = 0; i < numSamples; ++i) {
                float phase = (float)i / (float)numSamples;
                float synth = synthesizeParametricSample(phase);
                float diff = wavSamples[i] - synth;
                err += diff * diff;
            }
            if (err < minError) {
                minError = err;
                bestShape = testShape;
            }
        }
        waveShape.value = std::round(bestShape);
    }

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
            auto* self = static_cast<KickWavetable*>(ctx);
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
            if (self->isInitialized) {
                self->analyzeAndFitWaveform();
            }
        },
        .graph = [](void* ctx, float phase) {
            auto* self = static_cast<KickWavetable*>(ctx);
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
            auto* self = static_cast<KickWavetable*>(ctx);
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

    KickWavetable(const float sampleRate = 44100.0f)
        : EngineBase(Drum, "KickWavetable", params)
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
        analyzeAndFitWaveform();
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
