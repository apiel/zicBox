#pragma once

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <vector>

struct Step {
    bool active = false;
    uint8_t note = 60;
    float velocity = 0.8f;
    uint8_t probability = 100; // 0% to 100%
};

struct DrumTrack {
    const char* name = "Track";
    bool muted = false;
    Step steps[16];
};

class SequenceBrain {
public:
    static constexpr int NUM_TRACKS = 8;
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

        // 8 Tracks
        tracks[0].name = "Track 1";
        tracks[1].name = "Track 2";
        tracks[2].name = "Track 3";
        tracks[3].name = "Track 4";
        tracks[4].name = "Track 5";
        tracks[5].name = "Track 6";
        tracks[6].name = "Track 7";
        tracks[7].name = "Track 8";

        // Preset Groove
        tracks[0].steps[4].active = true;  // Snare
        tracks[0].steps[12].active = true;
        tracks[1].steps[4].active = true;  // Clap
        tracks[1].steps[12].active = true;
        tracks[2].steps[2].active = true;  // HatCl
        tracks[2].steps[6].active = true;
        tracks[2].steps[10].active = true;
        tracks[2].steps[14].active = true;
        tracks[4].steps[3].active = true;  // Perc1
        tracks[4].steps[7].active = true;
        tracks[4].steps[11].active = true;
        tracks[4].steps[15].active = true;
    }

    void generatePattern()
    {
        for (int t = 0; t < NUM_TRACKS; ++t) {
            for (int s = 0; s < NUM_STEPS; ++s) {
                float rnd = (float)rand() / RAND_MAX;
                if (t == 0 || t == 1) {
                    tracks[t].steps[s].active = (s == 4 || s == 12) || (rnd > 0.85f);
                } else if (t == 2 || t == 3) {
                    tracks[t].steps[s].active = (s % 2 == 0) || (rnd > 0.7f);
                } else {
                    tracks[t].steps[s].active = (rnd > 0.75f);
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
};
