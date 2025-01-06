#ifndef _UI_PIXEL_BASE_COMPONENT_SAMPLE_POSITION_H_
#define _UI_PIXEL_BASE_COMPONENT_SAMPLE_POSITION_H_

#include "plugins/components/component.h"
#include "plugins/components/utils/color.h"
#include <string>

class BaseSamplePositionComponent : public Component {
protected:
    bool jobDidRender = false;

    Color sampleColor;
public:
    struct SampleState {
        float position;
        int index;
        float release;
    };

    BaseSamplePositionComponent(ComponentInterface::Props props)
        : Component(props)
        , sampleColor(lighten({ 0x9d, 0xfe, 0x86 }, 0.3)) // #9dfe89
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
            Color color = sampleColor;
            if (sample.release != 1.0) {
                color = alpha(color, sample.release);
            }
            draw.filledRect({ x, y + (sample.index * spacing % (size.h - 10)) }, { 4, 4 }, { color });
        }
    }

    bool config(char* key, char* value)
    {
        if (strcmp(key, "SAMPLE_COLOR") == 0) {
            sampleColor = draw.getColor(value);
            return true;
        }

        return false;
    }
};

#endif
