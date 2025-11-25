/** Description:
This specialized utility, named `VisibilityGroup`, acts like a sophisticated switchboard for determining whether a certain element or panel should be displayed. It manages dynamic visibility based on a set of predefined logical rules.

The core mechanism relies on comparing an incoming numerical value (the current status or index) against several stored reference points.

**Defining the Rules:**
The class defines four types of conditions: showing the element only when the input value is **equal to**, **not equal to**, **greater than** (over), or **less than** (under) a specified reference number. The `config` function is used during setup to define these individual rules and their associated reference numbers. For an element to be visible, it must satisfy every single rule defined in this list.

**Checking the Status:**
The central function, `onGroupChanged`, is the calculation engine. When a new status index is provided, this function iterates through all configured rules. For each rule, it checks if the new index meets the requirement (e.g., is the input index greater than the rule's reference number?). It then combines the results of all individual checks. If even one rule fails, the final visibility status is set to hidden.

In summary, this class provides a clear, rule-based system for complex visibility management, ensuring an element only appears when all necessary numerical conditions are met simultaneously.

sha: df8bf53294c71d1a132846ba82fa97d8f073b4f9007d50f792479c733012ae9a 
*/
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
