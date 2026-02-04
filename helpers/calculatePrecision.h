#pragma once

#include <cmath>
#include <algorithm>

int calculatePrecision(float step) {
    // If step is 0 or an integer, no decimals needed
    if (step <= 0.0f || step == std::floor(step)) {
        return 0;
    }

    int precision = 0;
    float temp = step;

    // Keep multiplying by 10 until the remainder is negligible
    // Limit to 7 to avoid infinite loops with weird floats
    while (precision < 7) {
        temp *= 10.0f;
        precision++;
        
        // Check if the current value is close enough to an integer
        float diff = std::abs(temp - std::round(temp));
        if (diff < 0.0001f) { 
            break;
        }
    }

    return precision;
}