#ifndef CPP_WRAPPER_H
#define CPP_WRAPPER_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

void Cpp_Init(void);
void Cpp_Loop(void);
void Cpp_TriggerKick(void);

#ifdef __cplusplus
}
#endif

#endif