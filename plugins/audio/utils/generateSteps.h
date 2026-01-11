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
    StepGeneratorType generator
) {
    std::vector<Step> steps;

    if (stepCount <= 0)
        return steps;

    switch (generator) {

    // --------------------------------
    // TECHNO KICK
    // 4-on-the-floor
    // --------------------------------
    case GEN_TECHNO_KICK: {
        int spacing = stepCount / 4;
        if (spacing < 1) spacing = 1;

        for (int i = 0; i < stepCount; i += spacing) {
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
        int p1 = stepCount / 4;
        int p2 = (stepCount * 3) / 4;

        steps.push_back({
            .enabled = true,
            .velocity = 0.9f,
            .position = (uint16_t)p1,
            .len = 1,
            .note = 38 // E1
        });
        steps.push_back({
            .enabled = true,
            .velocity = 0.9f,
            .position = (uint16_t)p2,
            .len = 1,
            .note = 38 // E1
        });
        break;
    }

    // --------------------------------
    // PSYTRANCE BASS
    // Rolling off-beat
    // --------------------------------
    case GEN_PSY_BASS: {
        int step = stepCount / 16;
        if (step < 1) step = 1;

        for (int i = step; i < stepCount; i += step * 2) {
            steps.push_back({
                .enabled = true,
                .velocity = 0.8f,
                .position = (uint16_t)i,
                .len = (uint16_t)step,
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
