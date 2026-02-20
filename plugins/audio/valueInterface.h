/** Description:
This C++ header file establishes a standardized framework for managing configurable values or settings within a software application. It acts as a universal blueprint for any object that needs to represent a measurable, adjustable, or displayable parameter, such as a volume slider, a sensor reading, or a temperature limit.

The system uses specific flags to categorize how a value behaves (e.g., if it's a simple number or a text field) and how it should respond when adjusted (e.g., stepping linearly, or changing exponentially).

The core of the framework is the `ValueInterface` blueprint. Any concrete setting in the program must adhere to this blueprint, ensuring consistency.

**How it works:**

1.  **Properties:** Each value is defined by a `Props` structure containing essential metadata: its visible label, minimum and maximum limits, the size of adjustment steps, and the unit it represents (like "Hz" or "meters").
2.  **Access and Modification:** The blueprint defines functions that allow the program to retrieve the current setting, change it directly, or adjust it incrementally (step up or down). It also provides methods to handle the value as a percentage of its total range, simplifying user interface design.
3.  **Graphing and Callbacks:** A special function type is included for defining non-linear behavior or plotting the value. Crucially, the system allows other parts of the program to register a "callback," meaning they are immediately notified and can react whenever the value changes.

This approach ensures that regardless of whether a setting controls volume or brightness, it can be managed, displayed, and adjusted using the same reliable set of functions.

sha: b61bf9b51c4f0ce1d0bbc3f39a159b3315eaab7209e9fb12280f3de85f78dca7 
*/
#ifndef _MAPPING_INTERFACE_H_
#define _MAPPING_INTERFACE_H_

#include <functional>
#include <stdint.h>
#include <string>

#include "audio/engines/EngineParam.h"

typedef std::function<float (float)> GraphPointFn;

class ValueInterface {
public:
    struct Props {
        std::string label = "";
        uint8_t type = VALUE_BASIC;
        float min = 0.0f;
        float max = 100.0f;
        float step = 1.00f;
        int8_t floatingPoint = -1;
        std::string unit = "";
        uint8_t incType = INC_BASIC;
        GraphPointFn graph = NULL;
    };

    virtual std::string key() = 0;
    virtual Props& props() = 0;
    virtual std::string label() = 0;
    virtual inline float get() = 0;
    virtual void increment(int8_t steps) = 0;
    virtual inline float pct() = 0;
    virtual void setPct(float pct) = 0;
    virtual std::string string() = 0;
    virtual void set(float value, void* data = NULL) = 0;
    virtual void setOnUpdateCallback(std::function<void(float, void*)> callback, void* data) = 0;
    virtual void checkForUpdate() = 0;
    virtual void copy(ValueInterface* val) { };

    bool hasType(ParamType type)
    {
        return (props().type & type) != 0;
    }

    bool hasAllTypes(uint8_t types)
    {
        return (props().type & types) == types;
    }
};

#endif