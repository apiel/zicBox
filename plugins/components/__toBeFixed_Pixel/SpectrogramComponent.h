/** Description:
This file defines a core building block for a graphical user interface (UI) system, known as the `SpectrogramComponent`. Its primary purpose is to visually represent live audio data.

**Core Functionality**

The component acts as an audio visualizer. It constantly receives numerical data representing sound (often derived from an audio plugin or processor) and translates those numbers into dynamic graphics.

It supports two main visualization modes:

1.  **Raw Waveform:** This displays the immediate amplitude of the sound over time, showing the classic "squiggly line" shape of the audio signal. It can optionally mirror this wave vertically for a symmetrical look.
2.  **Frequency Power:** This mode analyzes the incoming audio, dividing it into several segments (like different frequency bands). It then calculates the energy within each segment and displays it as vertical bars, similar to a basic spectrum analyzer or equalizer display.

**Customization and Integration**

Since this is a UI component, its appearance is highly configurable. The class stores various colors for the background, text, and different parts of the wave (inner, middle, outer).

Crucially, the component includes a configuration system that allows external programs to dynamically:
*   Change colors and text displayed.
*   Toggle between the raw waveform and the frequency power view.
*   Link to a specific external audio source ("plugin") to retrieve the actual sound data it needs to draw.

In essence, this component bridges the gap between raw audio numbers and a dynamic graphical display, allowing developers to easily show users what their sound data "looks like."

sha: 982200a128bf9b6d294d00c69c8801f5862741b633b556ae4e953d83470dfa7f 
*/
#ifndef _UI_PIXEL_COMPONENT_SPECTROGRAM_H_
#define _UI_PIXEL_COMPONENT_SPECTROGRAM_H_

#include "audio/utils/limitBuffer.h"
#include "plugins/components/component.h"
#include "plugins/components/utils/color.h"

#include <cmath>
#include <string>
#include <vector>

/*md
## Spectrogram

Spectrogram components to display audio spectrogram.
*/

class SpectrogramComponent : public Component {
protected:
    Color bgColor;
    Color waveOut;
    Color waveMiddle;
    Color waveIn;

    Color textColor;
    std::string text;

    float* buffer;
    std::vector<float> bufferCopy;

    bool mirror = true;
    bool rawBuffer = false;

    void renderRawBuffer()
    {
        int yCenter = relativePosition.y + size.h / 2;

        int lineH = size.h / 2;
        if (mirror) {
            for (int i = 0; i < size.w; i++) {
                int graphH = bufferCopy[i] * lineH;
                if (graphH) {
                    int y1 = yCenter - graphH;
                    int y2 = yCenter + graphH;
                    draw.line({ i, y1 }, { i, y2 }, { waveIn });
                    draw.line({ i, (int)(y1 + graphH * 0.25) }, { i, (int)(y2 - graphH * 0.25) }, { waveMiddle });
                    draw.line({ i, (int)(y1 + graphH * 0.75) }, { i, (int)(y2 - graphH * 0.75) }, { waveOut });
                } else {
                    draw.pixel({ i, yCenter }, { waveOut });
                }
            }
        } else {
            for (int i = 0; i < size.w; i++) {
                int graphH = bufferCopy[i] * lineH;
                if (graphH) {
                    draw.line({ i, yCenter }, { i, yCenter + graphH }, { waveIn });
                    draw.line({ i, yCenter }, { i, (int)(yCenter + graphH * 0.25) }, { waveMiddle });
                    draw.line({ i, yCenter }, { i, (int)(yCenter + graphH * 0.75) }, { waveOut });
                } else {
                    draw.pixel({ i, yCenter }, { waveOut });
                }
            }
        }
    }

    void renderFrequencyPower()
    {
        const int segmentCount = 48; // Divide the buffer into segments
        const int samplesPerSegment = bufferCopy.size() / segmentCount;
        float energy[segmentCount] = { 0 };

        // Calculate energy for each segment
        for (int i = 0; i < segmentCount; i++) {
            for (int j = 0; j < samplesPerSegment; j++) {
                float sample = bufferCopy[i * samplesPerSegment + j];
                energy[i] += sample * sample; // Sum of squared samples
            }
            energy[i] = sqrt(energy[i] / samplesPerSegment); // Normalize by the number of samples
        }

        int bottom = relativePosition.y + size.h;
        int barWidth = size.w / segmentCount;
        // Visualize the energy as vertical bars
        for (int i = 0; i < segmentCount; i++) {
            int barHeight = (int)(energy[i] * size.h); // Scale energy to fit the display height
            int x = relativePosition.x + i * barWidth;
            int y = bottom - barHeight;
            draw.filledRect({ x, y }, { barWidth - 1, barHeight }, { waveIn });
            draw.filledRect({ x, y + (int)(barHeight * 0.10) }, { barWidth - 1, (int)(barHeight * 0.90) }, { waveOut });
        }
    }

public:
    SpectrogramComponent(ComponentInterface::Props props)
        : Component(props)
        , bgColor(styles.colors.background)
        , waveIn(styles.colors.primary)
        , waveMiddle(darken(styles.colors.primary, 0.2))
        , waveOut(darken(styles.colors.primary, 0.4))
        , textColor(lighten(styles.colors.background, 0.9))
    {
        // waveIn = rgba(35, 73, 117, 0.5);
        // waveMiddle = darken(waveIn, 0.2);
        // waveOut = darken(waveIn, 0.4);
        jobRendering = [this](unsigned long now) {
            renderNext();
        };
    }

    void render()
    {
        draw.filledRect(relativePosition, size, { bgColor });

        if (!text.empty()) {
            draw.textCentered({ relativePosition.x + (int)(size.w / 2), relativePosition.y }, text, 16, { textColor });
        }

        bufferCopy.clear();
        for (int i = 0; i < 1024; i++) {
            bufferCopy.push_back(buffer[i]);
        }
        limitBuffer(bufferCopy.data(), bufferCopy.size(), 0.9f);
        if (rawBuffer) {
            renderRawBuffer();
        } else {
            renderFrequencyPower();
        }
    }

    /*md **Config**: */
    bool config(char* key, char* value)
    {
        /*md - `BACKGROUND_COLOR: color` is the background color of the component. */
        if (strcmp(key, "BACKGROUND_COLOR") == 0) {
            bgColor = draw.getColor(value);
            return true;
        }

        /*md - `WAVE_COLOR: color` is the color of the wave. */
        if (strcmp(key, "WAVE_COLOR") == 0) {
            waveIn = draw.getColor(value);
            waveMiddle = darken(waveIn, 0.2);
            waveOut = darken(waveIn, 0.4);
            return true;
        }

        /*md - `WAVE_COLOR_OUT: color` is the color of the wave. */
        if (strcmp(key, "WAVE_COLOR_OUT") == 0) {
            waveOut = draw.getColor(value);
            return true;
        }

        /*md - `WAVE_COLOR_MIDDLE: color` is the color of the wave. */
        if (strcmp(key, "WAVE_COLOR_MIDDLE") == 0) {
            waveMiddle = draw.getColor(value);
            return true;
        }

        /*md - `WAVE_COLOR_IN: color` is the color of the wave. */
        if (strcmp(key, "WAVE_COLOR_IN") == 0) {
            waveIn = draw.getColor(value);
            return true;
        }

        /*md - `TEXT_COLOR: color` is the color of the text. */
        if (strcmp(key, "TEXT_COLOR") == 0) {
            textColor = draw.getColor(value);
            return true;
        }

        /*md - `TEXT: text` is the text to display. */
        if (strcmp(key, "TEXT") == 0) {
            text = value;
            return true;
        }

        /*md - `MIRROR: false` mirror the waveform horizontally (default: true). */
        if (strcmp(key, "MIRROR") == 0) {
            mirror = strcmp(value, "true") == 0;
            return true;
        }

        /*md - `RAW_BUFFER: true` display the raw buffer (default: false). */
        if (strcmp(key, "RAW_BUFFER") == 0) {
            rawBuffer = strcmp(value, "true") == 0;
            return true;
        }

        if (strcmp(key, "DATA") == 0) {
            AudioPlugin* plugin = NULL;
            plugin = &getPlugin(strtok(value, " "), track);

            uint8_t dataId = plugin->getDataId(strtok(NULL, " "));
            buffer = (float*)plugin->data(dataId);
            return true;
        }

        return false;
    }
};

#endif
