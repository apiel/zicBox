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
    };

    const char* _key;
    std::function<void(CallbackProps)> callback;

    Val(float initValue, const char* _key, ValueInterface::Props props = {}, std::function<void(CallbackProps)> _callback = NULL)
        : _props(props)
        , _key(_key)
        , callback(_callback)
    {
        if (callback == NULL) {
            callback = [this](auto p) { setFloat(p.value); };
        }
        setFloat(initValue);
    }

    ValueInterface::Props& props()
    {
        return _props;
    }

    const char* key()
    {
        return _key;
    }

    const char* label()
    {
        return _props.label ? _props.label : _key;
    }

    inline float get()
    {
        return value_f;
    }

    void increment(int8_t steps)
    {
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
        callback({ value, data });
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

    Val& val(float initValue, const char* _key, ValueInterface::Props props = {}, std::function<void(Val::CallbackProps)> _callback = NULL)
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

    int getValueIndex(const char* key)
    {
        for (int i = 0; i < mapping.size(); i++) {
            if (strcmp(mapping[i]->key(), key) == 0) {
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

    ValueInterface* getValue(const char* key)
    {
        for (int i = 0; i < mapping.size(); i++) {
            if (strcmp(mapping[i]->key(), key) == 0) {
                return mapping[i];
            }
        }
        return NULL;
    }
};

#endif