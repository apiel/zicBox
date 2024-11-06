#ifndef _UI_BASE_COMPONENT_SAMPLE_POSITION_H_
#define _UI_BASE_COMPONENT_SAMPLE_POSITION_H_

#include "../utils/inRect.h"
#include "../component.h"
#include <string>

class SamplePositionBaseComponent : public Component {
protected:
    bool jobDidRender = false;

    struct Colors {
        Color sample;
    } colors;

    Colors getColorsFromColor(Color color)
    {
        return Colors({ color });
    }

public:
    struct SampleState {
        float position;
        int index;
        float release;
    };

    SamplePositionBaseComponent(ComponentInterface::Props props)
        : Component(props)
        , colors(getColorsFromColor(lighten(draw.getColor((char*)"#9dfe86"), 0.3)))
    {
    }

    bool shouldRender(void* active)
    {
        if (active) {
            jobDidRender = true;
            return true;
        }
        if (jobDidRender) {
            jobDidRender = false;
            return true;
        }
        return false;
    }

    void render(std::vector<SampleState>* sampleStates)
    {
        int spacing = 13;
        int totalHeight = sampleStates->size() * spacing;
        int marginTop = 5;
        if (totalHeight < size.h) {
            marginTop = (size.h - totalHeight) / 2;
        }
        int y = position.y + marginTop;
        for (auto& sample : *sampleStates) {
            int x = position.x + sample.position * size.w;
            Color color = colors.sample;
            if (sample.release != 1.0) {
                color = alpha(color, sample.release);
            }
            draw.filledRect({ x, y + (sample.index * spacing % (size.h - 10)) }, { 4, 4 }, color);
        }
    }

    void render()
    {
        // Should set a pointer to plugin and sample data id...
    }

    void setColors(Color color)
    {
        colors = getColorsFromColor(color);
    }

    bool config(char* key, char* value)
    {
        if (strcmp(key, "SAMPLE_COLOR") == 0) {
            setColors(draw.getColor(value));
            return true;
        }

        return false;
    }
};

#endif
