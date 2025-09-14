#pragma once

#include <cstring>
#include <functional>
#include <math.h>
#include <stdint.h>
#include <string>
#include <time.h>

// #include "helpers/midiNote.h"
#include "helpers/format.h"
#include "helpers/clamp.h"
#include "log.h"

#include "libs/nlohmann/json.hpp"

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
    { "rand2", [](uint8_t) { return getRand() % 2; } },
    { "rand3", [](uint8_t) { return getRand() % 3; } },
    { "rand4", [](uint8_t) { return getRand() % 4; } },
    { "rand5", [](uint8_t) { return getRand() % 5; } },
    { "rand6", [](uint8_t) { return getRand() % 6; } },
    { "rand7", [](uint8_t) { return getRand() % 7; } },
    { "rand8", [](uint8_t) { return getRand() % 8; } },
    { "rand9", [](uint8_t) { return getRand() % 9; } },
    { "rand10", [](uint8_t) { return getRand() % 10; } },
    { "rand11", [](uint8_t) { return getRand() % 11; } },
    { "rand12", [](uint8_t) { return getRand() % 12; } },
    { "rand13", [](uint8_t) { return getRand() % 13; } },
    { "rand14", [](uint8_t) { return getRand() % 14; } },
    { "rand15", [](uint8_t) { return getRand() % 15; } },
    { "rand16", [](uint8_t) { return getRand() % 16; } },
    { "rand17", [](uint8_t) { return getRand() % 17; } },
    { "rand18", [](uint8_t) { return getRand() % 18; } },
    { "rand0_2", [](uint8_t) { return getRand() % 2 == 0 ? 0 : 2; } },
    { "rand0_3", [](uint8_t) { return getRand() % 2 == 0 ? 0 : 3; } },
    { "rand0_4", [](uint8_t) { return getRand() % 2 == 0 ? 0 : 4; } },
    { "rand0_5", [](uint8_t) { return getRand() % 2 == 0 ? 0 : 5; } },
    { "rand0_6", [](uint8_t) { return getRand() % 2 == 0 ? 0 : 6; } },
    { "rand0_12", [](uint8_t) { return getRand() % 2 == 0 ? 0 : 12; } },
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
        this->condition = CLAMP(condition, 0, STEP_CONDITIONS_COUNT - 1);
    }

    void setMotion(int motion)
    {
        this->motion = CLAMP(motion, 0, STEP_MOTIONS_COUNT - 1);
    }

    void setNote(int note)
    {
        this->note = CLAMP(note, 0, 127);
    }

    void setVelocity(float velocity)
    {
        this->velocity = CLAMP(velocity, 0.0, 1.0);
    }

    void setLength(int len)
    {
        this->len = CLAMP(len, 0, 4096);
    }

    void setPosition(int position)
    {
        this->position = CLAMP(position, 0, 4096);
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

    nlohmann::json json;
    nlohmann::json& serializeJson()
    {
        json["position"] = position;
        json["len"] = len;
        json["enabled"] = enabled;
        json["note"] = note;
        json["velocity"] = velocity;
        json["condition"] = stepConditions[condition].name;
        json["motion"] = stepMotions[motion].name;
        return json;
    }

    void hydrateJson(nlohmann::json& json)
    {
        try {
            position = json["position"];
            len = json["len"];
            enabled = json["enabled"];
            note = json["note"];
            velocity = json["velocity"];
            std::string conditionName = json["condition"];
            for (int i = 0; i < STEP_CONDITIONS_COUNT; i++) {
                if (stepConditions[i].name == conditionName) {
                    condition = i;
                    break;
                }
            }
            std::string motionName = json["motion"];
            for (int i = 0; i < STEP_MOTIONS_COUNT; i++) {
                if (stepMotions[i].name == motionName) {
                    motion = i;
                    break;
                }
            }
        } catch (const nlohmann::json::exception& e) {
            std::string errorMessage = e.what();
            logWarn("Failed to hydrate step: " + errorMessage);
        }
    }
};
