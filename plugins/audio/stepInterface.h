#ifndef _STEP_SEQUENCER_H_
#define _STEP_SEQUENCER_H_

#include <math.h>
#include <stdint.h>
#include <time.h>

// #include "../../helpers/midiNote.h"
#include "../../helpers/range.h"

int randCounter = 0;
int getRand()
{
    srand(time(NULL) + randCounter++);
    return rand();
}

struct StepCondition {
    const char* name;
    bool (*conditionMet)(uint8_t loopCounter);
} stepConditions[] = {
    { "---", [](uint8_t loopCounter) { return true; } },
    { "Pair", [](uint8_t loopCounter) { return loopCounter % 2 == 0; } },
    { "4th", [](uint8_t loopCounter) { return loopCounter % 4 == 0; } },
    { "6th", [](uint8_t loopCounter) { return loopCounter % 6 == 0; } },
    { "8th", [](uint8_t loopCounter) { return loopCounter % 8 == 0; } },
    { "Impair", [](uint8_t loopCounter) { return loopCounter % 2 == 1; } },
    { "1%", [](uint8_t loopCounter) { return (getRand() % 100) == 0; } },
    { "2%", [](uint8_t loopCounter) { return (getRand() % 100) < 2; } },
    { "5%", [](uint8_t loopCounter) { return (getRand() % 100) < 5; } },
    { "10%", [](uint8_t loopCounter) { return (getRand() % 100) < 10; } },
    { "20%", [](uint8_t loopCounter) { return (getRand() % 100) < 20; } },
    { "30%", [](uint8_t loopCounter) { return (getRand() % 100) < 30; } },
    { "40%", [](uint8_t loopCounter) { return (getRand() % 100) < 40; } },
    { "50%", [](uint8_t loopCounter) { return (getRand() % 100) < 50; } },
    { "60%", [](uint8_t loopCounter) { return (getRand() % 100) < 60; } },
    { "70%", [](uint8_t loopCounter) { return (getRand() % 100) < 70; } },
    { "80%", [](uint8_t loopCounter) { return (getRand() % 100) < 80; } },
    { "90%", [](uint8_t loopCounter) { return (getRand() % 100) < 90; } },
    { "95%", [](uint8_t loopCounter) { return (getRand() % 100) < 95; } },
    { "98%", [](uint8_t loopCounter) { return (getRand() % 100) < 98; } },
    { "99%", [](uint8_t loopCounter) { return (getRand() % 100) < 99; } },
};

uint8_t STEP_CONDITIONS_COUNT = sizeof(stepConditions) / sizeof(stepConditions[0]);
const uint8_t MAX_STEPS = 32;

class Step {
public:
    bool enabled = false;
    float velocity = 0;
    uint8_t condition = 0;
    uint8_t len = 1; // len 0 is infinite?
    uint8_t counter = 0;
    uint8_t note = 60;

    void reset()
    {
        enabled = false;
        velocity = 0;
        condition = 0;
        len = 1;
        counter = 0;
        note = 60;
    }

    void setCondition(int condition)
    {
        this->condition = range(condition, 0, STEP_CONDITIONS_COUNT - 1);
    }

    void setNote(int note)
    {
        this->note = range(note, 0, 127);
    }

    void setVelocity(float velocity)
    {
        this->velocity = range(velocity, 0.0, 1.0);
    }

    void setLength(int len)
    {
        this->len = range(len, 0, MAX_STEPS);
    }
};

#endif
