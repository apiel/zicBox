#pragma once

#include "plugins/audio/mapping.h"
#include "audio/MFx.h"

void valMFx(Val::CallbackProps& p, MFx& fx)
{
    p.val.setFloat(p.value);
    fx.setEffect(static_cast<int>(p.value));
    p.val.setString(fx.getEffectName());
}
