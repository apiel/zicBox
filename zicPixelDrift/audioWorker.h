#pragma once

#include "audio/engines/DriftKick.h"
#include "audio/engines/DriftSynth1.h"
#include "audio/engines/DriftSynth2.h"
#include "mixer.h"
#include "sequencer.h"
#include "audio/Scatter.h"
#include "audio/Compressor.h"

#include <alsa/asoundlib.h>
#include <atomic>
#include <iostream>

class AudioWorker {
public:
    std::atomic<bool> running { false };

    DriftKick kickEngine;
    DriftSynth1 synth1Engine;
    DriftSynth2 synth2Engine;
    Sequencer seqEngine;
    Mixer mixerEngine;
    Scatter scatter;
    Compressor compressor;

    bool isSynth1Muted = false;
    bool isSynth2Muted = false;

    AudioWorker(float sampleRate = 44100.0f)
        : kickEngine(sampleRate)
        , synth1Engine(sampleRate)
        , synth2Engine(sampleRate)
        , seqEngine(sampleRate)
        , mixerEngine(sampleRate)
        , compressor(sampleRate)
    {
    }

    void processAudioBlock(float* buffer, int numFrames)
    {
        double samplesPerStep = seqEngine.getSamplesPerStep();
        for (int i = 0; i < numFrames; ++i) {
            bool trigK = false, trigS1 = false, trigS2 = false;
            float vel = 1.0f;

            if (seqEngine.tick(trigK, trigS1, trigS2, vel)) {
                if (trigK) kickEngine.trigger(vel);
                if (trigS1 && !isSynth1Muted) synth1Engine.trigger();
                if (trigS2 && !isSynth2Muted) synth2Engine.trigger();
            }

            float kOut = kickEngine.sample();
            float s1Out = synth1Engine.sample();
            float s2Out = synth2Engine.sample();

            float mixOut = mixerEngine.process(
                kOut,
                s1Out, synth1Engine.delaySend.value,
                s2Out, synth2Engine.delaySend.value
            );

            mixOut = scatter.process(mixOut, samplesPerStep);
            mixOut = compressor.process(mixOut);

            buffer[i * 2 + 0] = mixOut;
            buffer[i * 2 + 1] = mixOut;
        }
    }
};
