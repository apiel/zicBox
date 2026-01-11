#pragma once

#include <vector>

#include "plugins/audio/stepInterface.h"

enum StepGeneratorType {
    GEN_TECHNO_KICK = 0,
    GEN_TECHNO_SNARE,
    GEN_PSY_BASS,
    GEN_DUB_CHORD,
    GEN_COUNT
};

std::vector<Step> generateSteps(
    const std::vector<Step>& /*baseSteps*/,
    int stepCount,
    StepGeneratorType generator)
{
    std::vector<Step> steps;

    if (stepCount <= 0)
        return steps;

    switch (generator) {

    // --------------------------------
    // TECHNO KICK
    // 4-on-the-floor
    // --------------------------------
    case GEN_TECHNO_KICK: {
        for (int i = 0; i < stepCount; i += 4) {
            steps.push_back({
                .enabled = true,
                .velocity = 1.0f,
                .position = (uint16_t)i,
                .len = 1,
                .note = 36 // C1
            });
        }
        break;
    }

    // --------------------------------
    // TECHNO SNARE
    // Backbeat
    // --------------------------------
    case GEN_TECHNO_SNARE: {
        for (int i = 0; i < stepCount; i += 8) {
            steps.push_back({
                .enabled = true,
                .velocity = 1.0f,
                .position = (uint16_t)i,
                .len = 1,
                .note = 36 // C1
            });
        }
        break;
    }

    // --------------------------------
    // PSYTRANCE BASS
    // Rolling off-beat
    // --------------------------------
    case GEN_PSY_BASS: {
        for (int i = 0; i < stepCount; i++) {
            if (i % 4 == 0) continue;

            steps.push_back({
                .enabled = true,
                .velocity = i % 2 == 0 ? 0.9f : 0.7f,
                .position = (uint16_t)i,
                .len = (uint16_t)i,
                .note = 43 // G1
            });
        }
        break;
    }

    // --------------------------------
    // DUB CHORD
    // Sparse & long
    // --------------------------------
    case GEN_DUB_CHORD: {
        int spacing = stepCount / 3;
        if (spacing < 1) spacing = 1;

        for (int i = spacing / 2; i < stepCount; i += spacing) {
            steps.push_back({
                .enabled = true,
                .velocity = 0.6f,
                .position = (uint16_t)i,
                .len = (uint16_t)spacing,
                .note = 48 // C2
            });
        }
        break;
    }

    default:
        break;
    }

    return steps;
}
