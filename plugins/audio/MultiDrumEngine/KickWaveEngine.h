#pragma once

#include "helpers/math.h"
#include "plugins/audio/MultiDrumEngine/DrumEngine.h"
#include "audio/Wavetable.h"
#include "audio/KickEnvTableGenerator.h"
#include "audio/MultiFx.h"

class KickWaveEngine : public DrumEngine {
protected:
    float velocity = 1.0f;

    MultiFx multiFx;
    MultiFx multiFx2;

    Wavetable wavetable;
    KickEnvTableGenerator kickEnv;

public:
    // --- Frequency & Pitch ---
    GraphPointFn frequencyGraph = [&](float index) { return *kickEnv.sample(&index); };
    Val& frequencyModulation = val(10.0f, "ENVELOPE_SHAPE", { .label = "Freq. mod.", .type = VALUE_BASIC, .step = 0.05f, .unit = "%", .graph = frequencyGraph }, [&](auto p) {
        p.val.setFloat(p.value);
        kickEnv.setMorph(p.val.pct());
    });

    Val& pitchOffset = val(0, "PITCH", { .label = "Pitch", .type = VALUE_CENTERED, .min = -24, .max = 24, .incType = INC_ONE_BY_ONE });

    // --- Wavetable Controls ---
    Val& wave = val(0, "WAVE", { .label = "Wave", VALUE_STRING }, [&](auto p) {
        p.val.setFloat(p.value);
        int position = (int)p.val.get();
        wavetable.open(position, false);
        p.val.setString(wavetable.fileBrowser.getFileWithoutExtension(position));
    });

    GraphPointFn graphWave = [&](auto index) { return *wavetable.sample(&index); };
    Val& waveEdit = val(1.0f, "WAVE_EDIT", { .label = "Wave Morph", .type = VALUE_STRING, .min = 1.0, .max = ZIC_WAVETABLE_WAVEFORMS_COUNT, .graph = graphWave }, [&](auto p) {
        p.val.setFloat(p.value);
        wavetable.morph((int)p.val.get() - 1);
        p.val.setString(std::to_string((int)p.val.get()) + "/" + std::to_string(ZIC_WAVETABLE_WAVEFORMS_COUNT));
    });
    
    // Punch Drive saturates the kick based on the Envelope (higher pitch = more drive)
    // MODIFIED: Increased range for a way more intense hardcore effect
    Val& punchDrive = val(20, "PUNCH_DRIVE", { .label = "Punch Drive", .unit = "%" });

    // Hard Clip for the industrial grit
    Val& clipLevel = val(0.0f, "HARD_CLIP", { .label = "Hard Clip", .unit = "%" });

    // --- Multi FX ---
    Val& fxType = val(0, "FX_TYPE", { .label = "FX type", .type = VALUE_STRING, .max = MultiFx::FXType::FX_COUNT - 1 }, multiFx.setFxType);
    Val& fxAmount = val(0, "FX_AMOUNT", { .label = "FX edit", .unit = "%" });

    Val& fx2Type = val(0, "FX2_TYPE", { .label = "FX2 type", .type = VALUE_STRING, .max = MultiFx::FXType::FX_COUNT - 1 }, multiFx2.setFxType);
    Val& fx2Amount = val(0, "FX2_AMOUNT", { .label = "FX2 edit", .unit = "%" });

    KickWaveEngine(AudioPlugin::Props& p, AudioPlugin::Config& c)
        : DrumEngine(p, c, "KickWav")
        , multiFx(props.sampleRate)
        , multiFx2(props.sampleRate)
    {
        initValues();
    }

    float baseFrequency = 1.0f;

    void sampleOn(float* buffer, float envelopeAmplitude, int sampleCounter, int totalSamples) override
    {
        float normalizedTime = (float)sampleCounter / totalSamples;
        
        // 1. Get Envelope (0.0 to 1.0)
        float envelopeValue = kickEnv.next(normalizedTime);
        
        // 2. Frequency Logic
        float modulatedFrequency = baseFrequency + envelopeValue;

        // 3. Wavetable Synthesis
        float output = wavetable.sample(&wavetable.sampleIndex, modulatedFrequency);

        // 4. THE PUNCH: Non-linear processing
        // Envelope-Driven Saturation: The drive is stronger at the start of the kick (when envelopeValue is high)
        if (punchDrive.pct() > 0.0f) {
            // MODIFIED: Massive multiplier (24x instead of 4x) for hardcore saturation
            // We also make the drive response more aggressive by squaring the envelope influence
            float driveIntensity = punchDrive.pct() * 24.0f * (envelopeValue * envelopeValue);
            
            // Apply a high-gain soft-clipping stage
            // This will squash the wave into a near-square when pushed
            output = output * (1.0f + driveIntensity);
            output = output / (1.0f + fabsf(output));
        }

        // Hard Clipping for extra harmonics
        if (clipLevel.pct() > 0.0f) {
            float threshold = 1.0f - (clipLevel.pct() * 0.95f);
            if (output > threshold) output = threshold;
            if (output < -threshold) output = -threshold;
            output /= threshold; // Gain compensation
        }

        // 5. Final FX Chain
        output = multiFx.apply(output * envelopeAmplitude, fxAmount.pct());
        output = multiFx2.apply(output, fx2Amount.pct());

        buffer[track] = output * velocity;
    }

    void sampleOff(float* buffer) override
    {
        float output = buffer[track];
        output = multiFx.apply(output, fxAmount.pct());
        buffer[track] = output;
    }

    uint8_t baseNote = 60 + 12;
    void noteOn(uint8_t note, float _velocity, void* = nullptr) override
    {
        DrumEngine::noteOn(note, _velocity);
        velocity = _velocity;
        
        wavetable.sampleIndex = 0.0f; // Reset phase for consistent hardcore punch
        baseFrequency = powf(2.0f, ((float)note - (float)baseNote + pitchOffset.get()) / 12.0f);
    }
};