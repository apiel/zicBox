#ifndef _SYNTH_PD_H_
#define _SYNTH_PD_H_

#include "audioPlugin.h"
#include "mapping.h"

#include <libgen.h>
#include <z_libpd.h>

/*md
## SynthPd

SynthPd is a plugin to play puredata patches.

Patch use midi input `notein` to trigger notes. To modulate parameters, you can either use midi cc `ctlin` or float message `r KEY_MSG`.

<img src="https://raw.githubusercontent.com/apiel/zicBox/main/plugins/audio/SynthPd.png" />

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
public:
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
        /*md - `OPEN_PD: filename` open puredata patch. */
        if (strcmp(key, "OPEN_PD") == 0) {
            char* filename = basename(value);
            char* folder = dirname(value);

            debug("[PD] Opening %s in %s\n", filename, folder);
            if (!libpd_openfile(filename, folder)) {
                debug("!!!! Error: could not open puredata file %s\n", filename);
            }
            return true;
        }

        /*md - `ASSIGN_CC: cc label default_val` assign CC value to be sent to pd `ctlin cc`. To use value in the UI, use `CC_1`, `CC_2`, ... */
        if (strcmp(key, "ASSIGN_CC") == 0) {
            uint8_t cc = atoi(strtok(value, " "));
            char* label = strtok(NULL, " ");
            uint8_t defaultVal = atoi(strtok(NULL, " "));
            val(defaultVal, "CC_" + std::to_string(cc), { label, .max = 127 }, [&, cc](auto p) {
                p.val.setFloat(p.value);
                libpd_controlchange(0, cc, p.val.get());
            });
            return true;
        }

        /*md - `ASSIGN_FLOAT: key label default_val` assign float value to be sent to pd `r key`. To use value in the UI, use the key as reference. So if you pd receiver is `r FREQ`, then the config is `ASSIGN_FLOAT FREQ Frequency 440` and the value key is `FREQ`. */
        if (strcmp(key, "ASSIGN_FLOAT") == 0) {
            char* k = strtok(value, " ");
            char* label = strtok(NULL, " ");
            uint8_t defaultVal = atof(strtok(NULL, " "));
            val(defaultVal, k, { label }, [&](auto p) {
                p.val.setFloat(p.value);
                libpd_start_message(1);
                libpd_add_float(p.val.get());
                libpd_finish_message(p.val.key().c_str(), "float");
            });
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

        libpd_noteon(0, note, velocity * 127);
    }

    void noteOff(uint8_t note, float velocity) override
    {
        libpd_noteon(0, note, 0);
    }
};

#endif