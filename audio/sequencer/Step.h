#pragma once

#ifndef SEQ_STEPS
#define SEQ_STEPS 64
#endif

struct Step {
    bool active = false;
    int note = 60;
    float velocity = 0.8f;
    float condition = 1.0f;
    float len = 1.0f; // 1.0f = 1 step
};
