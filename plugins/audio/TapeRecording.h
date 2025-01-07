#ifndef _TAPE_RECORDING_H_
#define _TAPE_RECORDING_H_

#include "audioPlugin.h"
#include "mapping.h"

/*md
## TapeRecording

TapeRecording plugin is used to record audio buffer for a given track.
*/
class TapeRecording : public Mapping {
public:
    /*md **Values**: */
    ///*/md - `VOLUME` to set volume. */
    // Val& volume = val(100.0f, "VOLUME", { "Volume", .unit = "%" }, [&](auto p) { setVolumeWithGain(p.value, gain.get()); });

    TapeRecording(AudioPlugin::Props& props, char* _name)
        : Mapping(props, _name)
    {
        initValues();
    }

    void sample(float* buf)
    {
        // buf[track] = buf[track];
    }
};

#endif
