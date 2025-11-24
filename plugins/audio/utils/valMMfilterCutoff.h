/** Description:
This file serves as a crucial link between a graphical user interface (like a virtual knob or slider) and the internal workings of an audio processing system. Specifically, it defines how a user controls the "cutoff" setting of a unique audio element called an "MM filter."

### How It Works

The function acts as a translator, taking a basic input value from the user interface and applying it to the audio filter while providing descriptive feedback.

1.  **Input Translation:** When a user adjusts a control on the screen, the function receives this raw numerical input. It processes this number and normalizes it into a specialized range, typically centered at zero (e.g., from -100% to +100%).

2.  **Dual Filter Control:** This design cleverly maps two different filter behaviors onto a single control:
    *   If the user pushes the control towards one extreme (the negative side), the function applies a **Low Pass (LP)** filter, which allows low frequencies to pass through.
    *   If the user pushes the control towards the other extreme (the positive side), it applies a **High Pass (HP)** filter, which allows high frequencies to pass through.

3.  **Applying the Effect:** The calculated percentage and type are immediately applied to the audio filter object, adjusting the sound output in real-time.

4.  **User Feedback:** The most important visible feature is the clear feedback provided. The function generates a descriptive string (e.g., "LP 65%" or "HP 40%") and sends it back to the user interface, ensuring the user always knows the exact filter type and intensity currently active.

sha: 899c2306c13a1258cc0a61da5f68cdfe846c9223ace897b787e8d7ec61c72b7f 
*/
#pragma once

#include "plugins/audio/mapping.h"
#include "audio/MMfilter.h"

void valMMfilterCutoff(Val::CallbackProps& p, MMfilter& filter)
{
    p.val.setFloat(p.value);
    float amount = p.val.pct() * 2 - 1.0f;

    char strBuf[128];
    filter.setCutoff(amount);
    if (amount > 0.0) {
        sprintf(strBuf, "HP %d%%", (int)(amount * 100));
    } else {
        sprintf(strBuf, "LP %d%%", (int)((-amount) * 100));
    }
    p.val.setString(strBuf);
}
