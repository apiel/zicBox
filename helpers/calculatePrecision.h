#pragma once

#include <cmath>
#include <algorithm>

int calculatePrecision(float step) {
    if (step <= 0.0f || step == std::floor(step)) {
        return 0;
    }

    int precision = 0;
    float temp = step;

    while (precision < 7) {
        temp *= 10.0f;
        precision++;
        
        float diff = std::abs(temp - std::round(temp));
        
        if (diff < 0.0001f) { 
            break;
        }
    }

    return precision;
}