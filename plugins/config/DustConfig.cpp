#include "../../dustscript/dustscript.h"

extern "C" {
void config(const char* filename, void (*callback)(char* command, char* params, const char* filename))
{
    DustScript::load(
        filename,
        [&callback](char* key, char* value, const char* filename, uint8_t indentation, DustScript& instance) {
            callback(key, value, filename);
        },
        { .variables = { { "$IS_RPI",
#ifdef IS_RPI
              "true"
#else
              "false"
#endif
          } } });
}
}
