#ifndef _CONFIG_H_
#define _CONFIG_H_

#include "fs.h"
#include "getFullpath.h"
#include "trim.h"

#include <stdio.h> // printf

void parseConfigLine(char *line, const char *filename, void (*callback)(char *key, char *value, const char *filename))
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
        printf("Invalid config line: %s\n", line);
        return;
    }
    callback(key, rtrim(value, '\n'), filename);
}

bool loadConfig(const char *filename, void (*callback)(char *key, char *value, const char *filename))
{
    FILE *file = fopen(filename, "r");
    if (file == NULL)
    {
        printf("Failed to load config file: %s\n", filename);
        return false;
    }

    char line[256];

    while (fgets(line, sizeof(line), file))
    {
        parseConfigLine(line, filename, callback);
    }
    fclose(file);

    return true;
}

#endif