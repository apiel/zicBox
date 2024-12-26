#ifndef _COMPONENT_VISIBILITY_CONTEXT_H_
#define _COMPONENT_VISIBILITY_CONTEXT_H_

#include <cstdint>
#include <string>
#include <vector>
#include <string.h>

class VisibilityContext {
public:
    enum Condition {
        SHOW_WHEN,
        SHOW_WHEN_NOT,
        SHOW_WHEN_OVER,
        SHOW_WHEN_UNDER
    } cond
        = SHOW_WHEN;

    struct ContextCondition {
        int16_t index;
        float value;
        Condition cond;
        bool isVisible = true;
    };

    std::vector<ContextCondition> contextConditions;
    bool visible = true;

    bool onContext(uint8_t index, float value)
    {
        bool update = false;
        visible = true;
        for (auto& cond : contextConditions) {
            if (index == cond.index) {
                if (cond.cond == SHOW_WHEN_NOT) {
                    cond.isVisible = value != cond.value;
                } else if (cond.cond == SHOW_WHEN_OVER) {
                    cond.isVisible = value > cond.value;
                } else if (cond.cond == SHOW_WHEN_UNDER) {
                    cond.isVisible = value < cond.value;
                } else {
                    cond.isVisible = value == cond.value;
                }
                update = true;
            }
            visible = visible && cond.isVisible;
        }
        return update;
    }

    bool config(char* key, char* value)
    {
        if (strcmp(key, "VISIBILITY_CONTEXT") == 0) {
            ContextCondition context;
            context.index = atoi(strtok(value, " "));
            std::string condStr = strtok(NULL, " ");
            context.value = atof(strtok(NULL, " "));
            if (condStr == "SHOW_WHEN_NOT") {
                context.cond = SHOW_WHEN_NOT;
            } else if (condStr == "SHOW_WHEN_OVER") {
                context.cond = SHOW_WHEN_OVER;
            } else if (condStr == "SHOW_WHEN_UNDER") {
                context.cond = SHOW_WHEN_UNDER;
            } else {
                context.cond = SHOW_WHEN;
            }
            contextConditions.push_back(context);
            return true;
        }
        return false;
    }
};

#endif