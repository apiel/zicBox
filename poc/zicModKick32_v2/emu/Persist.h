#pragma once

/**
 * Emulator only: remembers the pots, the pattern, the BPM and the tune between runs.
 *
 * - stored in ~/.config/zicModKick32_v2/state.txt (does not depend on where you start the emulator)
 * - every pot is stored by its key ("fmDepth 20"), never by position: adding, removing or
 *   reordering params never loses the other settings, unknown keys are ignored
 * - autosaved, so a crash or a killed terminal does not lose anything
 */

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <string>

#include "Sequencer.h"
#include "core/Kick32v2Panel.h"

namespace Persist {

inline std::string path()
{
    std::string dir;
    if (const char* xdg = std::getenv("XDG_CONFIG_HOME")) dir = xdg;
    else if (const char* home = std::getenv("HOME")) dir = std::string(home) + "/.config";
    else dir = ".";
    dir += "/zicModKick32_v2";
    std::error_code ec;
    std::filesystem::create_directories(dir, ec);
    return dir + "/state.txt";
}

inline std::string serialize(Kick32v2Panel::Panel& panel, const Sequencer& seq)
{
    std::ostringstream o;
    o << "bpm " << seq.bpm << "\ntranspose " << seq.transpose << "\n";
    for (int i = 0; i < Kick32v2Panel::POTS; ++i) o << "param " << panel.param(i).key << " " << panel.param(i).value << "\n";
    o << "steps";
    for (auto& s : seq.steps) o << " " << (s.on ? (int)s.note : -100);
    o << "\n";
    return o.str();
}

inline bool save(const std::string& file, const std::string& content)
{
    std::string tmp = file + ".tmp"; // write + rename: a crash never leaves a half written file
    {
        std::ofstream f(tmp);
        if (!f) return false;
        f << content;
    }
    std::error_code ec;
    std::filesystem::rename(tmp, file, ec);
    return !ec;
}

inline bool load(const std::string& file, Kick32v2Panel::Panel& panel, Sequencer& seq)
{
    std::ifstream f(file);
    if (!f) return false;
    std::string line;
    while (std::getline(f, line)) {
        std::istringstream l(line);
        std::string key;
        l >> key;
        if (key == "bpm") l >> seq.bpm;
        else if (key == "transpose") l >> seq.transpose;
        else if (key == "param") {
            std::string name;
            float value;
            if (l >> name >> value)
                for (int i = 0; i < Kick32v2Panel::POTS; ++i)
                    if (name == panel.param(i).key) panel.param(i).set(value);
        } else if (key == "steps") {
            for (auto& s : seq.steps) {
                int v = -100;
                l >> v;
                s.on = v != -100;
                s.note = s.on ? (int8_t)v : 0;
            }
        }
    }
    return true;
}

} // namespace Persist
