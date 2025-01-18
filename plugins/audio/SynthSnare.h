#ifndef _SYNTH_SNARE_H_
#define _SYNTH_SNARE_H_

#include "audioPlugin.h"
#include "mapping.h"

/*md
## SynthSnare

Synth engine to generate snare sounds.
*/
class SynthSnare : public Mapping {
protected:
    uint8_t baseNote = 60;
 
public:
    /*md **Values**: */
  
    /*md - `DURATION` set the duration of the envelop.*/
    Val& duration = val(100.0f, "DURATION", { "Duration", .min = 10.0, .max = 5000.0, .step = 10.0, .unit = "ms" });

    SynthSnare(AudioPlugin::Props& props, char* _name)
        : Mapping(props, _name)
    {
        initValues();
    }

    void sample(float* buf)
    {
        // some logic to generate output
        // buf[track] = output;
    }

    void noteOn(uint8_t note, float _velocity) override
    {
    }
};

#endif
