#ifndef _ZIC_LOG_H_
#define _ZIC_LOG_H_

#include <string>
#include <stdarg.h>

#define ZIC_LOG_DEBUG 1
#define ZIC_LOG_INFO 2
#define ZIC_LOG_WARN 3
#define ZIC_LOG_ERROR 4

#ifndef ZIC_LOG_LEVEL
#define ZIC_LOG_LEVEL ZIC_LOG_DEBUG
#endif

void showLogLevel() {
    if (ZIC_LOG_LEVEL == ZIC_LOG_DEBUG)
        printf("log level DEBUG\n");
    if (ZIC_LOG_LEVEL == ZIC_LOG_INFO)
        printf("log level INFO\n");
    if (ZIC_LOG_LEVEL == ZIC_LOG_WARN)
        printf("log level WARN\n");
    if (ZIC_LOG_LEVEL == ZIC_LOG_ERROR)
        printf("log level ERROR\n");
}

void logDebug(std::string message) {
#if ZIC_LOG_LEVEL <= ZIC_LOG_DEBUG
    printf("\033[32m[debug]\033[0m %s\n", message.c_str());
#endif
}

void logDebug(const char* message, ...) {
#if ZIC_LOG_LEVEL <= ZIC_LOG_DEBUG
    va_list args;
    va_start(args, message);
    printf("\033[32m[debug]\033[0m ");
    vprintf(message, args);
    printf("\n");
    va_end(args);
#endif
}

void logInfo(std::string message) {
#if ZIC_LOG_LEVEL <= ZIC_LOG_INFO
    printf("\033[34m[info]\033[0m %s\n", message.c_str());
#endif
}

void logInfo(const char* message, ...) {
#if ZIC_LOG_LEVEL <= ZIC_LOG_INFO
    va_list args;
    va_start(args, message);
    printf("\033[34m[info]\033[0m ");
    vprintf(message, args);
    printf("\n");
    va_end(args);
#endif
}

void logWarn(std::string message) {
#if ZIC_LOG_LEVEL <= ZIC_LOG_WARN
    printf("\033[33m[warn]\033[0m %s\n", message.c_str());
#endif
}

void logWarn(const char* message, ...) {
#if ZIC_LOG_LEVEL <= ZIC_LOG_WARN
    va_list args;
    va_start(args, message);
    printf("\033[33m[warn]\033[0m ");
    vprintf(message, args);
    printf("\n");
    va_end(args);
#endif
}

void logError(std::string message) {
#if ZIC_LOG_LEVEL <= ZIC_LOG_ERROR
    printf("\033[31m[error]\033[0m %s\n", message.c_str());
#endif
}

void logError(const char* message, ...) {
#if ZIC_LOG_LEVEL <= ZIC_LOG_ERROR
    va_list args;
    va_start(args, message);
    printf("\033[31m[error]\033[0m ");
    vprintf(message, args);
    printf("\n");
    va_end(args);
#endif
}

#endif
