#pragma once

#include <thread>

#include "zicRack/studio.h"
#include "helpers/clamp.h"
#include "helpers/midiNote.h"

// Helper to trigger a non-blocking note preview (noteOn -> wait -> noteOff)
void triggerPreview(Track& trk, int note, float velocity, int durationMs = 200)
{
    if (studio.isPlaying) return;
    {
        std::lock_guard<std::mutex> lock(studio.audioMutex);
        trk.engine->noteOn(note, velocity);
    }
    std::thread([&trk, note, durationMs]() {
        std::this_thread::sleep_for(std::chrono::milliseconds(durationMs));
        std::lock_guard<std::mutex> lock(studio.audioMutex);
        trk.engine->noteOff(note);
    }).detach();
}

void saveClip(Track& trk, int clipIdx)
{
    Clip& c = trk.clips[clipIdx];
    c.paramValues.clear();
    Param* params = trk.engine->getParams();
    for (size_t i = 0; i < trk.engine->getParamCount(); i++) {
        c.paramValues.push_back(params[i].value);
    }
    c.saved = true;
    c.sequence = trk.sequence;
    trk.activeClipIdx = clipIdx;
}

void loadClip(Track& trk, int clipIdx)
{
    Clip& c = trk.clips[clipIdx];

    Param* params = trk.engine->getParams();
    for (size_t i = 0; i < c.paramValues.size(); i++) {
        params[i].set(c.paramValues[i]);
    }
    trk.sequence = c.sequence;
    trk.activeClipIdx = clipIdx;
}

void editStep(Step& step, StepEditMode mode, int scaled)
{
    if (mode == StepEditMode::EDIT_NOTE) step.note = CLAMP(step.note + scaled, 0, (int)MIDI_LAST_NOTE);
    else if (mode == StepEditMode::EDIT_VELO) step.velocity = CLAMP(step.velocity + (scaled * 0.05f), 0.0f, 1.0f);
    else if (mode == StepEditMode::EDIT_PROB) step.condition = CLAMP(step.condition + (scaled * 0.01f), 0.0f, 1.0f);
    else if (mode == StepEditMode::EDIT_LEN) step.len = CLAMP(step.len + (scaled * 0.5f), 0.5f, 64.5f);
}