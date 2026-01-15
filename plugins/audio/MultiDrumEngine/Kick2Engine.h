#pragma once

#include "plugins/audio/MultiDrumEngine/DrumEngine.h"
#include "audio/effects/applyDrive.h"
#include "audio/effects/applyCompression.h"
#include <cmath>

class Kick2Engine : public DrumEngine {
protected:
    float velocity = 1.0f;
    float phase = 0.0f;
    bool active = false;

    // Internal state for smoothing/filtering
    float pitchEnv = 0.0f;
    float ampEnv = 1.0f;
    float clickLevel = 0.0f;

    // --- Parameters (10 total) ---
    // Tone
    Val& baseFreq = val(45.0f, "FREQ", { .label = "Sub Freq", .min = 30.0, .max = 80.0, .unit = "Hz" });
    Val& decay = val(40, "DECAY", { .label = "Decay", .unit = "%" });
    
    // Pitch Envelope (The "Knock")
    Val& sweepDepth = val(70, "SWEEP_DEPTH", { .label = "Sweep", .unit = "%" });
    Val& sweepSpeed = val(30, "SWEEP_SPEED", { .label = "Punch", .unit = "%" });
    
    // Character
    Val& click = val(20, "CLICK", { .label = "Click", .unit = "%" });
    Val& noise = val(5, "NOISE", { .label = "Air", .unit = "%" });
    
    // Saturation & Fatness
    Val& drive = val(30, "DRIVE", { .label = "Drive", .unit = "%" });
    Val& compression = val(20, "COMP", { .label = "Glue", .unit = "%" });
    
    // Master
    Val& tone = val(50, "TONE", { .label = "Tone", .unit = "%" }); // LPF on the drive
    Val& pitch = val(0.0f, "PITCH", { .label = "Pitch", .type = VALUE_CENTERED, .min = -12, .max = 12 });

public:
    Kick2Engine(AudioPlugin::Props& p, AudioPlugin::Config& c)
        : DrumEngine(p, c, "Kick2")
    {
        initValues();
    }

    void noteOn(uint8_t note, float _velocity, void* = nullptr) override
    {
        DrumEngine::noteOn(note, _velocity);
        velocity = _velocity;
        phase = 0.0f;
        pitchEnv = 1.0f;
        ampEnv = 1.0f;
        clickLevel = 1.0f;
        active = true;
    }

    void sampleOn(float* buf, float envAmpVal, int sc, int ts) override
    {
        if (!active) return;

        float t = (float)sc / ts;
        
        // 1. Pitch Envelope Calculation
        // Fast exponential decay for the pitch sweep
        float sweepDecay = 0.005f + (1.0f - sweepSpeed.pct()) * 0.05f;
        pitchEnv *= expf(-1.0f / (props.sampleRate * sweepDecay));

        // 2. Oscillators
        float f0 = baseFreq.get() * powf(2.0f, (pitch.get()) / 12.0f);
        // The sweep starts high and drops to baseFreq
        float currentFreq = f0 + (sweepDepth.pct() * 600.0f * pitchEnv);
        
        // Wrap phase
        phase += currentFreq / props.sampleRate;
        if (phase > 1.0f) phase -= 1.0f;
        
        float sine = sinf(2.0f * M_PI * phase);
        
        // 3. Transient & Noise
        float white = (props.lookupTable->getNoise() * 2.0f - 1.0f);
        float clickPart = white * clickLevel * click.pct();
        // Snap the click level down very fast
        clickLevel *= expf(-1.0f / (props.sampleRate * 0.002f));

        // 4. Combine Body and Transient
        float output = sine + clickPart;
        
        // Add subtle high-end "air" noise during the whole hit
        output += white * noise.pct() * 0.1f * envAmpVal;

        // 5. Processing "The Fatness"
        // Apply Drive (Soft clipping / Saturation)
        if (drive.get() > 0.0f) {
            output = applyDrive(output, drive.pct() * 2.0f, props.lookupTable);
        }

        // Simple Low Pass to shape the "Tone"
        float lpfCutoff = 0.1f + tone.pct() * 0.8f;
        static float lpState = 0.0f;
        lpState += lpfCutoff * (output - lpState);
        output = lpState;

        // Apply Glue (Compression)
        if (compression.get() > 0.0f) {
            output = applyCompression(output, compression.pct());
        }

        buf[track] = output * envAmpVal * velocity;
    }

    void sampleOff(float* buf) override 
    {
        // Kick usually stops immediately or relies on release
        active = false;
    }
};