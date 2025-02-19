#pragma once

#include "plugins/components/component.h"

#include <string>

class RectComponent : public Component {
protected:
    Color background;

public:
    RectComponent(ComponentInterface::Props props)
        : Component(props)
        , background(styles.colors.background)
    {
    }
    void render()
    {
        draw.filledRect(relativePosition, size, { background });
    }
};
