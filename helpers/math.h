#pragma once

inline float fastSin(float x) { return (1.27323954f * x) - (0.405284735f * x * x); }

inline float fastCos(float x) { return (1.27323954f * x) + (0.405284735f * x * x); }

inline float fastExpNeg(float x) { return 1.0f / (1.0f + x + 0.48f * x * x + 0.235f * x * x * x); }

inline float fastSqrtPoly(float x) { return x * (0.41731f + 0.59016f * x - 0.00761f * x * x); }

inline float exp6(float t) { float x2 = t * t; return x2 * x2 * t; }

inline float exp3(float t) { return t * t * t; }