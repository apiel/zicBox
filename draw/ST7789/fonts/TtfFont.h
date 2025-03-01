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
};
