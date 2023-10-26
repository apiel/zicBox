#ifndef _UI_CONFIG_H_
#define _UI_CONFIG_H_

#include "fs.h"
#include "viewManager.h"
#include "plugins.h"
#include "helpers/getFullpath.h"

#define CONFIG_FILE "./ui/index.ui"

bool loadConfig(const char *filename);

char *trimChar(char *str, char c = '\n')
{
    int len = strlen(str);
    for (int i = 0; i < len; i++)
    {
        if (str[i] == c)
        {
            str[i] = '\0';
            break;
        }
    }
    return str;
}

void assignKeyValue(char *key, char *value, const char *filename)
{
    if (strcmp(key, "INCLUDE") == 0)
    {
        loadConfig(getFullpath(value, filename));
    }
    else if (strcmp(key, "PLUGIN_CONTROLLER") == 0)
    {
        loadPluginController(getFullpath(value, filename));
    }
    else if (strcmp(key, "SET_COLOR") == 0)
    {
        char *name = strtok(value, " ");
        char *color = strtok(NULL, " ");
        ViewManager::get().draw.setColor(name, color);
    }
    else if (lastPluginInstance && lastPluginInstance->config(key, value))
    {
        return;
    }
    else
    {
        ViewManager::get().config(key, value, filename);
    }
}

void parseConfigLine(char *line, const char *filename)
{
    // ignore comments and empty lines
    if (line[0] == '#' || line[0] == '\n')
    {
        return;
    }
    // split by '='
    char *key = strtok(line, "=");
    char *value = strtok(NULL, "=");
    if (key == NULL || value == NULL)
    {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Invalid config line: %s\n", line);
        return;
    }
    assignKeyValue(key, trimChar(value), filename);
}

bool loadConfig(const char *filename = CONFIG_FILE)
{
    FILE *file = fopen(filename, "r");
    if (file == NULL)
    {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Failed to load config file: %s\n", filename);
        return false;
    }

    char line[256];

    while (fgets(line, sizeof(line), file))
    {
        parseConfigLine(line, filename);
    }
    fclose(file);

    return true;
}

#endif