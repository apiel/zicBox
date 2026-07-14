#pragma once

#include "zicXYv2/studio.h"

#include <string>
#include <optional> // Added for optional color overrides

struct Button {
    std::string text;
    std::optional<Color> bgColor = std::nullopt;
    std::optional<Color> fgColor = std::nullopt;

    // Implicit conversions to keep all existing string literals functioning as-is
    Button(const char* t)
        : text(t)
    {
    }
    Button(const std::string& t)
        : text(t)
    {
    }

    // Support string with an explicit foreground color override
    Button(std::string t, Color fg)
        : text(std::move(t))
        , fgColor(fg)
    {
    }

    // Support string with explicit background and foreground color overrides
    Button(std::string t, Color bg, Color fg)
        : text(std::move(t))
        , bgColor(bg)
        , fgColor(fg)
    {
    }
};