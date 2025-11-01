#pragma once

#include "view.h"

class UIManagerInterface {
public:
    virtual int8_t getSelectedEngine() = 0;
    virtual int8_t getEngineCount() = 0;
    virtual void selectEngine(int8_t index) = 0;
    virtual void setView(View& view) = 0;
};