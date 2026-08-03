#pragma once

#include <thread>

#include "audio/sequencer/SequenceUtils.h"
#include "helpers/clamp.h"
#include "helpers/midiNote.h"
#include "zicXYv2/studio.h"

// Helper to trigger a non-blocking note preview (noteOn -> wait -> noteOff)
void triggerPreview(Track& trk, int note, float velocity, int durationMs = 200)
{
    if (studio.isPlaying) return;
    auto engine = trk.engine.get(); // Capture raw pointer by value
    if (!engine) return; // Safety check

    {
        std::lock_guard<std::mutex> lock(studio.audioMutex);
        engine->noteOn(note, velocity);
    }
    std::thread([engine, note, durationMs]() {
        std::this_thread::sleep_for(std::chrono::milliseconds(durationMs));
        try {
            std::lock_guard<std::mutex> lock(studio.audioMutex);
            if (engine) { // Safety check: verify pointer is still valid
                engine->noteOff(note);
            }
        } catch (...) {
            // Silently ignore any errors during cleanup
        }
    }).detach();
}

void stretchTrackSequence(Track& trk, bool setLen = false)
{
    stretchSequence(trk.sequence, trk.genLen, trk.type == TRACK_TYPE_SYNTH, setLen);
}

void compressTrackSequence(Track& trk, bool setLen = false)
{
    compressSequence(trk.sequence, trk.genLen, trk.type == TRACK_TYPE_SYNTH, setLen);
}

void clearTrackSequence(Track& trk)
{
    for (int i = 0; i < SEQ_STEPS; i++)
        trk.sequence[i].active = false;
}

void runGeneration(int trkIdx)
{
    Track& trk = *studio.tracks[trkIdx];
    void (*generatorFunc)(std::vector<Step>&, float, float, float) = nullptr;
    if (trk.genEngine == 0) {
        generatorFunc = Generator::generateKick;
    } else if (trk.genEngine == 1) {
        generatorFunc = Generator::generateBass;
    } else if (trk.genEngine == 2) {
        generatorFunc = Generator::generateDrum;
    }

    if (generatorFunc != nullptr) {
        generatorFunc(trk.sequence, trk.genParams[0], trk.genParams[1], trk.genParams[2]);
        // Apply stretch logic based on display state (64 is native)
        if (trk.genLen == 32) stretchTrackSequence(trk);
        else if (trk.genLen == 16) {
            stretchTrackSequence(trk);
            stretchTrackSequence(trk);
        } else if (trk.genLen == 8) {
            stretchTrackSequence(trk);
            stretchTrackSequence(trk);
            stretchTrackSequence(trk);
        } else if (trk.genLen == 4) {
            stretchTrackSequence(trk);
            stretchTrackSequence(trk);
            stretchTrackSequence(trk);
            stretchTrackSequence(trk);
        } else if (trk.genLen == 128) {
            compressTrackSequence(trk);
        }

        studio.selTrack = trkIdx;
        studio.selStep = 0;
    }
}
