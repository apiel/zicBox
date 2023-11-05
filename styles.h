#ifndef _STYLES_H_
#define _STYLES_H_

#include "plugins/components/drawInterface.h"

Styles styles = {
    .screen = { 800, 480 },
    .margin = 1,
    .font = {
        "/usr/share/fonts/truetype/roboto/unhinted/RobotoTTF/Roboto-Regular.ttf",
        "/usr/share/fonts/truetype/roboto/unhinted/RobotoTTF/Roboto-Bold.ttf",
        "/usr/share/fonts/truetype/roboto/unhinted/RobotoTTF/Roboto-Italic.ttf" },
    .colors = {
        .background = { 0x21, 0x25, 0x2b, 255 }, // #21252b
        .overlay = { 0xd1, 0xe3, 0xff, 0x1e }, // #d1e3ff1e
        .on = { 0x00, 0xb3, 0x00, 255 }, // #00b300
        .white = { 0xff, 0xff, 0xff, 255 }, // #ffffff
        .blue = { 0xad, 0xcd, 0xff, 255 }, // #adcdff
        .red = { 0xff, 0x8d, 0x99, 255 }, // #ff8d99
        .turquoise = { 0x00, 0xFF, 0xEF, 255 }, // #00ffef
        .grey = { 0x80, 0x80, 0x80, 255 }, // #808080
    },
};

#endif
