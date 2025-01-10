#ifndef _UI_PIXEL_COMPONENT_TAPE_H_
#define _UI_PIXEL_COMPONENT_TAPE_H_

#include "plugins/components/component.h"
#include "plugins/components/utils/color.h"
#include "helpers/range.h"

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
    Color valueColor;

    std::string folder = "tape";
    std::string filename = "track";
    SNDFILE* sndfile = nullptr;
    SF_INFO sfinfo;

    std::vector<float> avgBuffer;
    std::vector<float> rawBuffer;

    int beatStart = 0;
    int beatLength = 4;
    int totalBeat = 0;

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
        totalBeat = sfinfo.frames / samplesPerBeat;

        // Seek to the beatStarting point
        sf_seek(sndfile, beatStart * samplesPerBeat, SEEK_SET);

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
        , valueColor(applyAlphaColor(styles.colors.background, styles.colors.text, 0.8))
    {
    }

    void renderWavFile(int y, int h)
    {
        // Draw beat markers
        int samplesPerBeat = size.w / beatLength;
        for (int b = 0; b <= beatLength; ++b) {
            int x = relativePosition.x + b * samplesPerBeat;
            draw.line({ x,y }, { x, y + h }, { beatColor });
        }

        if (!rawBuffer.empty()) {
            std::vector<Point> waveformPoints;
            for (int i = 0; i < rawBuffer.size(); ++i) {
                float normalizedValue = (rawBuffer[i] + 1.0f) / 2.0f; // Normalize to 0-1
                int yy = y + h - static_cast<int>(normalizedValue * h);
                waveformPoints.push_back({ relativePosition.x + i, yy });
            }
            draw.lines(waveformPoints, { rawColor });
        }

        if (!avgBuffer.empty()) {
            std::vector<Point> waveformPoints;
            for (int i = 0; i < avgBuffer.size(); ++i) {
                float normalizedValue = (avgBuffer[i] + 1.0f) / 2.0f; // Normalize to 0-1
                int yy = y + h - static_cast<int>(normalizedValue * h);
                waveformPoints.push_back({ relativePosition.x + i, yy });
            }
            draw.lines(waveformPoints, { avgColor });
        }
    }

    void render()
    {
        if (updatePosition()) {
            draw.filledRect(relativePosition, size, { background });
            renderWavFile(relativePosition.y, size.h);

            // show total count
            draw.text({ relativePosition.x + 4, relativePosition.y + 2 }, std::to_string(beatStart) + "/" + std::to_string(totalBeat), 8, { valueColor });
        }
    }

    uint8_t beatEncoderId = 0;

    void onEncoder(int id, int8_t direction)
    {
        if (id == beatEncoderId) {
            beatStart += direction;
            beatStart = range(beatStart, 0, totalBeat);
            loadAudioFile();
            renderNext();
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

        /*md - `BEAT_ENCODER: encoderId` to set the encoder id to define the beat where to start to display the waveform (default: 0).*/
        if (strcmp(key, "BEAT_ENCODER") == 0) {
            beatEncoderId = atoi(value);
            return true;
        }

        return false;
    }
};

#endif
