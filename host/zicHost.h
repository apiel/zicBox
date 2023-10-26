#ifndef _ZIC_HOST_H_
#define _ZIC_HOST_H_

#include "audioPluginHandler.h"
#include "config.h"
#include "def.h"

AudioPluginHandlerInterface* initHost()
{
    // TODO make config.cfg a parameter
    // make default config
    if (!loadHostConfig()) {
        return NULL;
    }
    return &AudioPluginHandler::get();
}

#endif
