#ifndef _SCRIPT_H_
#define _SCRIPT_H_

#include "fs.h"
#include "getFullpath.h"
#include "mathParser.h"
#include "trimChar.h"

#include <stdio.h> // printf
#include <vector>

uint16_t countLeadingSpaces(char* str)
{
    uint16_t count = 0;
    while (str[0] == ' ') {
        count++;
        str++;
    }
    return count;
}

char* removeLeadingSpaces(char* str)
{
    uint16_t count = countLeadingSpaces(str);
    return str + count;
}

namespace Script {

struct Variable {
    // Could assign char size dynamically but for the moment let's keep it simple
    char key[64];
    char value[64];
};
std::vector<Variable> variables;

void setVariable(char* line)
{
    char* key = strtok(line, "=");
    char* value = strtok(NULL, "=");
    if (key == NULL || value == NULL) {
        printf("Invalid variable line: %s\n", line);
        return;
    }
    // search first if variable already exists
    for (auto variable : variables) {
        if (strcmp(variable.key, key) == 0) {
            strcpy(variable.value, value);
            return;
        }
    }
    // else add new variable
    Variable variable;
    strcpy(variable.key, key);
    strcpy(variable.value, value);
    variables.push_back(variable);
}

char* applyVariable(char* str)
{
    std::string target(str);

    for (auto variable : variables) {
        int pos;
        std::string k(variable.key);
        while ((pos = target.find(k)) != std::string::npos) {
            target.replace(pos, k.length(), variable.value);
        }
    }
    strcpy(str, target.c_str());
    return str;
}

char* prepareParam(char* param)
{
    param = removeLeadingSpaces(param);
    return param;
}

std::vector<char*> getParams(char* paramsStr)
{
    std::vector<char*> params;
    if (paramsStr != NULL) {
        applyVariable(paramsStr);
        char* param = strtok(paramsStr, ",");
        while (param != NULL) {
            params.push_back(prepareParam(param));
            param = strtok(NULL, ",");
        }
    }
    return params;
}

char* prepareKey(char* key)
{
    return key;
}

void parseScriptLine(char* line, const char* filename, void (*callback)(char* key, std::vector<char*> params, const char* filename))
{
    // ignore comments and empty lines
    if (line[0] == '#' || line[0] == '\n') {
        return;
    }

    line = trimChar(line);
    uint16_t indentation = countLeadingSpaces(line);
    line = line + indentation; // remove leading spaces

    if (line[0] == '$') {
        setVariable(line);
        return;
    }

    char* key = strtok(line, ":");
    if (key == NULL) {
        return;
    }

    char* paramsStr = strtok(NULL, ":");
    callback(key, getParams(paramsStr), filename);
}

}

bool loadScript(const char* filename, void (*callback)(char* key, std::vector<char*> params, const char* filename))
{
    FILE* file = fopen(filename, "r");
    if (file == NULL) {
        printf("Failed to load script: %s\n", filename);
        return false;
    }

    char line[512];

    while (fgets(line, sizeof(line), file)) {
        Script::parseScriptLine(line, filename, callback);
    }
    fclose(file);

    MathParser mathParser;
    double res = mathParser.eval_exp((char*)"1 + 2 * (3 + 1) + 1 000");
    printf("--------- Result: %f\n", res);

    return true;
}

#endif