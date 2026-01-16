/** Description:
This code snippet functions as the central visual style configuration file, or "style guide," for a piece of software or graphical component. It does not contain executable instructions; instead, it defines the essential aesthetic rules the program must follow.

**Basic Idea:**
This header defines a comprehensive set of default appearance settings, grouped under the name "styles." Any part of the application that needs to display something visually will reference these definitions to ensure consistency.

**The configuration includes three main areas:**

1.  **Layout and Dimensions:** It establishes the intended viewing area size (800 by 480 units) and defines standard spacing used around elements (the margin).
2.  **Font Resources:** It specifies the exact location (file paths) of the required typeface files, ensuring the software knows where to find the regular, bold, and italic versions of the approved font.
3.  **Color Palette:** A detailed color scheme is defined. Each color is given a functional name (like background, text, white, and various levels of primary, secondary, and tertiary accents). These colors are defined using standardized Red, Green, and Blue numerical values, ensuring precise color reproduction throughout the entire user interface.

By centralizing all these visual properties, the software ensures a unified look and feel, and makes it simple for developers to change the entire application's theme by modifying only this one file.

sha: 1a7d252a7ed46a97b0a06c141c811392f1abd4f13a6307d0ffbedbd1dfea2ab6 
*/
#pragma once

#include "draw/drawInterface.h"
#include "plugins/components/utils/color.h"

Styles styles = {
    .screen = { 800, 480 },
    .margin = 1,
    .font = {
        "/usr/share/fonts/truetype/roboto/unhinted/RobotoTTF/Roboto-Regular.ttf",
        "/usr/share/fonts/truetype/roboto/unhinted/RobotoTTF/Roboto-Bold.ttf",
        "/usr/share/fonts/truetype/roboto/unhinted/RobotoTTF/Roboto-Italic.ttf" },
    .colors = {
        .background =rgb(52, 50, 51),
        .white = rgb(255, 255, 255),
        .text = rgb(255, 255, 255),
        .primary = rgb(173, 205, 255),
        .secondary = rgb(255, 141, 153),
        .tertiary = rgb(141, 221, 153),
        .quaternary = rgb(255, 153, 0),
    },
};
