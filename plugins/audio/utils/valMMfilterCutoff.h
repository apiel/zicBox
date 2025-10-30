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
