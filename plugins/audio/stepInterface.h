#pragma once

#include <cstring>
#include <functional>
#include <math.h>
#include <stdint.h>
#include <string>
#include <time.h>

// #include "helpers/midiNote.h"
#include "helpers/format.h"
#include "helpers/range.h"

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

struct StepMotion {
    const char* name;
    std::function<int8_t(uint8_t loopCounter)> get;
} stepMotions[] = {
    { "---", [](uint8_t loopCounter) { return 0; } },
    { "0,1", [](uint8_t loopCounter) { uint8_t semitone[2] = { 0, 1 }; return semitone[loopCounter % 2]; } },
    { "1,0", [](uint8_t loopCounter) { uint8_t semitone[2] = { 1, 0 }; return semitone[loopCounter % 2]; } },
    { "0,2", [](uint8_t loopCounter) { uint8_t semitone[2] = { 0, 2 }; return semitone[loopCounter % 2]; } },
    { "2,0", [](uint8_t loopCounter) { uint8_t semitone[2] = { 2, 0 }; return semitone[loopCounter % 2]; } },
    { "0,3", [](uint8_t loopCounter) { uint8_t semitone[2] = { 0, 3 }; return semitone[loopCounter % 2]; } },
    { "3,0", [](uint8_t loopCounter) { uint8_t semitone[2] = { 3, 0 }; return semitone[loopCounter % 2]; } },
    { "0,4", [](uint8_t loopCounter) { uint8_t semitone[2] = { 0, 4 }; return semitone[loopCounter % 2]; } },
    { "4,0", [](uint8_t loopCounter) { uint8_t semitone[2] = { 4, 0 }; return semitone[loopCounter % 2]; } },
    { "0,5", [](uint8_t loopCounter) { uint8_t semitone[2] = { 0, 5 }; return semitone[loopCounter % 2]; } },
    { "5,0", [](uint8_t loopCounter) { uint8_t semitone[2] = { 5, 0 }; return semitone[loopCounter % 2]; } },
    { "0,6", [](uint8_t loopCounter) { uint8_t semitone[2] = { 0, 6 }; return semitone[loopCounter % 2]; } },
    { "6,0", [](uint8_t loopCounter) { uint8_t semitone[2] = { 6, 0 }; return semitone[loopCounter % 2]; } },
    { "0,12", [](uint8_t loopCounter) { uint8_t semitone[2] = { 0, 12 }; return semitone[loopCounter % 2]; } },
    { "12,0", [](uint8_t loopCounter) { uint8_t semitone[2] = { 12, 0 }; return semitone[loopCounter % 2]; } },
    { "0,1,2", [](uint8_t loopCounter) { uint8_t semitone[3] = { 0, 1, 2 }; return semitone[loopCounter % 3]; } },
    { "2,1,0", [](uint8_t loopCounter) { uint8_t semitone[3] = { 2, 1, 0 }; return semitone[loopCounter % 3]; } },
    { "0,2,4", [](uint8_t loopCounter) { uint8_t semitone[3] = { 0, 2, 4 }; return semitone[loopCounter % 3]; } },
    { "4,2,0", [](uint8_t loopCounter) { uint8_t semitone[3] = { 4, 2, 0 }; return semitone[loopCounter % 3]; } },
    { "0,3,6", [](uint8_t loopCounter) { uint8_t semitone[3] = { 0, 3, 6 }; return semitone[loopCounter % 3]; } },
    { "6,3,0", [](uint8_t loopCounter) { uint8_t semitone[3] = { 6, 3, 0 }; return semitone[loopCounter % 3]; } },
    { "0,4,8", [](uint8_t loopCounter) { uint8_t semitone[3] = { 0, 4, 8 }; return semitone[loopCounter % 3]; } },
    { "8,4,0", [](uint8_t loopCounter) { uint8_t semitone[3] = { 8, 4, 0 }; return semitone[loopCounter % 3]; } },
    { "0,6,12", [](uint8_t loopCounter) { uint8_t semitone[3] = { 0, 6, 12 }; return semitone[loopCounter % 3]; } },
    { "12,6,0", [](uint8_t loopCounter) { uint8_t semitone[3] = { 12, 6, 0 }; return semitone[loopCounter % 3]; } },
    { "0,12,24", [](uint8_t loopCounter) { uint8_t semitone[3] = { 0, 12, 24 }; return semitone[loopCounter % 3]; } },
    { "24,12,0", [](uint8_t loopCounter) { uint8_t semitone[3] = { 24, 12, 0 }; return semitone[loopCounter % 3]; } },
    { "0,1,2,3", [](uint8_t loopCounter) { uint8_t semitone[4] = { 0, 1, 2, 3 }; return semitone[loopCounter % 4]; } },
    { "3,2,1,0", [](uint8_t loopCounter) { uint8_t semitone[4] = { 3, 2, 1, 0 }; return semitone[loopCounter % 4]; } },
    { "0,2,4,6", [](uint8_t loopCounter) { uint8_t semitone[4] = { 0, 2, 4, 6 }; return semitone[loopCounter % 4]; } },
    { "6,4,2,0", [](uint8_t loopCounter) { uint8_t semitone[4] = { 6, 4, 2, 0 }; return semitone[loopCounter % 4]; } },
    { "0,3,6,9", [](uint8_t loopCounter) { uint8_t semitone[4] = { 0, 3, 6, 9 }; return semitone[loopCounter % 4]; } },
    { "9,6,3,0", [](uint8_t loopCounter) { uint8_t semitone[4] = { 9, 6, 3, 0 }; return semitone[loopCounter % 4]; } },
    { "0,4,8,12", [](uint8_t loopCounter) { uint8_t semitone[4] = { 0, 4, 8, 12 }; return semitone[loopCounter % 4]; } },
    { "12,8,4,0", [](uint8_t loopCounter) { uint8_t semitone[4] = { 12, 8, 4, 0 }; return semitone[loopCounter % 4]; } },
    { "0,6,12,18", [](uint8_t loopCounter) { uint8_t semitone[4] = { 0, 6, 12, 18 }; return semitone[loopCounter % 4]; } },
    { "18,12,6,0", [](uint8_t loopCounter) { uint8_t semitone[4] = { 18, 12, 6, 0 }; return semitone[loopCounter % 4]; } },
    { "0,2,1,3", [](uint8_t loopCounter) { uint8_t semitone[4] = { 0, 2, 1, 3 }; return semitone[loopCounter % 4]; } },
    { "3,1,2,0", [](uint8_t loopCounter) { uint8_t semitone[4] = { 3, 1, 2, 0 }; return semitone[loopCounter % 4]; } },
    { "0,4,2,6", [](uint8_t loopCounter) { uint8_t semitone[4] = { 0, 4, 2, 6 }; return semitone[loopCounter % 4]; } },
    { "6,2,4,0", [](uint8_t loopCounter) { uint8_t semitone[4] = { 6, 2, 4, 0 }; return semitone[loopCounter % 4]; } },
    { "0,6,3,9", [](uint8_t loopCounter) { uint8_t semitone[4] = { 0, 6, 3, 9 }; return semitone[loopCounter % 4]; } },
    { "9,3,6,0", [](uint8_t loopCounter) { uint8_t semitone[4] = { 9, 3, 6, 0 }; return semitone[loopCounter % 4]; } },
    { "0,8,4,12", [](uint8_t loopCounter) { uint8_t semitone[4] = { 0, 8, 4, 12 }; return semitone[loopCounter % 4]; } },
    { "12,4,8,0", [](uint8_t loopCounter) { uint8_t semitone[4] = { 12, 4, 8, 0 }; return semitone[loopCounter % 4]; } },
    { "0,12,6,18", [](uint8_t loopCounter) { uint8_t semitone[4] = { 0, 12, 6, 18 }; return semitone[loopCounter % 4]; } },
    { "18,6,12,0", [](uint8_t loopCounter) { uint8_t semitone[4] = { 18, 6, 12, 0 }; return semitone[loopCounter % 4]; } },
};

uint8_t STEP_CONDITIONS_COUNT = sizeof(stepConditions) / sizeof(stepConditions[0]);
uint8_t STEP_MOTIONS_COUNT = sizeof(stepMotions) / sizeof(stepMotions[0]);

class Step {
public:
    bool enabled = false;
    float velocity = 0.8f;
    uint8_t condition = 0;
    uint16_t position = 0;
    uint16_t len = 1; // len 0 is infinite?
    uint8_t counter = 0;
    uint8_t note = 60;
    uint8_t motion = 0;

    void reset()
    {
        enabled = false;
        velocity = 0;
        condition = 0;
        position = 0;
        len = 1;
        counter = 0;
        note = 60;
        motion = 0;
    }

    bool equal(Step& other)
    {
        return enabled == other.enabled
            && velocity == other.velocity
            && condition == other.condition
            && motion == other.motion
            && position == other.position
            && len == other.len
            && note == other.note;
    }

    void setCondition(int condition)
    {
        this->condition = range(condition, 0, STEP_CONDITIONS_COUNT - 1);
    }

    void setMotion(int motion)
    {
        this->motion = range(motion, 0, STEP_MOTIONS_COUNT - 1);
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
        this->len = range(len, 0, 4096);
    }

    void setPosition(int position)
    {
        this->position = range(position, 0, 4096);
    }

    std::string serialize()
    {
        return std::to_string(position) + " "
            + std::to_string(len) + " "
            + std::to_string(enabled) + " "
            + std::to_string(note) + " "
            + fToString(velocity, 2) + " "
            + stepConditions[condition].name + " "
            + stepMotions[motion].name;
    }

    bool hydrate(std::string value)
    {
        position = atoi(strtok((char*)value.c_str(), " "));
        len = atoi(strtok(NULL, " "));
        enabled = strtok(NULL, " ")[0] == '1';
        note = atoi(strtok(NULL, " "));
        velocity = atof(strtok(NULL, " "));
        char* conditionName = strtok(NULL, " ");
        for (int i = 0; i < STEP_CONDITIONS_COUNT; i++) {
            if (strcmp(stepConditions[i].name, conditionName) == 0) {
                condition = i;
                break;
            }
        }
        char* motionName = strtok(NULL, " ");
        for (int i = 0; i < STEP_MOTIONS_COUNT; i++) {
            if (strcmp(stepMotions[i].name, motionName) == 0) {
                motion = i;
                break;
            }
        }

        return enabled && len > 0;
    }
};
