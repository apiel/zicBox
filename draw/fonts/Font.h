/** Description:
This technical document establishes the necessary definitions for managing fonts (text styles) within a computer program. It acts as a blueprint, ensuring consistency in how various font files are stored and processed.

The core definition is a categorization system called `FontType`. This system specifies the two expected formats for font files: `COMPILED_FONT`, which refers to font data that has been optimized or pre-processed specifically for the application, and `TTF_FONT`, which denotes a standard, widely used format like TrueType. This distinction is crucial because the program requires different instructions to correctly interpret the data based on its format.

The second major component is the `Font` structure. This structure serves as a standardized container for a single font object. It holds two pieces of critical information:

1.  The raw data: This holds the actual graphical information—the stored shapes and sizes for all the letters and symbols.
2.  The type: This label indicates which `FontType` the stored data uses. By default, if the type is not specified when the font is loaded, the program assumes it is the optimized `COMPILED_FONT`.

In summary, this header file provides the fundamental organizational framework necessary for the software to load, identify, and render text correctly, regardless of the font's source format.

sha: 72668dab26f3d79858c00f1242f709c1036fc97610aac7bbc6dab6e59225635e 
*/
#pragma once

#include <cstdint>

enum FontType {
    COMPILED_FONT = 0,
    TTF_FONT
};

struct Font {
    void *data;
    FontType type = COMPILED_FONT;
};