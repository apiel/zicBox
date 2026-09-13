#pragma once

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <vector>

#include "audio/engines/EngineBase.h"

struct Step {
    bool active = false;
    uint8_t note = 60;
    float velocity = 0.8f;
};

struct DrumTrack {
    const char* name = "Track";
    IEngine* engine = nullptr;
    bool muted = false;
    Step steps[16];
};

class SequenceBrain {
public:
    static constexpr int NUM_TRACKS = 4;
    static constexpr int NUM_STEPS = 16;

    DrumTrack tracks[NUM_TRACKS];

    float sampleRate = 44100.0f;
    float bpm = 125.0f;
    bool isPlaying = true;

    int currentStep = 0;
    uint64_t sampleCounter = 0;
    float samplesPerStep = 0.0f;

    int selectedTrack = 0;

    SequenceBrain(float sampleRate = 44100.0f)
        : sampleRate(sampleRate)
    {
        updateTiming();
        initDefaultPatterns();
    }

    void updateTiming()
    {
        // 16th notes: 4 steps per beat
        samplesPerStep = (sampleRate * 60.0f) / (bpm * 4.0f);
    }

    void setBpm(float newBpm)
    {
        bpm = std::clamp(newBpm, 60.0f, 240.0f);
        updateTiming();
    }

    void initDefaultPatterns()
    {
        for (int t = 0; t < NUM_TRACKS; ++t) {
            for (int s = 0; s < NUM_STEPS; ++s) {
                tracks[t].steps[s].active = false;
                tracks[t].steps[s].note = 60;
                tracks[t].steps[s].velocity = 0.8f;
            }
            tracks[t].muted = false;
        }

        // 4 Tracks: Snare, Clap, HiHat, Perc (No Kick!)
        tracks[0].name = "Snare";
        tracks[1].name = "Clap";
        tracks[2].name = "HiHat";
        tracks[3].name = "Perc";

        // Preset Techno/House Groove
        // Snare: 4, 12
        tracks[0].steps[4].active = true;
        tracks[0].steps[12].active = true;

        // Clap: 4, 12
        tracks[1].steps[4].active = true;
        tracks[1].steps[12].active = true;

        // HiHat: 2, 6, 10, 14 (off-beats)
        tracks[2].steps[2].active = true;
        tracks[2].steps[6].active = true;
        tracks[2].steps[10].active = true;
        tracks[2].steps[14].active = true;

        // Perc: 3, 7, 11, 15 (syncopated fills)
        tracks[3].steps[3].active = true;
        tracks[3].steps[7].active = true;
        tracks[3].steps[11].active = true;
        tracks[3].steps[15].active = true;
    }

    void generatePattern()
    {
        for (int t = 0; t < NUM_TRACKS; ++t) {
            for (int s = 0; s < NUM_STEPS; ++s) {
                float rnd = (float)rand() / RAND_MAX;
                if (t == 0) {
                    // Snare: 4 & 12 base + ghost syncopations
                    tracks[0].steps[s].active = (s == 4 || s == 12) || (rnd > 0.85f);
                } else if (t == 1) {
                    // Clap: 4 & 12 accents
                    tracks[1].steps[s].active = (s == 4 || s == 12) || (rnd > 0.9f);
                } else if (t == 2) {
                    // HiHat: off-beats or dense 16ths
                    tracks[2].steps[s].active = (s % 2 == 0) || (rnd > 0.65f);
                } else {
                    // Perc: syncopated fills
                    tracks[3].steps[s].active = (rnd > 0.7f);
                }
            }
        }
    }

    void toggleStep(int trackIdx, int stepIdx)
    {
        if (trackIdx >= 0 && trackIdx < NUM_TRACKS && stepIdx >= 0 && stepIdx < NUM_STEPS) {
            tracks[trackIdx].steps[stepIdx].active = !tracks[trackIdx].steps[stepIdx].active;
        }
    }

    void toggleMute(int trackIdx)
    {
        if (trackIdx >= 0 && trackIdx < NUM_TRACKS) {
            tracks[trackIdx].muted = !tracks[trackIdx].muted;
        }
    }

    void processSampleTick()
    {
        if (!isPlaying) return;

        sampleCounter++;
        if (sampleCounter >= (uint64_t)samplesPerStep) {
            sampleCounter = 0;
            currentStep = (currentStep + 1) % NUM_STEPS;

            // Trigger steps for unmuted tracks
            for (int t = 0; t < NUM_TRACKS; ++t) {
                if (!tracks[t].muted && tracks[t].steps[currentStep].active) {
                    if (tracks[t].engine) {
                        tracks[t].engine->noteOn(tracks[t].steps[currentStep].note, tracks[t].steps[currentStep].velocity);
                    }
                }
            }
        }
    }

    float renderAudioSample()
    {
        processSampleTick();

        // Render mono mix of all 4 track engines
        float mix = 0.0f;
        for (int t = 0; t < NUM_TRACKS; ++t) {
            if (tracks[t].engine) {
                // Note: Engine sample() is called regardless of mute status
                // so active release tails decay naturally!
                mix += tracks[t].engine->sample();
            }
        }
        return mix * 0.5f; // Headroom scaling
    }
};
