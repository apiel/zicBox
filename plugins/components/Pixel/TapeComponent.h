#ifndef _UI_PIXEL_COMPONENT_TAPE_H_
#define _UI_PIXEL_COMPONENT_TAPE_H_

#include "plugins/components/component.h"

#include <string>
#include <sndfile.h>

/*md
## Tape

Tape components to draw a rectangle.
*/

class TapeComponent : public Component {
protected:
    Color background;

    std::string folder = "tape";
    std::string filename = "track";
    SNDFILE* sndfile = nullptr;
    SF_INFO sfinfo;

    std::vector<float> buffer;

public:
    TapeComponent(ComponentInterface::Props props)
        : Component(props)
        , background(styles.colors.background)
    {
    }
    void render()
    {
        if (updatePosition()) {
            draw.filledRect(relativePosition, size, { background });
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
            return true;
        }

        return false;
    }
};

#endif
