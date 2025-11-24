/** Description:
This specialized segment of code defines a user interface element known as the **Text Component**. Its primary function is to draw and manage text displayed on a screen.

### Structure and Purpose

The Text Component inherits capabilities from a more general base component, allowing it to be integrated into larger graphical layouts. It is specifically designed to handle all aspects related to displaying words.

### Core Functionality

1.  **Text Storage and Appearance:** It maintains the actual string of characters to be shown, along with crucial visual settings: the font size (defaulting to 12), the color of the text (initially set to white), and how the text is positioned horizontally (justification).

2.  **Drawing (Rendering):** When the component is instructed to appear on the screen, it examines its current horizontal alignment (left, center, or right). It then uses the appropriate drawing function to place the text precisely, ensuring the stored font size and color are applied during the rendering process.

### Customization

The component can be easily configured using key-value instructions:

*   **TEXT:** Sets the actual content (the words) displayed by the component.
*   **COLOR:** Changes the text color, often using a standard web format like a hex code.
*   **FONT\_SIZE:** Adjusts the numerical size of the letters.
*   **ALIGN:** Controls the horizontal justification, allowing the text to be aligned to the left, center, or right side of its designated area.

sha: 474c403dc775be9d55351820a0dfad3f3fbbca70e1f483fcbdc02b91f77bb3ea 
*/
#ifndef _UI_COMPONENT_TEXT_H_
#define _UI_COMPONENT_TEXT_H_

#include "../component.h"
#include <string>

/*md
## SDL TextComponent

Draw a text on the view.
*/
class TextComponent : public Component {
protected:
    std::string text;

    uint8_t size = 12;

    struct Colors {
        Color text;
    } colors;

    enum Align {
        LEFT,
        CENTER,
        RIGHT
    } align
        = CENTER;

    // const char* fontPath;

public:
    TextComponent(ComponentInterface::Props props)
        : Component(props)
        , colors({ styles.colors.white })
    // , fontPath(styles.font.regular)
    {
    }

    void render()
    {
        // if (align == LEFT) {
        //     draw.text(position, text, size, { colors.text, fontPath });
        // } else if (align == CENTER) {
        //     draw.textCentered(position, text, size, { colors.text, fontPath });
        // } else {
        //     draw.textRight(position, text, size, { colors.text, fontPath });
        // }

        if (align == LEFT) {
            draw.text(position, text, size, { colors.text });
        } else if (align == CENTER) {
            draw.textCentered(position, text, size, { colors.text });
        } else {
            draw.textRight(position, text, size, { colors.text });
        }
    }

    bool config(char* key, char* value)
    {
        /*md - `TEXT: Hello World` set the text. */
        if (strcmp(key, "TEXT") == 0) {
            text = value;
            return true;
        }

        /*md - `COLOR: #FFFFFF` set the text color. Default is white. */
        if (strcmp(key, "COLOR") == 0) {
            colors.text = draw.getColor(value);
            return true;
        }

        /*md - `FONT_SIZE: 10` set the font size. Default is 12. */
        if (strcmp(key, "FONT_SIZE") == 0) {
            size = atoi(value);
            return true;
        }

        /*md - `ALIGN: left` set the text align. Default is center. */
        if (strcmp(key, "ALIGN") == 0) {
            if (strcmp(value, "left") == 0) {
                align = LEFT;
            } else if (strcmp(value, "center") == 0) {
                align = CENTER;
            } else if (strcmp(value, "right") == 0) {
                align = RIGHT;
            }
            return true;
        }

        // /*md - `BOLD: true` set bold text. */
        // if (strcmp(key, "BOLD") == 0) {
        //     if (strcmp(value, "true") == 0) {
        //         fontPath = styles.font.bold;
        //     }
        //     return true;
        // }

        return false;
    }
};

#endif
