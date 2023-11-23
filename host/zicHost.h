#ifndef _ZIC_HOST_H_
#define _ZIC_HOST_H_

#include "audioPluginHandler.h"
#include "config.h"
#include "def.h"

AudioPluginHandlerInterface* initHost(const char* configPath = "./config.cfg")
{
    loadHostConfig(configPath);
    return &AudioPluginHandler::get();
}

#endif
