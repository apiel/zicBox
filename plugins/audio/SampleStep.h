#pragma once

#include <cstdint>
#include <string>
#include <sndfile.h>
#include <string.h>

#include "helpers/range.h"
#include "helpers/format.h"
#include "log.h"

class SampleStep {
public:
    bool enabled = false;
    float velocity = 0.8f;
    uint64_t sampleCount = 0;
    uint64_t end = 0;
    float fEnd = 0.0f;
    uint64_t start = 0;
    float fStart = 0.0f;
    std::string filename = "---";
    SNDFILE* file = NULL;
    float stepIncrement = 1.0f;

    void setVelocity(float velocity)
    {
        this->velocity = CLAMP(velocity, 0.0, 1.0);
    }

    void setEnd(float value)
    {
        fEnd = CLAMP(value, fStart, 100.0);
        end = fEnd * sampleCount;
    }

    void setStart(float value)
    {
        // this->start = CLAMP(start, 0, sampleCount);
        fStart = CLAMP(value, 0.0, fEnd);
        start = fStart * sampleCount;
    }

    void setFilename(std::string filename, uint8_t channels)
    {
        // Let's keep it easy for the moment each step has his own instance of SNDFILE
        // We gonna try to optimize later only if necessary by reusing the same SNDFILE
        if (file != NULL) {
            sf_close(file);
            file = NULL;
        }
        this->filename = filename;
        if (filename != "---") {
            SF_INFO sfinfo;
            // printf("Load filename %s\n", filename.c_str());
            file = sf_open(filename.c_str(), SFM_READ, &sfinfo);
            if (file) {
                sampleCount = sfinfo.frames;
                if (sfinfo.channels < channels) {
                    stepIncrement = 0.5f;
                } else if (sfinfo.channels > channels) {
                    stepIncrement = 2.0f;
                } else {
                    stepIncrement = 1.0f;
                }
                setStart(fStart);
                setEnd(fEnd);
            } else {
                logWarn("SampleSequencer: Could not open step file %s [%s]", filename.c_str(), sf_strerror(file));
            }
        }
    }

    // need to be changed to JSON!!
    std::string serialize()
    {
        return std::to_string(enabled) + " "
            + fToString(velocity, 2) + " "
            + std::to_string(fStart) + " "
            + std::to_string(fEnd) + " "
            + filename;
    }

    // need to be changed to JSON!!
    void hydrate(std::string value, uint8_t channels)
    {
        // printf("hydrate %s\n", value.c_str());
        enabled = strtok((char*)value.c_str(), " ")[0] == '1';
        velocity = atof(strtok(NULL, " "));
        fStart = atof(strtok(NULL, " "));
        fEnd = atof(strtok(NULL, " "));
        setFilename(strtok(NULL, " "), channels);
    }
};
