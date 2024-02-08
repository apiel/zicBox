#ifndef _MAPPING_H_
#define _MAPPING_H_

#include <functional>
#include <math.h>
#include <stdint.h>
#include <vector>

#include "../../helpers/range.h"

#include "audioPlugin.h"

class Val : public ValueInterface {
protected:
    float value_f;
    float value_pct;
    std::string value_s;

    void (*onUpdatePtr)(float, void* data) = [](float, void* data) {};
    void* onUpdateData = NULL;

    ValueInterface::Props _props;

public:
    struct CallbackProps {
        float value;
        void* data = NULL;
        Val& val;
    };

    std::string _key;
    std::function<void(CallbackProps)> callback;

    Val(float initValue, std::string _key, ValueInterface::Props props = {}, std::function<void(CallbackProps)> _callback = NULL)
        : _props(props)
        , _key(_key)
        , callback(_callback)
    {
        if (_props.label == "") {
            _props.label = _key;
        }
        if (callback == NULL) {
            callback = [this](auto p) { setFloat(p.value); };
        }
        setFloat(initValue);
    }

    ValueInterface::Props& props()
    {
        return _props;
    }

    std::string key()
    {
        return _key;
    }

    std::string label()
    {
        return _props.label;
    }

    inline float get()
    {
        return value_f;
    }

    void increment(int8_t steps)
    {
        if (_props.incrementationType == VALUE_INCREMENTATION_EXP) {
            // use _props.step for base
            float base = _props.step == 1.0f ? 2.0f : _props.step;
            float incVal = log(get()) / log(base);
            incVal += steps;
            set(pow(base, incVal));
            return;
        }
        if (_props.incrementationType == VALUE_INCREMENTATION_MULT) {
            float mult = (_props.step == 1.0f ? 1.1f : _props.step) * abs(steps);
            if (steps < 0) {
                set(get() / mult);
            } else {
                set(get() * mult);
            }
            return;
        }
        set(get() + ((float)steps * _props.step));
    }

    std::string string()
    {
        return value_s;
    }

    void setString(std::string value)
    {
        value_s = value;
    }

    void setFloat(float value)
    {
        value_f = range(value, _props.min, _props.max);
        value_pct = (value_f - _props.min) / (_props.max - _props.min);
    }

    void set(float value, void* data = NULL)
    {
        callback({ value, data, *this });
        (*onUpdatePtr)(value, onUpdateData);
    }

    void onUpdate(void (*callback)(float, void* data), void* data)
    {
        onUpdatePtr = callback;
        onUpdateData = data;
    }

    float pct()
    {
        return value_pct;
    }

    void checkForUpdate() { }
};

class Mapping : public AudioPlugin {
protected:
    std::vector<ValueInterface*> mapping;

    Val& val(float initValue, std::string _key, ValueInterface::Props props = {}, std::function<void(Val::CallbackProps)> _callback = NULL)
    {
        Val* v = new Val(initValue, _key, props, _callback);
        mapping.push_back(v);
        // debug("-------- Mapping: %s\n", v->key());
        return *v;
    }

public:
    Mapping(AudioPlugin::Props& props, char* _name, std::vector<ValueInterface*> mapping = {})
        : AudioPlugin(props, _name)
        , mapping(mapping)
    {
    }

    void initValues()
    {
        for (ValueInterface* val : mapping) {
            val->set(val->get());
        }
    }

    int getValueIndex(std::string key)
    {
        for (int i = 0; i < mapping.size(); i++) {
            if (mapping[i]->key() == key) {
                return i;
            }
        }
        return -1;
    }

    int getValueCount()
    {
        return mapping.size();
    }
    ValueInterface* getValue(int valueIndex)
    {
        if (valueIndex < 0 || valueIndex >= mapping.size()) {
            return NULL;
        }
        return mapping[valueIndex];
    }

    ValueInterface* getValue(std::string key)
    {
        for (int i = 0; i < mapping.size(); i++) {
            if (mapping[i]->key() == key) {
                return mapping[i];
            }
        }
        printf("!!!!!!!! getValue not found: %s\n", key.c_str());
        // for (int i = 0; i < mapping.size(); i++) {
        //     printf("--> %s\n", mapping[i]->key().c_str());
        // }
        return NULL;
    }

    void serialize(FILE* file, std::string separator)
    {
        for (ValueInterface* val : mapping) {
            fprintf(file, "%s %f%s", val->key().c_str(), val->get(), separator.c_str());
        }
    }

    void hydrate(std::string value)
    {
        char* key = strtok((char*)value.c_str(), " ");
        float fValue = strtof(strtok(NULL, " "), NULL);
        ValueInterface* val = getValue(key);
        if (val) {
            val->set(fValue);
        }
    }
};

#endif