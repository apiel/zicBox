#pragma once

#include "zicXYv2/studio.h"

#include <string>
#include <optional> // Added for optional color overrides

struct Button {
    std::string text;
    std::optional<Color> color = std::nullopt;
    std::optional<Color> bgColor = std::nullopt;

    // Implicit conversions to keep all existing string literals functioning as-is
    Button(const char* t)
        : text(t)
    {
    }
    Button(const std::string& t)
        : text(t)
    {
    }

    // Support string with an explicit text color override
    Button(std::string t, Color c)
        : text(std::move(t))
        , color(c)
    {
    }

    // Support string with explicit text and background color overrides
    Button(std::string t, Color c, Color bg)
        : text(std::move(t))
        , color(c)
        , bgColor(bg)
    {
    }
};