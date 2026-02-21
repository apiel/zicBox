#pragma once
#include <functional>
#include "plugins/audio/mapping.h"
#include "audio/MFx.h"

// auto valMFx(MFx& fx) 
// {
//     return [&fx](Val::CallbackProps& p) {
//         p.val.setFloat(p.value);
//         fx.setEffect(static_cast<int>(p.value));
//         p.val.setString(fx.getEffectName());
//     };
// }

inline Val::CallbackFn valMFx(MFx& fx) 
{
    return [&fx](Val::CallbackProps p) {
        p.val.setFloat(p.value);
        fx.setEffect(static_cast<int>(p.value));
        p.val.setString(fx.getEffectName());
    };
}

// inline Val::CallbackFn valMFx(MFx& fx) 
// {
//     auto callback = [&fx](Val::CallbackProps& p) {
//         p.val.setFloat(p.value);
//         fx.setEffect(static_cast<int>(p.value));
//         p.val.setString(fx.getEffectName());
//     };
//     return callback;
// }

// inline Val::CallbackFn valMFx(MFx& fx) 
// {
//     // Remove the '&' from CallbackProps
//     return [&fx](Val::CallbackProps p) {
//         p.val.setFloat(p.value);
//         fx.setEffect(static_cast<int>(p.value));
//         p.val.setString(fx.getEffectName());
//     };
// }

// inline Val::CallbackFn valMFx(MFx& fx) 
// {
//     return Val::CallbackFn([&fx](auto& p) {
//         p.val.setFloat(p.value);
//         fx.setEffect(static_cast<int>(p.value));
//         p.val.setString(fx.getEffectName());
//     });
// }