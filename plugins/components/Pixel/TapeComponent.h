#ifndef _UI_PIXEL_COMPONENT_TAPE_H_
#define _UI_PIXEL_COMPONENT_TAPE_H_

#include "plugins/components/component.h"
#include "plugins/components/utils/color.h"

#include <algorithm>
#include <cmath>
#include <sndfile.h>
#include <string>
#include <vector>

/*md
## Tape

Tape components to draw a rectangle.
*/

class TapeComponent : public Component {
protected:
    Color background;
    Color avgColor;
    Color rawColor;
    Color beatColor;

    std::string folder = "tape";
    std::string filename = "track";
    SNDFILE* sndfile = nullptr;
    SF_INFO sfinfo;

    std::vector<float> avgBuffer;
    std::vector<float> rawBuffer;

    int start = 0;
    int beatLength = 4;

    void loadAudioFile()
    {
        std::string filepath = folder + "/tmp/" + filename + ".wav";
        sndfile = sf_open(filepath.c_str(), SFM_READ, &sfinfo);
        if (!sndfile) {
            // Handle file open error
            return;
        }

        // Calculate the number of samples per beat and total samples to load
        int sampleRate = sfinfo.samplerate;
        int channels = sfinfo.channels;
        int samplesPerBeat = (sampleRate * 60) / 160; // Assuming 160 BPM for now
        int totalSamples = samplesPerBeat * beatLength;

        // Seek to the starting point
        sf_seek(sndfile, start * samplesPerBeat, SEEK_SET);

        // Resize the buffer to match the width of the UI component
        avgBuffer.resize(size.w);
        rawBuffer.resize(size.w);

        // Temporary buffer to read audio samples
        std::vector<float> tempBuffer(totalSamples * channels);
        sf_read_float(sndfile, tempBuffer.data(), totalSamples * channels);

        // Downsample using averaging
        int samplesPerPixel = totalSamples / size.w;
        for (int i = 0; i < size.w; ++i) {
            float sum = 0.0f;
            int count = 0;

            for (int j = 0; j < samplesPerPixel; ++j) {
                int sampleIndex = (i * samplesPerPixel + j) * channels; // First channel
                if (sampleIndex < tempBuffer.size()) {
                    sum += tempBuffer[sampleIndex];
                    ++count;
                }
            }

            avgBuffer[i] = (count > 0) ? (sum / count) : 0.0f; // Avoid division by zero

            int sampleIndex = i * (totalSamples / size.w);
            float sampleValue = tempBuffer[sampleIndex * channels]; // Take the first channel
            rawBuffer[i] = sampleValue;
        }

        sf_close(sndfile);
        sndfile = nullptr;
    }

public:
    TapeComponent(ComponentInterface::Props props)
        : Component(props)
        , background(styles.colors.background)
        , avgColor(styles.colors.primary)
        , rawColor(darken(styles.colors.primary, 0.5))
        , beatColor(lighten(styles.colors.background, 0.5))
    {
    }

    void render()
    {
        if (updatePosition()) {
            draw.filledRect(relativePosition, size, { background });

            // Draw beat markers
            int samplesPerBeat = size.w / beatLength;
            for (int b = 0; b <= beatLength; ++b) {
                int x = relativePosition.x + b * samplesPerBeat;
                draw.line({ x, relativePosition.y }, { x, relativePosition.y + size.h }, { beatColor });
            }

            if (!rawBuffer.empty()) {
                std::vector<Point> waveformPoints;
                for (int i = 0; i < rawBuffer.size(); ++i) {
                    float normalizedValue = (rawBuffer[i] + 1.0f) / 2.0f; // Normalize to 0-1
                    int y = relativePosition.y + size.h - static_cast<int>(normalizedValue * size.h);
                    waveformPoints.push_back({ relativePosition.x + i, y });
                }
                draw.lines(waveformPoints, { rawColor });
            }

            if (!avgBuffer.empty()) {
                std::vector<Point> waveformPoints;
                for (int i = 0; i < avgBuffer.size(); ++i) {
                    float normalizedValue = (avgBuffer[i] + 1.0f) / 2.0f; // Normalize to 0-1
                    int y = relativePosition.y + size.h - static_cast<int>(normalizedValue * size.h);
                    waveformPoints.push_back({ relativePosition.x + i, y });
                }
                draw.lines(waveformPoints, { avgColor });
            }
        }
    }

    /*md **Config**: */
    bool config(char* key, char* value)
    {
        /*md - `BACKGROUND_COLOR: background` is the color of the background of the component. */
        if (strcmp(key, "BACKGROUND_COLOR") == 0) {
            background = draw.getColor(value);
            return true;
        }

        /*md - `TAPE_FOLDER` set samples folder path. */
        if (strcmp(key, "TAPE_FOLDER") == 0) {
            folder = value;
            return true;
        }

        /*md - `FILENAME: filename` to set filename. By default it is `track`.*/
        if (strcmp(key, "FILENAME") == 0) {
            filename = value;
            loadAudioFile();
            return true;
        }

        return false;
    }
};

#endif
