#pragma once

// #include "MusicNote.h"
#include "DejaVuSans_12.h"
#include "DejaVuSans_16.h"
#include "DejaVuSans_24.h"
#include "DejaVuSans_8.h"
#include "PoppinsLight_12.h"
#include "PoppinsLight_16.h"
#include "PoppinsLight_24.h"
#include "PoppinsLight_8.h"
#include "RobotoThin_12.h"
#include "RobotoThin_16.h"
#include "RobotoThin_24.h"
#include "RobotoThin_8.h"

#ifdef FT_FREETYPE_H
#include "TtfFont.h"
#endif

#include <set>
#include <string.h>

#define DEFAULT_FONT_SIZE 12
#define DEFAULT_FONT PoppinsLight_12

#ifdef FT_FREETYPE_H
std::set<TtfFont*> ttfFonts;
#endif

void* getFontPtr(std::string& name)
{
    // if (name == nullptr || strcmp(name, "default") == 0) {
    if (name.empty() || name == "default") {
        return &DEFAULT_FONT;
    }

#ifdef FT_FREETYPE_H
    // If name end with .ttf, it's a ttf font
    if (name.find(".ttf") != std::string::npos) {
        // First search to see if it's ttf font already loaded
        for (TtfFont* font : ttfFonts) {
            if (font->fontPath == name) {
                return font;
            }
        }
        // Else create a new ttf font
        TtfFont* font = new TtfFont(name);
        ttfFonts.insert(font);
        return font;
    }
#endif

    if (name == "RobotoThin_8") {
        return &RobotoThin_8;
    } else if (name == "RobotoThin_12") {
        return &RobotoThin_12;
    } else if (name == "RobotoThin_16") {
        return &RobotoThin_16;
    } else if (name == "RobotoThin_24") {
        return &RobotoThin_24;
    } else if (name == "PoppinsLight_8") {
        return &PoppinsLight_8;
    } else if (name == "PoppinsLight_12") {
        return &PoppinsLight_12;
    } else if (name == "PoppinsLight_16") {
        return &PoppinsLight_16;
    } else if (name == "PoppinsLight_24") {
        return &PoppinsLight_24;
    } else if (name == "DejaVuSans_8") {
        return &DejaVuSans_8;
    } else if (name == "DejaVuSans_12") {
        return &DejaVuSans_12;
    } else if (name == "DejaVuSans_16") {
        return &DejaVuSans_16;
    } else if (name == "DejaVuSans_24") {
        return &DejaVuSans_24;
    }

    return nullptr;
}
