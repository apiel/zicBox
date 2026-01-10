#pragma once

#include <algorithm>
#include <cmath>
#include <vector>

#include "plugins/audio/stepInterface.h"

std::vector<Step> densifySteps(const std::vector<Step>& sourceSteps, int stepCount, float density)
{
    // Center → original
    if (std::abs(density) < 0.001f)
        return sourceSteps;

    std::vector<Step> result = sourceSteps;

    int baseCount = sourceSteps.size();
    int maxCount = stepCount;

    // Target number of steps
    int delta = (int)std::round(density * (maxCount - baseCount));
    int targetCount = std::clamp(baseCount + delta, 0, maxCount);

    // ---------------------------
    // ADD STEPS
    // ---------------------------
    while ((int)result.size() < targetCount) {

        // Collect positions
        std::vector<int> pos;
        pos.reserve(result.size());
        for (auto& s : result)
            pos.push_back(s.position);

        std::sort(pos.begin(), pos.end());

        // Find largest gap
        int bestGap = -1;
        int bestStart = -1;

        for (int i = 0; i < (int)pos.size(); i++) {
            int a = pos[i];
            int b = pos[(i + 1) % pos.size()];
            int gap = (b - a - 1 + stepCount) % stepCount;

            if (gap > bestGap) {
                bestGap = gap;
                bestStart = a;
            }
        }

        // No space left
        if (bestGap <= 0)
            break;

        // Insert in the middle of the gap
        int insertPos = (bestStart + 1 + bestGap / 2) % stepCount;

        // Find nearest step to inherit data
        Step ref = result[0];
        int bestDist = stepCount;

        for (auto& s : result) {
            int d = std::abs((int)s.position - insertPos);
            if (d < bestDist) {
                bestDist = d;
                ref = s;
            }
        }

        Step ns;
        ns.enabled = true;
        ns.position = insertPos;
        ns.note = ref.note;
        ns.len = 1;
        ns.velocity = ref.velocity * 0.8f;

        result.push_back(ns);
    }

    // ---------------------------
    // REMOVE STEPS
    // ---------------------------
    while ((int)result.size() > targetCount) {

        int removeIndex = -1;
        int smallestGap = stepCount;

        for (int i = 0; i < (int)result.size(); i++) {
            int p = result[i].position;

            int leftDist = stepCount;
            int rightDist = stepCount;

            for (auto& s : result) {
                if (s.position < p)
                    leftDist = std::min(leftDist, p - s.position);
                if (s.position > p)
                    rightDist = std::min(rightDist, s.position - p);
            }

            int localGap = leftDist + rightDist;

            if (localGap < smallestGap) {
                smallestGap = localGap;
                removeIndex = i;
            }
        }

        if (removeIndex >= 0)
            result.erase(result.begin() + removeIndex);
        else
            break;
    }

    return result;
}
