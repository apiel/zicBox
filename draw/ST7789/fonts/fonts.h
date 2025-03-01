#pragma once

#include "ArialBold.h"
#include "ArialNormal.h"
#include "BigFont.h"
#include "MusicNote.h"
#include "Sinclair_M.h"
#include "Sinclair_S.h"
#include "Ubuntu.h"
#include "UbuntuBold.h"
#include "5x6.h"

#include <string.h>

void* getFontPtr(std::string& name) 
{
    // if (name == nullptr || strcmp(name, "default") == 0) {
    if (name.empty() || name == "default") {
        return &Sinclair_S;
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
    } else if (name == "5x6") {
        printf("-----------------------> try to use font 5x6\n");
        return &Font5x6;
    }

    return nullptr;
}
