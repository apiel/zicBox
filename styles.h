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
        .white = { 0xff, 0xff, 0xff, 255 }, // #ffffff
        .text = { 0xff, 0xff, 0xff, 255 }, // #ffffff
        .primary = { 0xad, 0xcd, 0xff, 255 }, // #adcdff
        .secondary = { 0xff, 0x8d, 0x99, 255 }, // #ff8d99
        .tertiary = { 0x8d, 0xDD, 0x99, 255 }, // #8dDD99
    },
};

#endif
