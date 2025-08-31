#pragma once

#include <functional>
#include <math.h>
#include <stdint.h>
#include <vector>

#include "audioPlugin.h"
#include "helpers/range.h"
#include "log.h"

struct DataFn {
    std::string name;
    std::function<void*(void*)> fn;
};

#define DEFINE_GETDATAID_AND_DATA                                                   \
    static const int DATA_COUNT = sizeof(dataFunctions) / sizeof(dataFunctions[0]); \
    uint8_t getDataId(std::string name) override                                    \
    {                                                                               \
        for (size_t i = 0; i < DATA_COUNT; ++i) {                                   \
            if (name == dataFunctions[i].name) {                                    \
                return static_cast<uint8_t>(i);                                     \
            }                                                                       \
        }                                                                           \
        return static_cast<uint8_t>(atoi(name.c_str()));                            \
    }                                                                               \
    void* data(int id, void* userdata = nullptr) override                           \
    {                                                                               \
        if (id >= static_cast<int>(DATA_COUNT)) {                                   \
            return nullptr;                                                         \
        }                                                                           \
        return dataFunctions[id].fn(userdata);                                      \
    }

class Val : public ValueInterface {
protected:
    float value_f;
    float value_pct;
    std::string value_s;

    std::function<void(float, void*)> onUpdateFn = [](float, void* data) { };
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

    void copy(ValueInterface* val) override
    {
        value_f = val->get();
        value_pct = val->pct();
        value_s = val->string();
        _props = val->props();
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
        if (_props.incType & INC_ONE_BY_ONE != 0) {
            steps = steps > 0 ? 1 : -1;
        }
        if (_props.incType & INC_EXP != 0) {
            // use _props.step for base
            float base = _props.step == 1.0f ? 2.0f : _props.step;
            float incVal = log(get()) / log(base);
            incVal += steps;
            set(pow(base, incVal));
            return;
        }
        if (_props.incType & INC_MULT != 0) {
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
        onUpdateFn(value, onUpdateData);
    }

    void setPct(float pct)
    {
        float value = range(pct * (_props.max - _props.min) + _props.min, _props.min, _props.max);
        set(value);
    }

    void setOnUpdateCallback(std::function<void(float, void*)> callback, void* data)
    {
        onUpdateFn = callback;
        onUpdateData = data;
    }

    float pct()
    {
        return value_pct;
    }

    void checkForUpdate() { }
};

class Mapping : public AudioPlugin {
public:
    std::vector<ValueInterface*> mapping;

protected:
    Val& val(float initValue, std::string _key, ValueInterface::Props props = {}, std::function<void(Val::CallbackProps)> _callback = NULL)
    {
        Val* v = new Val(initValue, _key, props, _callback);
        mapping.push_back(v);
        // debug("-------- Mapping: %s\n", v->key());
        return *v;
    }

public:
    Mapping(AudioPlugin::Props& props, AudioPlugin::Config& config, std::vector<ValueInterface*> mapping = {})
        : AudioPlugin(props, config)
        , mapping(mapping)
    {
    }

    void val(ValueInterface* value)
    {
        mapping.push_back(value);
    }

    void initValues()
    {
        for (ValueInterface* val : mapping) {
            val->set(val->get());
        }
    }

    void initValues(std::vector<ValueInterface*> skips)
    {
        for (ValueInterface* val : mapping) {
            for (auto skip : skips) {
                if (val->key() == skip->key()) {
                    continue;
                }
            }
            val->set(val->get());
        }
    }

    int getValueIndex(std::string key) override
    {
        for (int i = 0; i < mapping.size(); i++) {
            if (mapping[i]->key() == key) {
                return i;
            }
        }
        return -1;
    }

    int getValueCount() override
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

    ValueInterface* getValue(std::string key) override
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

    void serializeJson(nlohmann::json& json) override
    {
        // Use array because order matter
        nlohmann::json values = nlohmann::json::array();
        for (ValueInterface* val : mapping) {
            values.push_back({ { "key", val->key() }, { "value", val->get() } });
        }
        json["values"] = values;
    }

    void hydrateJson(nlohmann::json& json) override
    {
        if (json.contains("values")) {
            auto& values = json["values"];
            for (int i = 0; i < values.size(); i++) {
                std::string key = values[i]["key"];
                ValueInterface* val = getValue(key);
                if (val) {
                    float value = values[i]["value"];
                    // logDebug("hydrate value %s %f", values[i]["key"].get<std::string>().c_str(), value);
                    val->set(value);
                }
            }
        }
    }
};
