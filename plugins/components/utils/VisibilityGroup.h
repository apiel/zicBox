#pragma once

#include <cstdint>
#include <string.h>
#include <string>
#include <vector>

class VisibilityGroup {
public:
    enum Condition {
        SHOW_WHEN,
        SHOW_WHEN_NOT,
        SHOW_WHEN_OVER,
        SHOW_WHEN_UNDER
    } cond
        = SHOW_WHEN;

    struct GroupCondition {
        int8_t index;
        Condition cond;
        bool isVisible = true;
    };

    std::vector<GroupCondition> groupConditions;
    bool visible = true;

    bool onGroupChanged(int8_t index)
    {
        bool prev = visible;
        visible = true;
        for (auto& cond : groupConditions) {
            if (cond.cond == SHOW_WHEN_NOT) {
                cond.isVisible = index != cond.index;
            } else if (cond.cond == SHOW_WHEN_OVER) {
                cond.isVisible = index > cond.index;
            } else if (cond.cond == SHOW_WHEN_UNDER) {
                cond.isVisible = index < cond.index;
            } else {
                cond.isVisible = index == cond.index;
            }
            visible = visible && cond.isVisible;
        }
        return prev != visible;
    }

    bool config(char* key, char* value)
    {
        if (strcmp(key, "VISIBILITY_GROUP") == 0) {
            GroupCondition context;
            std::string condStr = strtok(value, " ");
            context.index = atoi(strtok(NULL, " "));
            if (condStr == "SHOW_WHEN_NOT") {
                context.cond = SHOW_WHEN_NOT;
            } else if (condStr == "SHOW_WHEN_OVER") {
                context.cond = SHOW_WHEN_OVER;
            } else if (condStr == "SHOW_WHEN_UNDER") {
                context.cond = SHOW_WHEN_UNDER;
            } else {
                context.cond = SHOW_WHEN;
            }
            groupConditions.push_back(context);
            return true;
        }
        return false;
    }
};
