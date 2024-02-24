#include "../../dustscript/dustscript.h"

void (*cb)(char* command, char* params, const char* filename) = nullptr;

void dustCallback(char* key, char* value, const char* filename, uint8_t indentation, DustScript& instance)
{
	if (cb) {
		cb(key, value, filename);
	}
}

extern "C" {
void config(const char* filename, void (*callback)(char* command, char* params, const char* filename))
{
	cb = callback;
    DustScript::load(filename, dustCallback, { .variables = { { "$IS_RPI",
#ifdef IS_RPI
                                                       "true"
#else
                                                       "false"
#endif
                                                   } } });
}
}
