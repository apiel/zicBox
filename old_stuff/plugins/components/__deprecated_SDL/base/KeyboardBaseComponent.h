/** Description:
This file serves as the foundational blueprint for a dedicated on-screen virtual keyboard component within a user interface (UI) system. It is not the keyboard itself, but the definition of how the keyboard behaves and is structured.

The primary purpose of this component is to manage user text input and organize the available characters. It is constructed from 32 individual buttons and controls three distinct sets of characters, referred to as "layouts," much like the shift key on a standard keyboard:
1. **Uppercase (ABC)**
2. **Lowercase (abc)**
3. **Numbers and Symbols (123)**

When the user interacts with the keyboard (e.g., taps a button), the code handles this event. If a standard character key is pressed, that character is added to the currently stored input text string.

If a special control key is tapped, such as one labeled "ABC" or "123," the component automatically switches the display, changing all 32 buttons to reflect the characters of the new layout. It also includes specific logic for functions like the backspace key, which instantly removes the last character entered.

Overall, this component defines the structure and behavior of a fully interactive keyboard, ensuring that all button presses are handled correctly, the input text is updated in real-time, and the visible keyboard layout changes dynamically based on user selection.

sha: 090550a0e2b1c1ed2b3691d324ef196c3c1c392d20b287118dd5012c5edbf4c8 
*/
#ifndef _UI_BASE_COMPONENT_KEYBOARD_H_
#define _UI_BASE_COMPONENT_KEYBOARD_H_

#include "../../component.h"
#include "../ButtonComponent.h"

#include <string>

class KeyboardBaseComponent : public Component {
protected:
    static const uint8_t keyCount = 32;
    const uint8_t columnCount = 8;

    ButtonComponent* buttons[keyCount];

    const char* ABC[keyCount] = {
        // clang-format off
        "A", "B", "C", "D", "E", "F", "G", "H",
        "I", "J", "K", "L", "M", "N", "O", "P",
        "Q", "R", "S", "T", "U", "V", "W", "X",
        "Y", "Z", ".", "_", "-", "123!.?", "abc", "&icon::backspace",
        // clang-format on
    };

    const char* abc[keyCount] = {
        // clang-format off
        "a", "b", "c", "d", "e", "f", "g", "h",
        "i", "j", "k", "l", "m", "n", "o", "p",
        "q", "r", "s", "t", "u", "v", "w", "x",
        "y", "z", ".", "_", "-", "123!.?", "ABC", "&icon::backspace",
        // clang-format on
    };

    const char* _123[keyCount] = {
        // clang-format off
        "1", "2", "3", "4", "5", "6", "7", "8",
        "9", "0", "!", "?", "#", "$", "%", "^",
        "&", "*", "(", ")", "-", "+", "=", "@",
        "[", "]", ":", ";", "~", "ABC", "abc", "&icon::backspace",
        // clang-format on
    };

    uint8_t layout = 0;

    const char** getLayout()
    {
        switch (layout) {
        case 0:
            return ABC;
        case 1:
            return abc;
        case 2:
            return _123;
        }
        return ABC;
    }

    void setLayout(uint8_t val)
    {
        layout = val;
        setButtonsLabel();
        renderNext();
    }

    void setButtonsLabel()
    {
        for (int i = 0; i < 32; i++) {
            uint8_t row = i / columnCount;
            uint8_t column = i % columnCount;
            const char* label = getLayout()[i];
            buttons[i]->setLabel(label);
            if (strcmp(label, "ABC") == 0) {
                buttons[i]->onRelease = [this]() { setLayout(0); };
            } else if (strcmp(label, "abc") == 0) {
                buttons[i]->onRelease = [this]() { setLayout(1); };
            } else if (strcmp(label, "123!.?") == 0) {
                buttons[i]->onRelease = [this]() { setLayout(2); };
            } else if (strcmp(label, "&icon::backspace") == 0) {
                buttons[i]->onRelease = [this]() {
                    if (value && value->length() > 0) {
                        // value[strlen(value) - 1] = 0;
                        value->resize(value->length() - 1);
                        onUpdate(value);
                    }
                };
            } else {
                buttons[i]->onRelease = [this, label]() {
                    if (value) {
                        // strcat(value, label);
                        value->append(label);
                        onUpdate(value);
                    }
                };
            }
        }
    }

public:
    std::string * value = NULL;
    std::function<void(std::string * value)> onUpdate = [](std::string * value) {};

    KeyboardBaseComponent(ComponentInterface::Props props)
        : Component(props)
    {
        uint8_t rowCount = keyCount / columnCount;
        Size btnArea = {
            (int)((props.size.w) / (float)columnCount),
            (int)((props.size.h) / (float)(rowCount + 1))
        };

        ComponentInterface::Props btnProps = props;

        const int btnMargin = 4;
        for (int i = 0; i < 32; i++) {
            uint8_t row = i / columnCount;
            uint8_t column = i % columnCount;

            btnProps.position = {
                (int)(position.x + column * btnArea.w + btnMargin),
                (int)(position.y + row * btnArea.h + btnMargin),
            };
            btnProps.size = {
                (int)(btnArea.w - 2 * btnMargin),
                (int)(btnArea.h - 2 * btnMargin)
            };
            buttons[i] = new ButtonComponent(btnProps);
        }
        setButtonsLabel();
    }

    ~KeyboardBaseComponent()
    {
        for (int i = 0; i < 32; i++) {
            delete buttons[i];
        }
    }

    void renderNext()
    {
        // Component::renderNext();
        for (int i = 0; i < 32; i++) {
            buttons[i]->renderNext();
        }
    }

    void onMotion(MotionInterface& motion)
    {
        for (int i = 0; i < 32; i++) {
            buttons[i]->handleMotion(motion);
        }
    }

    void onMotionRelease(MotionInterface& motion)
    {
        for (int i = 0; i < 32; i++) {
            buttons[i]->handleMotionRelease(motion);
        }
    }
};

#endif
