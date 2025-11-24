/** Description:
This file serves as a central hub for managing all the text styles and sizes (fonts) available to the application. It acts like a resource list and a finding mechanism, ensuring the program can easily display text in the correct visual format.

The header pre-loads definitions for many common, fixed font styles, such as different sizes of "DejaVu Sans," "Poppins Light," and "Roboto Thin." This ensures those specific fonts are always ready for immediate use. It also sets a standard default font (Poppins Light size 12) for when no specific style is requested.

The core of this system is a dedicated function that finds the requested font. When the program needs a font by name, this function checks a specific list of possibilities:

1.  **Default Check:** If the program requests the standard font, it returns the built-in default.
2.  **External Files (Dynamic Loading):** If the system has the capability to read font files from outside the program (like TrueType files), the function checks if the requested file has already been loaded. If it's a new file, the system loads the external font and keeps it saved for future requests.
3.  **Internal List (Static Check):** If the request matches one of the many pre-compiled, fixed font styles (like "RobotoThin\_16"), the system immediately provides access to that data.

This design allows the application to utilize both permanently embedded fonts and flexible fonts loaded directly from files, all through one standardized request process.

sha: 12cef6cc306519d238f187d4351caa4fa041cad063d5b8a0f6c772b2e165f88f 
*/
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
#include "PoppinsLight_6.h"
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
    } else if (name == "PoppinsLight_6") {
        return &PoppinsLight_6;
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
