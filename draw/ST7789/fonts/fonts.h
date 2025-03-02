#pragma once

#include "ArialBold.h"
#include "ArialNormal.h"
#include "BigFont.h"
#include "MusicNote.h"
#include "Sinclair_M.h"
#include "Sinclair_S.h"
#include "Ubuntu.h"
#include "UbuntuBold.h"
#include "RobotoThin_8.h"
#include "RobotoThin_16.h"

#include "TtfFont.h"

#include <string.h>
#include <set>

std::set<TtfFont*> ttfFonts;

void* getFontPtr(std::string& name) 
{
    // if (name == nullptr || strcmp(name, "default") == 0) {
    if (name.empty() || name == "default") {
        return &Sinclair_S;
    }

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

    if (name == "ArialBold") {
        return &ArialBold;
    } else if (name == "ArialNormal") {
        return &ArialNormal;
    } else if (name == "BigFont") {
        return &BigFont;
    } else if (name == "Sinclair_M") {
        return &Sinclair_M;
    } else if (name == "Sinclair_S") {
        return &Sinclair_S;
    } else if (name == "MusicNote") {
        return &FontMusicNote;
    } else if (name == "Ubuntu") {
        return &Ubuntu;
    } else if (name == "UbuntuBold") {
        return &UbuntuBold;
    } else if (name == "RobotoThin_8") {
        return &RobotoThin_8;
    } else if (name == "RobotoThin_16") {
        return &RobotoThin_16;
    }

    return nullptr;
}
