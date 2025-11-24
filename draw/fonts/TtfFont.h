/** Description:
This code defines a specialized blueprint, or class, called `TtfFont`. Its purpose is to handle and manage TrueType or OpenType font files, which are commonly used for displaying text on a screen.

The `TtfFont` is designed to be a concrete implementation of a more general `Font` concept, allowing applications to treat different font types consistently.

**How it Works:**

1.  **Foundation:** The class relies on an external, professional standard library called FreeType. This library is responsible for the complex task of reading and interpreting the structural data within a font file (like glyph shapes and metrics).
2.  **Loading:** When a `TtfFont` object is created, it immediately attempts to initialize the FreeType system and load a specific font file (identified by its file path). If the file is not found or is corrupted, the system records an error. The successfully loaded font data is stored for later use.
3.  **Text Measurement:** The primary functionality provided is the ability to measure text. The class contains functions that calculate the exact pixel width required to display either a single character or an entire string. It accomplishes this by consulting the loaded font data via FreeType, determining the bitmap dimensions of each letter, and totaling them up, optionally including user-defined spacing between characters.

In summary, this class acts as a dedicated interface between a graphics application and the intricate details contained within standard font files, ensuring accurate text layout and rendering.

sha: d005e86311f968d170d155081b19510ccaf77c994bd3bd49948296f0681fe67b 
*/
#pragma once

#include <ft2build.h>
#include <string>
#include FT_FREETYPE_H

#include "log.h"
#include "Font.h"

class TtfFont : public Font {
public:
    FT_Face face;
    std::string fontPath;

    TtfFont(const std::string& fontPath)
        : fontPath(fontPath)
    {
        type = TTF_FONT;

        FT_Library ft;
        if (FT_Init_FreeType(&ft)) {
            logError("Could not initialize FreeType library");
            return;
        }

        if (FT_New_Face(ft, fontPath.c_str(), 0, &face)) {
            logError("Could not load font file: " + fontPath);
            return;
        }
    }

    int getWidth(std::string& text, int spacing = 0) {
        int width = 0;
        for (char& c : text) {
            if (FT_Load_Char(face, c, FT_LOAD_RENDER)) {
                logWarn("Could not load character: " + std::string(1, c));
                return 0;
            }
            width += face->glyph->bitmap.width + spacing;
        }
        return width;
    }

    int getWidth(char c) {
        if (FT_Load_Char(face, c, FT_LOAD_RENDER)) {
            logWarn("Could not load character: " + std::string(1, c));
            return 0;
        }
        return face->glyph->bitmap.width;
    }
};
