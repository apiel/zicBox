#include "AudioPluginHandler.h"
#include "def.h"

#include <iostream>
#include <fstream>
#include "libs/nlohmann/json.hpp"

// For .so file
extern "C" AudioPluginHandlerInterface* load()
{
    return &AudioPluginHandler::get();
}

int main() {
    // Open and read the config.json file
    std::ifstream file("config.json");
    if (!file) {
        std::cerr << "Error: Unable to open config.json\n";
        return 1;
    }

    // Parse the JSON content
    nlohmann::json config;
    try {
        file >> config;
    } catch (const nlohmann::json::parse_error& e) {
        std::cerr << "JSON parse error: " << e.what() << '\n';
        return 1;
    }

    // Pass the config to AudioPluginHandler and start the loop
    AudioPluginHandler::get().config(config).loop();
    return 0;
}