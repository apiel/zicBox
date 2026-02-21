#pragma once
#include <functional>
#include "plugins/audio/mapping.h"
#include "audio/MultiFx.h"

inline Val::CallbackFn valMultiFx(MultiFx& fx) 
{
    return [&fx](Val::CallbackProps p) {
        p.val.setFloat(p.value);
        fx.setEffect(static_cast<int>(p.value));
        p.val.setString(fx.getEffectName());
    };
}
