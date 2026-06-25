#pragma once

#include <thread>

#include "zicXYv2/studio.h"
#include "helpers/clamp.h"
#include "helpers/midiNote.h"

// Helper to trigger a non-blocking note preview (noteOn -> wait -> noteOff)
void triggerPreview(Track& trk, int note, float velocity, int durationMs = 200)
{
    if (studio.isPlaying) return;
    auto engine = trk.engine.get();  // Capture raw pointer by value
    if (!engine) return;  // Safety check
    
    {
        std::lock_guard<std::mutex> lock(studio.audioMutex);
        engine->noteOn(note, velocity);
    }
    std::thread([engine, note, durationMs]() {
        std::this_thread::sleep_for(std::chrono::milliseconds(durationMs));
        try {
            std::lock_guard<std::mutex> lock(studio.audioMutex);
            if (engine) {  // Safety check: verify pointer is still valid
                engine->noteOff(note);
            }
        } catch (...) {
            // Silently ignore any errors during cleanup
        }
    }).detach();
}
