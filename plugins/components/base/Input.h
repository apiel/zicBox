/** Description:
This code defines a highly customizable interactive text input field, named `Input`, designed for use in a structured user interface (UI) environment, likely on an embedded device or screen that utilizes hardware controls like rotary encoders and keypads.

**Core Purpose and Structure**

The `Input` class serves as a fundamental UI element (a "Component"). Its primary role is to capture, manage, and display text that a user is actively editing. It inherits basic functionality from a parent `Component` class, allowing it to integrate smoothly into a larger application framework.

**How Text Entry Works**

The component stores the text being edited, along with the current position of the editing cursor. It defines a comprehensive set of allowed characters—including lowercase, uppercase, numbers, and various special symbols—that the user can select from.

1.  **Visual Display:** The input field dynamically adjusts its colors for the background, general text, and the specific character currently being edited, ensuring the cursor's location is always clear to the user. It also manages custom fonts and font sizes for rendering the text.
2.  **Hardware Interaction:** Instead of a traditional keyboard, input is managed via two separate rotary encoders (dials). One encoder is dedicated to moving the cursor left or right within the text. The second encoder cycles through the predefined character set, allowing the user to select the correct letter or symbol at the cursor's position.
3.  **Keypad Actions:** The component handles standard keypad commands like clearing the entire input, performing a backspace, and a special "jump" function that quickly switches the currently selected character from one group to the next (e.g., jumping from "a" to "A" or "1").
4.  **Finalizing Input:** The class includes a mandatory function (`save`) that is called when the user successfully finishes editing the text, allowing the external system to process the final result.

sha: 23036e506fa66c62c17533b5608c48c09c2cd46c47e387a4c7f707bcf8627a9e 
*/
#pragma once

#include "plugins/components/component.h"
#include "plugins/components/utils/color.h"

#include <string>

class Input : public Component {
protected:
    Color bgColor;
    Color textColor;
    Color textEditColor;
    Color foregroundColor;

    int fontSize = 12;
    void* font = NULL;
    int cursorHeight = 2;

    int8_t valueEncoderId = 1; // encoder used to edit value / move cursor
    int8_t cursorEncoderId = 2; // encoder used to move cursor

    std::string value = "";
    int cursorPos = 0;

    std::string charsetLower = "abcdefghijklmnopqrstuvwxyz";
    std::string charsetUpper = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
    std::string charsetNum = "0123456789";
    std::string charsetSpecial = ".-_=+:!&#@$%()[]";
    std::string charset = charsetLower + charsetUpper + charsetNum + charsetSpecial;

public:
    Input(ComponentInterface::Props props)
        : Component(props, [&](std::string action) {
            std::function<void(KeypadLayout::KeyMap&)> func = NULL;

            if (action == ".clear") {
                func = [this](KeypadLayout::KeyMap& keymap) {
                    if (KeypadLayout::isReleased(keymap)) {
                        value = "";
                        cursorPos = 0;
                        renderNext();
                    }
                };
            }
            if (action == ".backspace") {
                func = [this](KeypadLayout::KeyMap& keymap) {
                    if (KeypadLayout::isReleased(keymap)) {
                        if (!value.empty() && cursorPos > 0) {
                            value.erase(cursorPos - 1, 1); // remove 1 character at cursorPos - 1
                            cursorPos--; // move cursor back
                            renderNext();
                        }
                    }
                };
            }
            if (action == ".jump") {
                func = [this](KeypadLayout::KeyMap& keymap) {
                    if (KeypadLayout::isReleased(keymap)) {
                        // jump to next character group (abc, ABC, 012, !@#)
                        char c = value[cursorPos];
                        size_t pos = charset.find(c);
                        // logDebug("pos: %d charsetLen: %d npos: %d", pos, (int)charset.size(), (int)pos);
                        if (pos == std::string::npos) {
                            value.push_back(charset[0]);
                        } else {
                            if (pos < charsetLower.size()) {
                                pos = charsetLower.size();
                            } else if (pos < charsetLower.size() + charsetUpper.size()) {
                                pos = charsetLower.size() + charsetUpper.size();
                            } else if (pos < charsetLower.size() + charsetUpper.size() + charsetNum.size()) {
                                pos = charsetLower.size() + charsetUpper.size() + charsetNum.size();
                            } else {
                                pos = 0;
                            }
                            value[cursorPos] = charset[pos];
                        }
                        renderNext();
                    }
                };
            }
            if (action == ".save") {
                func = [this](KeypadLayout::KeyMap& keymap) {
                    if (KeypadLayout::isReleased(keymap)) {
                        save(value);
                    }
                };
            }
            return func;
        })
        , bgColor(styles.colors.background)
        , textColor(styles.colors.text)
        , textEditColor(darken(styles.colors.text, 0.5))
        , foregroundColor(lighten(styles.colors.background, 0.5))
    {
        nlohmann::json& config = props.config;
        bgColor = draw.getColor(config["bgColor"], bgColor);
        textColor = draw.getColor(config["textColor"], textColor);
        foregroundColor = draw.getColor(config["foregroundColor"], foregroundColor);

        if (config.contains("font")) {
            font = draw.getFont(config["font"].get<std::string>().c_str());
            fontSize = draw.getDefaultFontSize(font);
        }
        fontSize = config.value("fontSize", fontSize);
        cursorHeight = config.value("cursorHeight", cursorHeight);

        valueEncoderId = config.value("valueEncoderId", valueEncoderId);
        cursorEncoderId = config.value("cursorEncoderId", cursorEncoderId);
    }

    void render() override
    {
        draw.filledRect(relativePosition, size, { bgColor });

        int sep = 4;
        Point topPos = { relativePosition.x, relativePosition.y };
        Size topSize = { size.w, size.h };

        draw.filledRect(topPos, topSize, { foregroundColor });
        int textY = topPos.y + (size.h - fontSize) * 0.5;

        // split text in 3 parts
        std::string left, mid, right;

        if (cursorPos > 0)
            left = value.substr(0, cursorPos);
        if (cursorPos < (int)value.size())
            mid = value.substr(cursorPos, 1);
        if (cursorPos + 1 < (int)value.size())
            right = value.substr(cursorPos + 1);

        // Draw each part with correct color
        int x = topPos.x + 4;
        if (!left.empty())
            x = draw.text({ x, textY }, left, fontSize, { textEditColor, .font = font });

        int cursorY = topPos.y + size.h - 1;
        if (!mid.empty()) {
            int prevX = x;
            x = draw.text({ x + 2, textY }, mid, fontSize, { textColor, .font = font });
            // draw.line({ prevX, cursorY }, { x, cursorY }, { textEditColor });
            draw.filledRect({ prevX, cursorY - cursorHeight }, { x - prevX, cursorHeight }, { textEditColor });
        } else {
            // draw.line({ x, cursorY }, { x + 8, cursorY }, { textEditColor });
            draw.filledRect({ x, cursorY - cursorHeight }, { 8, cursorHeight }, { textEditColor });
        }

        if (!right.empty())
            draw.text({ x + 2, textY }, right, fontSize, { textEditColor, .font = font });
    }

    void onEncoder(int8_t id, int8_t direction) override
    {
        if (id == cursorEncoderId) {
            cursorPos += direction;
            if (cursorPos < 0)
                cursorPos = 0;
            if (cursorPos > (int)value.size())
                cursorPos = value.size();
            renderNext();
            return;
        }

        if (id == valueEncoderId) {
            if (cursorPos == (int)value.size()) {
                int idx = (direction > 0) ? 0 : (int)charset.size() - 1;
                value.push_back(charset[idx]);
            } else if (!value.empty()) {
                char c = value[cursorPos];
                size_t pos = charset.find(c);
                int newPos = 0;
                if (pos == std::string::npos)
                    newPos = (direction > 0) ? 0 : (int)charset.size() - 1;
                else {
                    int cand = (int)pos + direction;
                    if (cand < 0)
                        cand = (int)charset.size() - 1;
                    if (cand >= (int)charset.size())
                        cand = 0;
                    newPos = cand;
                }
                value[cursorPos] = charset[newPos];
            }
            renderNext();
        }
    }

    virtual void save(std::string inputValue) = 0;
};
