#ifndef _UI_PIXEL_COMPONENT_FILTER_H_
#define _UI_PIXEL_COMPONENT_FILTER_H_

#include "plugins/components/component.h"
#include "helpers/range.h"

#include <string>

/*//md
## Filter

Filter components to make representation of an audio filter.
*/

class FilterComponent : public Component {
protected:
    Color background;
    std::vector<float> frequencies;

    AudioPlugin* filterPlugin = NULL;
    uint8_t filterDataId = 0;

public:
    FilterComponent(ComponentInterface::Props props)
        : Component(props)
        , background(styles.colors.background)
    {
        precomputeData();
    }

    void render() override
    {
        if (updatePosition()) {
            // Render background
            draw.filledRect(relativePosition, size, { background });

            printf("rendering filter %d\n", filterDataId);
            if (filterPlugin != NULL) {
                std::vector<Point> points;
                // Render the frequency response graph
                for (int i = 0; i < size.w; ++i) {
                    // float gain = *(float*)filterPlugin->data(filterDataId, &frequencies[i]);
                    float f = 1.0f / (1.0f + frequencies[i] / 1000.0f);
                    float gain = range(*(float*)filterPlugin->data(filterDataId, &f), 0.0, 1.0);
                    points.push_back({ relativePosition.x + i, position.y + (int)(size.h - (gain * size.h)) });
                    printf("%f,", gain);
                }
                printf("\n\n");
                draw.lines(points, { styles.colors.primary });
            }
        }
    }

    void precomputeData()
    {
        if (size.w <= 1) {
            return;
        }

        // Define frequency range
        const float startFreq = 20.0f; // Start frequency (Hz)
        const float endFreq = 20000.0f; // End frequency (Hz)

        // Precompute logarithmic frequencies
        for (int i = 0; i < size.w; ++i) {
            float t = static_cast<float>(i) / (size.w - 1);
            frequencies.push_back(std::pow(10.0f, std::log10(startFreq) + t * (std::log10(endFreq) - std::log10(startFreq))));
        }
    }

    /*//md **Config**: */
    bool config(char* key, char* value)
    {
        /*//md - `BACKGROUND_COLOR: color` is the color of the component. */
        if (strcmp(key, "BACKGROUND_COLOR") == 0) {
            background = draw.getColor(value);
            return true;
        }

        // PLUGIN: name [filterDataId]
        if (strcmp(key, "PLUGIN") == 0) {
            filterPlugin = &getPlugin(strtok(value, " "), track);
            char* dataId = strtok(NULL, " ");
            filterDataId = filterPlugin->getDataId(dataId != NULL ? dataId : "COMPUTE");

            watch(filterPlugin->getValue("CUTOFF"));
            watch(filterPlugin->getValue("RESONANCE"));

            return true;
        }

        return false;
    }
};

#endif
