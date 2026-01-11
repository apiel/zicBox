#pragma once

#include <vector>

#include "helpers/random.h"
#include "plugins/audio/stepInterface.h"

enum StepGeneratorType {
    GEN_TECHNO_KICK = 0,
    GEN_TECHNO_SNARE,
    GEN_PSY_BASS,
    GEN_DUB_CHORD,
    GEN_COUNT
};

uint32_t genSeed = 0;
uint8_t genNote = 60;

std::vector<Step> generateSteps(int stepCount, StepGeneratorType generator)
{
    std::vector<Step> steps;
    if (stepCount <= 0)
        return steps;

    genSeed++;

    switch (generator) {

    // --------------------------------
    // TECHNO KICK
    // --------------------------------
    case GEN_TECHNO_KICK: {
        bool lastHasGhost = true;
        for (int i = 0; i < stepCount; i += 4) {

            if (!lastHasGhost && rand01(genSeed) < 0.05f) // If it has ghost, step is mandatory
                continue;

            steps.push_back({
                .enabled = true,
                .velocity = 0.9f + rand01(genSeed) * 0.1f,
                .position = (uint16_t)i,
                .len = 1,
                .note = genNote // C1
            });

            lastHasGhost = false;
            for (int j = 1; j < 3; j++) {
                if (rand01(genSeed) < 0.1f) {
                    lastHasGhost = true;
                    steps.push_back({ .enabled = true,
                        .velocity = 0.4f,
                        .position = (uint16_t)(i + j + 1),
                        .len = 1,
                        .note = genNote });
                }
            }
        }
        break;
    }

    // --------------------------------
    // TECHNO SNARE
    // --------------------------------
    case GEN_TECHNO_SNARE: {
        for (int i = 4; i < stepCount; i += 8) {

            float vel = 0.7f + rand01(genSeed) * 0.3f;

            steps.push_back({
                .enabled = true,
                .velocity = vel,
                .position = (uint16_t)i,
                .len = 1,
                .note = genNote // D1
            });

            // flam
            if (rand01(genSeed) < 0.2f && i + 1 < stepCount) {
                steps.push_back({ .enabled = true,
                    .velocity = vel * 0.5f,
                    .position = (uint16_t)(i + 1),
                    .len = 1,
                    .note = genNote });
            }
        }
        break;
    }

    // --------------------------------
    // PSYTRANCE BASS
    // --------------------------------
    case GEN_PSY_BASS: {
        for (int i = 0; i < stepCount; i++) {

            if (i % 4 == 0)
                continue;

            if (rand01(genSeed) < 0.15f)
                continue;

            steps.push_back({
                .enabled = true,
                .velocity = (i % 2 == 0) ? 0.9f : 0.7f,
                .position = (uint16_t)i,
                .len = 1,
                .note = genNote // G1
            });
        }
        break;
    }

    // --------------------------------
    // DUB CHORD
    // --------------------------------
    case GEN_DUB_CHORD: {
        int spacing = stepCount / 3;
        if (spacing < 1) spacing = 1;

        for (int i = spacing / 2; i < stepCount; i += spacing) {

            if (rand01(genSeed) < 0.25f)
                continue;

            steps.push_back({
                .enabled = true,
                .velocity = 0.5f + rand01(genSeed) * 0.2f,
                .position = (uint16_t)i,
                .len = (uint16_t)(spacing + ((rand01(genSeed) < 0.3f) ? spacing / 2 : 0)),
                .note = genNote // C2
            });
        }
        break;
    }

    default:
        break;
    }

    return steps;
}