#include "./DustConfig.h"

extern "C" {
void config(std::string filename, void (*callback)(char* command, char* params, const char* filename), std::vector<DustVar> variables)
{
    dustConfig(filename, callback, variables);
}
}
