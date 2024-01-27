#ifndef _SYNTH_PD_H_
#define _SYNTH_PD_H_

#include "audioPlugin.h"
#include "mapping.h"

#include <z_libpd.h>
#include <libgen.h>

/*md
## SynthPd

SynthPd is a plugin to play puredata patches.

> [!NOTE]
> - `TODO` process audio input, e.g. apply effect using PD
*/
class SynthPd : public Mapping {
protected:
public:
    /*md **Values**: */

    SynthPd(AudioPlugin::Props& props, char* _name)
        : Mapping(props, _name)
    {
        libpd_init();
        // 0 is input channel but ultimately we could also use pd to process input audio
        libpd_init_audio(0, props.channels, props.sampleRate);

        initValues();
    }

    /*md **Config**: */
    bool config(char* key, char* value) override
    {
        /*md - `OPEN_PD` open puredata patch. */
        if (strcmp(key, "OPEN_PD") == 0) {
            char *filename = basename(value);
            char *folder = dirname(value);

            printf("opening %s in %s\n", filename, folder);

            // if (!libpd_openfile(argv[1], argv[2])) {
            //     debug("Error: could not open puredata file %s\n", filename);
            // }
            return true;
        }

        return AudioPlugin::config(key, value);
    }

    void sample(float* buf)
    {
        float out = 0.0f;

        buf[track] = out;
    }

    void noteOn(uint8_t note, float velocity) override
    {
        if (velocity == 0) {
            return noteOff(note, velocity);
        }
    }

    void noteOff(uint8_t note, float velocity) override
    {
    }
};

#endif