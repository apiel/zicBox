#pragma once

#include <sstream>
#include <map>

std::map<std::string, std::string> parseFormUrlEncoded(const std::string &body) {
    std::map<std::string, std::string> result;
    std::istringstream ss(body);
    std::string pair;

    while (std::getline(ss, pair, '&')) {
        auto pos = pair.find('=');
        if (pos != std::string::npos) {
            std::string key = pair.substr(0, pos);
            std::string val = pair.substr(pos + 1);
            result[key] = val;
        }
    }
    return result;
}