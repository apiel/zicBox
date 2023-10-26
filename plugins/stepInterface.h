#ifndef _STEP_SEQUENCER_H_
#define _STEP_SEQUENCER_H_

#include <stdint.h>

struct Step {
    bool enabled = false;
    float velocity = 0;
    uint8_t condition = 0;
    uint8_t len = 1; // len 0 is infinite?
    uint8_t counter = 0;
    uint8_t note = 60;
};

#endif
