#pragma once

#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <string>

namespace Persist {

inline std::string path() {
    std::string dir;
    if (const char* xdg = std::getenv("XDG_CONFIG_HOME")) dir = xdg;
    else if (const char* home = std::getenv("HOME")) dir = std::string(home) + "/.config";
    else dir = ".";
    dir += "/zicStudio";
    std::error_code ec;
    std::filesystem::create_directories(dir, ec);
    return dir + "/state.txt";
}

inline bool save(const std::string& file, const std::string& content) {
    std::string tmp = file + ".tmp";
    { std::ofstream f(tmp); if (!f) return false; f << content; }
    std::error_code ec;
    std::filesystem::rename(tmp, file, ec);
    return !ec;
}

} // namespace Persist