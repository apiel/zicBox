#ifndef _SYNTH_PD_H_
#define _SYNTH_PD_H_

#include "audioPlugin.h"
#include "mapping.h"

#include <libgen.h>
#include <z_libpd.h>

/*md
## SynthPd

SynthPd is a plugin to play puredata patches.

> [!NOTE]
> - `TODO` process audio input, e.g. apply effect using PD
*/

void pdprint(const char* s)
{
    if (s[0] == '\n') {
        return;
    }
    printf("[PD print] %s\n", s);
}

class SynthPd : public Mapping {
protected:
public:
    /*md **Values**: */

    SynthPd(AudioPlugin::Props& props, char* _name)
        : Mapping(props, _name)
    {
        libpd_set_printhook(pdprint);
        libpd_init();
        // 0 is input channel but ultimately we could also use pd to process input audio
        libpd_init_audio(0, props.channels, props.sampleRate);

        // compute audio    [; pd dsp 1(
        libpd_start_message(1); // one entry in list
        libpd_add_float(1.0f);
        libpd_finish_message("pd", "dsp");

        initValues();
    }

    /*md **Config**: */
    bool config(char* key, char* value) override
    {
        /*md - `OPEN_PD` open puredata patch. */
        if (strcmp(key, "OPEN_PD") == 0) {
            char* filename = basename(value);
            char* folder = dirname(value);

            debug("[PD] Opening %s in %s\n", filename, folder);
            if (!libpd_openfile(filename, folder)) {
                debug("!!!! Error: could not open puredata file %s\n", filename);
            }
            return true;
        }

        return AudioPlugin::config(key, value);
    }

    uint8_t outbufPos = -1;
    float outbuf[128];
    void sample(float* buf)
    {
        if (outbufPos > 128) {
            libpd_process_float(1, NULL, outbuf);
            outbufPos = 0;
        }

        buf[track] = outbuf[outbufPos];
        outbufPos++;
    }

    void noteOn(uint8_t note, float velocity) override
    {
        if (velocity == 0) {
            return noteOff(note, velocity);
        }
        debug("[PD] noteOn: %d %f\n", note, velocity);

        libpd_noteon(1, note, velocity * 127);
    }

    void noteOff(uint8_t note, float velocity) override
    {
        libpd_noteon(1, note, 0);
    }
};

#endif