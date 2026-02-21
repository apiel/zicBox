#pragma once
#include <functional>
#include "plugins/audio/mapping.h"
#include "audio/MFx.h"

inline Val::CallbackFn valMFx(MFx& fx) 
{
    return [&fx](Val::CallbackProps p) {
        p.val.setFloat(p.value);
        fx.setEffect(static_cast<int>(p.value));
        p.val.setString(fx.getEffectName());
    };
}
