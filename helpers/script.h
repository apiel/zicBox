#ifndef _SCRIPT_H_
#define _SCRIPT_H_

#include "fs.h"
#include "getFullpath.h"
#include "mathParser.h"
#include "trim.h"

#include <stdexcept>
#include <stdio.h> // printf
#include <vector>

namespace Script {

struct Variable {
    string key;
    string value;
};
std::vector<Variable> variables;

string parseValue(char* param)
{
    param = ltrim(param, ' ');
    try {
        double val = MathParser::eval(param);
        return rtrim(std::to_string(val), "0.");
    } catch (const std::exception& e) {
        // do nothing, it's just not a math expression
    }
    return param;
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

void setVariable(char* line)
{
    char* key = strtok(line, "=");
    char* value = strtok(NULL, "=");
    if (key == NULL || value == NULL) {
        printf("Invalid variable line: %s\n", line);
        return;
    }
    value = applyVariable(value);

    // search first if variable already exists
    for (int i = 0; i < variables.size(); i++) {
        if (variables[i].key == key) {
            variables[i].value = parseValue(value);
            return;
        }
    }
    Variable variable;
    variable.key = key;
    variable.value = parseValue(value);
    variables.push_back(variable);
}

std::vector<string> getParams(char* paramsStr)
{
    std::vector<string> params;
    if (paramsStr != NULL) {
        applyVariable(paramsStr);
        char* param = strtok(paramsStr, ",");
        while (param != NULL) {
            params.push_back(parseValue(param));
            param = strtok(NULL, ",");
        }
    }
    return params;
}

enum ResultTypes {
    DEFAULT = 0,
    IF_FALSE,
    LOOP
};

ResultTypes defaultCallback(char* key, std::vector<string> params, const char* filename, void (*callback)(char* key, std::vector<string> params, const char* filename))
{
    if (strcmp(key, "if") == 0) {
        if (params.size() != 3) {
            throw std::runtime_error("Invalid if statement, required 3 paremeters: if: $var1, =, $var3");
        }

        bool result = false;
        if (params[1] == "==") {
            result = params[0] == params[2];
        } else if (params[1] == "!=") {
            result = params[0] != params[2];
        } else if (params[1] == ">") {
            result = stod(params[0]) > stod(params[2]);
        } else if (params[1] == "<") {
            result = stod(params[0]) < stod(params[2]);
        } else if (params[1] == ">=") {
            result = stod(params[0]) >= stod(params[2]);
        } else if (params[1] == "<=") {
            result = stod(params[0]) <= stod(params[2]);
        } else {
            throw std::runtime_error("Invalid if statement operator: " + params[1]);
        }

        printf("------ if %s %s %s = %s\n", params[0].c_str(), params[1].c_str(), params[2].c_str(), result ? "true" : "false");

        return result ? ResultTypes::DEFAULT : ResultTypes::IF_FALSE;
    }
    callback(key, params, filename);

    return ResultTypes::DEFAULT;
}

ResultTypes parseScriptLine(char* line, const char* filename, void (*callback)(char* key, std::vector<string> params, const char* filename))
{
    line = ltrim(line, ' ');

    // ignore comments and empty lines
    if (line[0] == '#' || line[0] == '\n') {
        return ResultTypes::DEFAULT;
    }

    line = rtrim(line, '\n');

    if (line[0] == '$') {
        setVariable(line);
        return ResultTypes::DEFAULT;
    }

    char* key = strtok(line, ":");
    if (key == NULL) {
        return ResultTypes::DEFAULT;
    }

    char* paramsStr = strtok(NULL, ":");
    std::vector<string> params = getParams(paramsStr);
    return defaultCallback(key, params, filename, callback);
}

}

bool loadScript(const char* filename, void (*callback)(char* key, std::vector<string> params, const char* filename))
{
    FILE* file = fopen(filename, "r");
    if (file == NULL) {
        printf("Failed to load script: %s\n", filename);
        return false;
    }

    char line[512];

    uint8_t skipTo = -1;
    while (fgets(line, sizeof(line), file)) {
        uint16_t indentation = countLeadingChar(line, ' ');
        if (indentation > skipTo) {
            continue;
        }
        skipTo = -1; // will set to max value
        Script::ResultTypes result = Script::parseScriptLine(line, filename, callback);
        if (result == Script::ResultTypes::IF_FALSE) {
            skipTo = indentation;
        }
    }
    fclose(file);

    return true;
}

#endif