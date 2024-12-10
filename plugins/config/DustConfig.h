#ifndef _DUST_CONFIG_H_
#define _DUST_CONFIG_H_

#include "../../dustscript/dustscript.h"

#include <string>
#include <vector>

struct Var {
    std::string key;
    std::string value;
};

void dustConfig(std::string filename, void (*callback)(char* command, char* params, const char* filename), std::vector<Var> variables)
{
    std::vector<DustScript::Variable> vars = { { "$IS_RPI",
#ifdef IS_RPI
        "true"
#else
        "false"
#endif
    } };

    for (int i = 0; i < variables.size(); i++) {
        vars.push_back({ "$" + variables[i].key, variables[i].value });
    }

    DustScript::load(
        filename.c_str(),
        [&callback](char* key, char* value, const char* filename, uint8_t indentation, DustScript& instance) {
            callback(key, value, filename);
        },
        { .variables = vars });
}

#endif
