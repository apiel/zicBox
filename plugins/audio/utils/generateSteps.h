#pragma once

#include <vector>

#include "helpers/random.h"
#include "plugins/audio/stepInterface.h"

enum StepGeneratorType {
    GEN_TECHNO_KICK = 0,
    GEN_TECHNO_SNARE,
    GEN_PSY_BASS,
    GEN_DUB_CHORD,
    GEN_ARP,
    GEN_ACID_303,
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

    // --------------------------------
    // ARPEGGIATOR (REPETITIVE)
    // --------------------------------
    case GEN_ARP: {

        // Choose scale once
        const int scales[][5] = {
            { 0, 4, 7, 12, -1 }, // major
            { 0, 3, 7, 12, -1 }, // minor
            { 0, 3, 7, 10, 12 } // minor 7
        };

        int scaleIndex = (int)(rand01(genSeed) * 3);
        const int* scale = scales[scaleIndex];

        int scaleSize = 0;
        while (scale[scaleSize] >= 0)
            scaleSize++;

        // ARP settings (decided ONCE)
        int arpLen = 4 + (int)(rand01(genSeed) * 4); // 4–7 steps
        int dir = rand01(genSeed) < 0.5f ? 1 : -1;
        bool upDown = rand01(genSeed) < 0.4f;

        int idx = (dir > 0) ? 0 : scaleSize - 1;

        for (int i = 0; i < stepCount; i++) {

            int stepInArp = i % arpLen;

            int note = genNote + scale[idx];

            steps.push_back({ .enabled = true,
                .velocity = 0.7f,
                .position = (uint16_t)i,
                .len = 1,
                .note = (uint8_t)note });

            // advance arp ONLY inside arp cycle
            if (stepInArp == arpLen - 1)
                continue;

            idx += dir;

            if (idx >= scaleSize) {
                if (upDown) {
                    idx = scaleSize - 2;
                    dir = -1;
                } else {
                    idx = 0;
                }
            } else if (idx < 0) {
                if (upDown) {
                    idx = 1;
                    dir = 1;
                } else {
                    idx = scaleSize - 1;
                }
            }
        }
        break;
    }

    // --------------------------------
    // ACID 303 STYLE
    // --------------------------------
    case GEN_ACID_303: { // Need to work on this.... maybe a mix of kick pattern but every 2 step, mix a bit with ARP...

        const int notes[] = {
            0, // root
            7, // fifth
            12, // octave
            10 // minor 7th (acid flavor)
        };
        const int noteCount = 4;

        for (int i = 0; i < stepCount; i++) {

            if (rand01(genSeed) < 0.2f)
                continue;

            int n = notes[(int)(rand01(genSeed) * noteCount)];
            bool accent = rand01(genSeed) < 0.3f;
            bool slide = rand01(genSeed) < 0.25f;

            steps.push_back({ .enabled = true,
                .velocity = accent ? 1.0f : 0.65f,
                .position = (uint16_t)i,
                .len = (uint16_t)(slide ? 2 : 1),
                .note = (uint8_t)(genNote + n) });
        }
        break;
    }

    default:
        break;
    }

    return steps;
}