#pragma once

#include "view.h"

class UIManagerInterface {
public:
    virtual uint8_t getSelectedEngine() = 0;
    virtual uint8_t getEngineCount() = 0;
    virtual void selectEngine(uint8_t index) = 0;
    virtual void setView(View& view) = 0;
};