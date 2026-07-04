/** Description:
## SynthPd: A Pure Data Integration Plugin

This file describes a software component, `SynthPd`, which acts as a bridge, allowing a music application (or device) to run patches created in Pure Data (PD). Pure Data is a graphical programming environment commonly used for creating custom audio effects, synthesizers, and real-time processing tools.

### Core Functionality

`SynthPd` embeds the Pure Data audio engine directly into the application. Its main job is to initialize this engine and manage the flow of data between the musical host and the loaded PD patch.

1.  **Audio Generation:** The plugin handles the real-time processing loop, asking the embedded PD engine to calculate the resulting sound output based on current inputs.
2.  **MIDI Control:** It translates standard musical commands into PD messages:
    *   **Notes:** When a user plays a key (Note On/Off), the corresponding MIDI message is sent to the PD patch to trigger or stop a sound.
    *   **Continuous Controllers (CC):** Knobs or sliders from the host application are mapped to PD's control inputs, allowing real-time modulation of parameters.

### Configuration and Customization

Users must configure the plugin to specify which patch to run and how to control it:

*   **Loading Patches:** A specific command allows the user to define the file path of the desired Pure Data patch to open.
*   **Parameter Mapping:** Users can assign specific software controls or MIDI CC numbers to parameters within the patch. This allows for complex control over parameters beyond standard MIDI controls, using custom float messages inside the PD environment.

### Technical Context

This plugin relies on the **libpd** library, which is the necessary component for embedding the Pure Data environment. Due to installation requirements, especially on platforms like the Raspberry Pi, users may need to manually install this library before compiling and running the `SynthPd` plugin.

sha: 5b0ae02638d7fd2d79dcc5895c57c4da652f276c03aa5257957b5a3789a00162 
*/
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
> - `FIXME` `apt install libpd` doesnt work on RPi. Need to embded libpd or to find a artifactory to download.

On Raspberry, libpd is not available out of the box, this is why SynthPd has been commented out from makefile. To use it, you need to install [libpd](https://github.com/libpd/libpd) manually.

On ubuntu, you can directly install it from the artifactory:
```sh
sudo apt-get install libpd-dev libpd0
```

Finally, you need to compile SynthPd:
```sh
make -C plugins/audio SynthPd
```

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
    SynthPd(AudioPlugin::Props& props, AudioPlugin::Config& config)
        : Mapping(props, config)
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

    void noteOn(uint8_t note, float velocity, void* userdata = NULL) override
    {
        if (velocity == 0) {
            return noteOff(note, velocity);
        }
        debug("[PD] noteOn: %d %f\n", note, velocity);

        libpd_noteon(0, note, velocity * 127);
    }

    void noteOff(uint8_t note, float velocity, void* userdata = NULL) override
    {
        libpd_noteon(0, note, 0);
    }
};

#endif