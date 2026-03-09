#pragma once

static constexpr int SEQ_STEPS = 64;

struct Step {
    bool active = false;
    int note = 60;
    float velocity = 0.8f;
    float condition = 1.0f;
};