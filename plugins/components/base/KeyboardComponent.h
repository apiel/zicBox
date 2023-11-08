#ifndef _UI_COMPONENT_KEYBOARD_H_
#define _UI_COMPONENT_KEYBOARD_H_

#include "ButtonBaseComponent.h"
#include "../component.h"

class KeyboardComponent : public Component {
protected:
    static const uint8_t keyCount = 32;
    const uint8_t columnCount = 8;

    ButtonBaseComponent* buttons[keyCount];

    const char* ABC[keyCount] = {
        // clang-format off
        "A", "B", "C", "D", "E", "F", "G", "H",
        "I", "J", "K", "L", "M", "N", "O", "P",
        "Q", "R", "S", "T", "U", "V", "W", "X",
        "Y", "Z", ".", "_", "-", "123!.?", "abc", "Backspace",
        // clang-format on
    };

    const char* abc[keyCount] = {
        // clang-format off
        "a", "b", "c", "d", "e", "f", "g", "h",
        "i", "j", "k", "l", "m", "n", "o", "p",
        "q", "r", "s", "t", "u", "v", "w", "x",
        "y", "z", ".", "_", "-", "123!.?", "ABC", "Backspace",
        // clang-format on
    };

    const char* _123[keyCount] = {
        // clang-format off
        "1", "2", "3", "4", "5", "6", "7", "8",
        "9", "0", "!", "?", "#", "$", "%", "^",
        "&", "*", "(", ")", "-", "+", "=", "@",
        "[", "]", ":", ";", "~", "ABC", "abc", "Backspace",
        // clang-format on
    };

    uint8_t keyboardLayout = 0;

    const char** getKeyboard()
    {
        switch (keyboardLayout) {
        case 0:
            return ABC;
        case 1:
            return abc;
        case 2:
            return _123;
        }
        return ABC;
    }

    void setButtonsLabel()
    {
        for (int i = 0; i < 32; i++) {
            uint8_t row = i / columnCount;
            uint8_t column = i % columnCount;
            buttons[i]->label = getKeyboard()[i];
        }
    }

public:
    KeyboardComponent(ComponentInterface::Props& props)
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
            buttons[i] = new ButtonBaseComponent(btnProps);
        }
        setButtonsLabel();
    }

    ~KeyboardComponent()
    {
        for (int i = 0; i < 32; i++) {
            delete buttons[i];
        }
    }

    void render()
    {
        for (int i = 0; i < 32; i++) {
            buttons[i]->render();
        }
    }

    void onMotion(MotionInterface& motion)
    {
        for (int i = 0; i < 32; i++) {
            buttons[i]->handleMotion(motion);
            if (buttons[i]->needRendering) {
                renderNext();
            }
        }
    }

    void onMotionRelease(MotionInterface& motion)
    {
        for (int i = 0; i < 32; i++) {
            buttons[i]->handleMotionRelease(motion);
            if (buttons[i]->needRendering) {
                renderNext();
            }
        }
    }
};

#endif
