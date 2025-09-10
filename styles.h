#pragma once

#include "plugins/components/drawInterface.h"
#include "plugins/components/utils/color.h"

Styles styles = {
    .screen = { 800, 480 },
    .margin = 1,
    .font = {
        "/usr/share/fonts/truetype/roboto/unhinted/RobotoTTF/Roboto-Regular.ttf",
        "/usr/share/fonts/truetype/roboto/unhinted/RobotoTTF/Roboto-Bold.ttf",
        "/usr/share/fonts/truetype/roboto/unhinted/RobotoTTF/Roboto-Italic.ttf" },
    .colors = {
        .background =rgb(52, 50, 51),
        .white = rgb(255, 255, 255),
        .text = rgb(255, 255, 255),
        .primary = rgb(173, 205, 255),
        .secondary = rgb(255, 141, 153),
        .tertiary = rgb(141, 221, 153),
        .quaternary = rgb(255, 153, 0),
    },
};
