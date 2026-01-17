#pragma once

#include "libs/nlohmann/json.hpp"

#include "plugins/components/EventInterface.h"

class DrawRenderer {
public:
    virtual void init() { }
    virtual void quit() { }
    virtual void render() = 0;
    virtual void preRender(EventInterface* view, uint64_t now) { }

    virtual void config(nlohmann::json& config) { }

    virtual bool handleEvent(EventInterface* view) { return true; }

    virtual Point getWindowPosition() { return { -1, -1 }; }
    virtual Size getWindowSize() { return { -1, -1 }; }
};
