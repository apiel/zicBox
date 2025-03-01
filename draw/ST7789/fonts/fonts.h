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

void* getFontPtr(const char* name = nullptr) 
{
    printf("load font %s\n", name);
    if (name == nullptr || strcmp(name, "default") == 0) {
        return &Sinclair_S;
    }

    if (strcmp(name, "ArialBold") == 0) {
        return &ArialBold;
    } else if (strcmp(name, "ArialNormal") == 0) {
        return &ArialNormal;
    } else if (strcmp(name, "BigFont") == 0) {
        return &BigFont;
    } else if (strcmp(name, "Sinclair_M") == 0) {
        return &Sinclair_M;
    } else if (strcmp(name, "Sinclair_S") == 0) {
        return &Sinclair_S;
    } else if (strcmp(name, "MusicNote") == 0) {
        return &FontMusicNote;
    } else if (strcmp(name, "Ubuntu") == 0) {
        return &Ubuntu;
    } else if (strcmp(name, "UbuntuBold") == 0) {
        return &UbuntuBold;
    } else if (strcmp(name, "5x6") == 0) {
        printf("-----------------------> try to use font 5x6\n");
        return &Font5x6;
    }

    return nullptr;
}
