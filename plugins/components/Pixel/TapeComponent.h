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

    int start = 0;
    int beatLength = 4;

    void loadAudioFile()
    {
        std::string filepath = folder + "/tmp/" + filename + ".wav";
        sndfile = sf_open(filepath.c_str(), SFM_READ, &sfinfo);

        // here should be some magic to load the file into buffer, generating the 
        // waveform taking into account the start and beatLength, the available width with size.w
        // and the audio samplerate...
    }

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
            // here will be the code to draw the waveform
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
