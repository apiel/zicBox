#ifndef _UI_PIXEL_COMPONENT_RECT_H_
#define _UI_PIXEL_COMPONENT_RECT_H_

#include "plugins/components/component.h"

#include <string>

/*md
## Rect

<img src="https://raw.githubusercontent.com/apiel/zicBox/main/plugins/components/Pixel/rect.png" />

Rect components to draw a rectangle.
*/

class RectComponent : public Component {
protected:
    Color color;

    bool filled = true;

public:
    RectComponent(ComponentInterface::Props props)
        : Component(props)
        , color(styles.colors.background)
    {
        /*md
**Config**:
```tsx
<Text
        */
       nlohmann::json config = props.config;

        /*md   // The color of the rectangle. */
        /*md   color="#000000" */
        if (config.contains("color")) {
            color = draw.getColor((char *)config["color"].get<std::string>().c_str());
        }

        /*md   // If true, the rectangle will be filled. Default is true. */
        /*md   filled={false} */
        filled = config.value("filled", filled);

        /*md
/>
```
        */
    }
    void render()
    {
        if (updatePosition()) {
            if (filled) {
                draw.filledRect(relativePosition, size, { color });
            } else {
                draw.rect(relativePosition, size, { color });
            }
        }
    }
};

#endif
