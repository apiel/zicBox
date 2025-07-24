#pragma once
#include "plugins/audio/MultiDrumEngine/DrumEngine.h"
#include "plugins/audio/utils/WavetableGenerator2.h"
#include "plugins/audio/utils/effects/applyCompression.h"
#include "plugins/audio/utils/effects/applyWaveshape.h"
#include "plugins/audio/utils/effects/applyDrive.h"
#include "plugins/audio/utils/effects/applyReverb.h"
#include "plugins/audio/utils/filterArray.h"

class BassEngine : public DrumEngine {
protected:
    EffectFilterArray<2> filter;

    float velocity = 1.0f;

    WavetableInterface* wave = nullptr;
    WavetableGenerator waveform;
#define BASS_WAVEFORMS_COUNT 6
    struct WaveformType {
        std::string name;
        WavetableInterface* wave;
        uint8_t indexType = 0;
    } waveformTypes[BASS_WAVEFORMS_COUNT] = {
        { "Sine", &waveform, (uint8_t)WavetableGenerator::Type::Sine },
        { "Sawtooth", &waveform, (uint8_t)WavetableGenerator::Type::Saw },
        { "Square", &waveform, (uint8_t)WavetableGenerator::Type::Square },
        { "Pulse", &waveform, (uint8_t)WavetableGenerator::Type::Pulse },
        { "Triangle", &waveform, (uint8_t)WavetableGenerator::Type::Triangle },
        { "FMSquare", &waveform, (uint8_t)WavetableGenerator::Type::FMSquare },
    };

    static constexpr int REVERB_BUFFER_SIZE = 48000; // 1 second buffer at 48kHz
    float reverbBuffer[REVERB_BUFFER_SIZE] = { 0.0f };
    int reverbIndex = 0;

    float applyEffects(float input)
    {
        float output = input;

        output = applyReverb(output, reverb.pct(), reverbBuffer, reverbIndex, REVERB_BUFFER_SIZE);
        output = doCompression(output);
        output = applyDrive(output, drive.pct(), props.lookupTable);
        return output;
    }

    float doCompression(float input)
    {
        if (compression.pct() == 0.5f) {
            return input;
        }
        if (compression.pct() > 0.5f) {
            float amount = compression.pct() * 2 - 1.0f;
            return applyCompression(input, amount);
        }
        float amount = 1 - compression.pct() * 2;
        return applyWaveshapeLut(input, amount, props.lookupTable);
    }

public:
    Val& pitch = val(0, "PITCH", { "Pitch", VALUE_CENTERED, .min = -24, .max = 24, .skipJumpIncrements = true });
    Val& bend = val(0.4f, "BEND", { "Bend", .unit = "%" });
    Val& cutoff = val(50.0, "CUTOFF", { "Cutoff", .unit = "%" }, [&](auto p) {
        p.val.setFloat(p.value);
        float cutoffValue = 0.85 * p.val.pct() + 0.1;
        filter.setCutoff(cutoffValue);
    });
    Val& resonance = val(0.0, "RESONANCE", { "Resonance", .unit = "%" }, [&](auto p) {
        p.val.setFloat(p.value);
        // float res = 0.95 * (1.0 - std::pow(1.0 - p.val.pct(), 3));
        float res = 0.95 * (1.0 - std::pow(1.0 - p.val.pct(), 2));
        filter.setResonance(res);
    });
    Val& clipping = val(0.0, "GAIN_CLIPPING", { "Gain Clipping", .unit = "%" });
    Val& drive = val(0.0f, "DRIVE", { "Drive", .unit = "%" });
    Val& compression = val(0.0f, "COMPRESSION", { "Compression", .type = VALUE_CENTERED, .min = -100.0, .max = 100.0, .step = 1.0, .unit = "%" });
    Val& reverb = val(0.3f, "REVERB", { "Reverb", .unit = "%" });
    Val& waveformType = val(1.0f, "WAVEFORM_TYPE", { "Waveform", VALUE_STRING, .max = BASS_WAVEFORMS_COUNT - 1 }, [&](auto p) {
        float current = p.val.get();
        p.val.setFloat(p.value);
        if (wave && current == p.val.get()) {
            return;
        }
        WaveformType type = waveformTypes[(int)p.val.get()];
        p.val.setString(type.name);
        wave = type.wave;
        waveform.setType((WavetableGenerator::Type)type.indexType);
        // shape.set(waveform.modulation * 1000.0f);
    });
    Val& shape = val(0.0f, "SHAPE", { "Shape", VALUE_BASIC, .unit = "%" }, [&](auto p) {
        p.val.setFloat(p.value);
        waveform.setMorph(p.val.pct());
    });

    BassEngine(AudioPlugin::Props& p, AudioPlugin::Config& c)
        : DrumEngine(p, c, "Bass")
        , waveform(props.lookupTable, props.sampleRate)
    {
        initValues();
    }

    float scaledClipping = 0.0f;
    float freq = 1.0f;

    float sampleIndex = 0.0f;
    void sampleOn(float* buf, float envAmp, int sampleCounter, int totalSamples) override
    {
        // float out = wave->sample(&wavetable.sampleIndex, freq);

        float t = float(sampleCounter) / totalSamples;
        float bendAmt = bend.pct();
        float bendedFreq = freq * (1.f - bendAmt * t);
        float out = wave->sample(&sampleIndex, bendedFreq);
        out = out * velocity * envAmp;

        filter.setCutoff(0.85 * cutoff.pct() * envAmp + 0.1);
        filter.setSampleData(out, 0);
        filter.setSampleData(filter.lp[0], 1);
        out = filter.lp[1];

        out = range(out + out * clipping.pct() * 8, -1.0f, 1.0f);
        out = applyEffects(out);
        buf[track] = out;
    }

    void sampleOff(float* buf) override
    {
        buf[track] = applyReverb(buf[track], reverb.pct(), reverbBuffer, reverbIndex, REVERB_BUFFER_SIZE);
    }

    // Higher base note is, lower pitch will be
    //
    // Usualy our base note is 60
    // but since we want a bass sound we want to remove one octave (12 semitones)
    // So we send note 60, we will play note 48...
    uint8_t baseNote = 60 + 12;
    void noteOn(uint8_t note, float _velocity, void* = nullptr) override
    {
        velocity = _velocity;
        sampleIndex = 0;
        freq = pow(2, ((note - baseNote + pitch.get()) / 12.0));
    }
};
