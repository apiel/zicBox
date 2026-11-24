#pragma once

#include "Fm4Sequencer.h"
#include "core/Fm4Panel.h"
#include <algorithm>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <string>

namespace Persist {

inline std::string path()
{
    std::string dir;
    if (const char* xdg = std::getenv("XDG_CONFIG_HOME")) dir = xdg;
    else if (const char* home = std::getenv("HOME")) dir = std::string(home) + "/.config";
    else dir = ".";
    dir += "/zicStudioFm4";
    std::error_code ec;
    std::filesystem::create_directories(dir, ec);
    return dir + "/state.txt";
}

inline std::string serialize(Fm4Panel::Panel& panel, const Fm4Sequencer& seq)
{
    std::ostringstream o;
    o << "bpm " << seq.bpm << "\n";
    o << "transpose " << seq.transpose << "\n";
    o << "patternLength " << seq.patternLength << "\n";

    for (int i = 0; i < Fm4Panel::POTS; ++i) {
        o << "param " << panel.param(i).key << " " << panel.param(i).value << "\n";
    }

    o << "steps " << Fm4Sequencer::STEPS << "\n";
    for (int i = 0; i < Fm4Sequencer::STEPS; ++i) {
        const auto& s = seq.steps[i];
        o << "step " << i << " "
          << (s.on ? 1 : 0) << " "
          << (int)s.note << " "
          << s.length << " "
          << s.velocity << " "
          << (s.accent ? 1 : 0) << " "
          << (s.slide ? 1 : 0) << "\n";
    }
    return o.str();
}

inline bool save(const std::string& file, const std::string& content)
{
    std::string tmp = file + ".tmp";
    {
        std::ofstream f(tmp);
        if (!f) return false;
        f << content;
    }
    std::error_code ec;
    std::filesystem::rename(tmp, file, ec);
    return !ec;
}

inline bool load(const std::string& file, Fm4Panel::Panel& panel, Fm4Sequencer& seq)
{
    std::ifstream f(file);
    if (!f) return false;
    std::string line;
    while (std::getline(f, line)) {
        std::istringstream l(line);
        std::string key;
        l >> key;
        if (key == "bpm") {
            l >> seq.bpm;
        } else if (key == "transpose") {
            l >> seq.transpose;
        } else if (key == "patternLength") {
            l >> seq.patternLength;
            seq.patternLength = std::clamp(seq.patternLength, 1, Fm4Sequencer::STEPS);
        } else if (key == "param") {
            std::string name;
            float val;
            if (l >> name >> val) {
                for (int i = 0; i < Fm4Panel::POTS; ++i) {
                    if (name == panel.param(i).key) {
                        panel.param(i).set(val);
                        panel.defaults[i] = panel.norm(i);
                        break;
                    }
                }
            }
        } else if (key == "step") {
            int idx, on, note, acc, sld;
            float len, vel;
            if (l >> idx >> on >> note >> len >> vel >> acc >> sld) {
                if (idx >= 0 && idx < Fm4Sequencer::STEPS) {
                    seq.steps[idx].on = (on != 0);
                    seq.steps[idx].note = (uint8_t)std::clamp(note, 0, 127);
                    seq.steps[idx].length = std::clamp(len, 0.1f, 8.0f);
                    seq.steps[idx].velocity = std::clamp(vel, 0.0f, 1.0f);
                    seq.steps[idx].accent = (acc != 0);
                    seq.steps[idx].slide = (sld != 0);
                }
            }
        }
    }
    return true;
}

} // namespace Persist
