/** Description:
This header file contains the core initialization routine for the entire application, serving as the central hub for managing all system settings.

### Purpose and Functionality

The file defines a function responsible for reading configuration parameters from a specialized JSON settings file. This is the first critical step upon application startup, ensuring all components are configured correctly before execution begins.

### Basic Idea of Operation

1.  **File Loading:** The system is directed to a specific path where the configuration file resides. It securely attempts to open and read this file, logging the progress.
2.  **Data Interpretation:** The contents of the file, which are written in the structured JSON format, are translated into instructions the program can understand.
3.  **Distribution of Settings:** The settings are strategically distributed to specific functional systems within the application:
    *   It initializes the **audio host**, ensuring sound processing capabilities are ready.
    *   It configures the various **controllers** that manage the application’s logic, behavior, and input handling.
    *   It sets up the **View Manager**, which is the primary system governing the layout, drawing, and interaction of the entire user interface.

Robust error handling is included (using a "try-catch" mechanism), meaning if the configuration file is missing or corrupted, the system logs the exact issue rather than causing the program to crash. This ensures a more reliable startup process.

sha: b29bdb4c9d4f9eaf2e10a9a0e7e0221422afca1fabc070c894388693086e3d7d 
*/
#pragma once

#include "controllers.h"
#include "host.h"
#include "log.h"
#include "plugins/components/drawInterface.h"
#include "styles.h"
#include "viewManager.h"

#include "plugins/components/utils/color.h"

#include <fstream>

void loadJsonConfig(std::string configPath)
{
    try {
        std::ifstream configFile(configPath);
        logInfo("load json config: %s", configPath.c_str());
        if (configFile.is_open()) {
            nlohmann::json config = nlohmann::json::parse(configFile);
            configFile.close();
            if (config.contains("audio")) {
                logInfo("----------- init audio -------------");
                hostConfig(config["audio"]);
            }
            logInfo("----------- init controllers -------------");
            lastPluginControllerInstance->config(config); // <--- not very nice!!!!
            if (config.contains("controllers")) {

                // TODO to be implemented...
            }

            ViewManager::get().config(config);
        }
        logInfo("----------- config done -------------");
    } catch (const std::exception& e) {
        logError("load json config: %s", e.what());
    }
}