/** Description:
This code defines a core structure named `SampleStep`, acting as a blueprint for a single programmable event within a larger sound sequencer or music program. It manages all the necessary settings required to play one specific audio sample.

### Data Management and Settings

The `SampleStep` class stores crucial information about the sound event:
1.  **Control:** A simple toggle (`enabled`) determines if the step is active.
2.  **Playback Volume:** The `velocity` setting controls the loudness, ensuring it stays within a valid range (0.0 to 1.0).
3.  **Source:** It tracks the path to the actual audio file (`filename`) and maintains a connection (`file`) to the sound library for reading the data.
4.  **Timing:** It defines which portion of the sound file to play, specifying the `start` and `end` points. These points are tracked both as percentages (for user settings) and as precise sample counts (for internal playback).

### Core Functionality

The class includes several protective functions to maintain integrity:

*   **Input Validation:** Functions like `setVelocity`, `setStart`, and `setEnd` automatically limit the input values to sensible boundaries. For example, the start point cannot be after the end point.
*   **File Loading (`setFilename`):** This is the key action. When a filename is provided, the step opens the audio file, reads its properties (like total length), and calculates how it should be played back. It also adjusts an internal increment value if the sound file has more or fewer audio channels than the system expects, ensuring smooth integration.
*   **Saving and Loading State:** The step can convert all its settings (velocity, enabled status, timing, and filename) into a single text string (`serialize`). This string can then be read back later (`hydrate`) to completely restore the step’s configuration, essential for saving and loading projects.

sha: 5d877d5919ec30a0affd80f00dea8e62fdd91889a7b8f942c551bb85bbca6821 
*/
#pragma once

#include <cstdint>
#include <string>
#include <sndfile.h>
#include <string.h>

#include "helpers/clamp.h"
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
