#ifndef _LOG_H_
#define _LOG_H_

#include <string>

#define LOG_DEBUG 1
#define LOG_INFO 2
#define LOG_WARN 3
#define LOG_ERROR 4

#ifdef LOG_LEVEL
#define LOG_LEVEL LOG_DEBUG
#endif

void logDebug(std::string message) {
#if LOG_LEVEL >= LOG_DEBUG
    printf("[debug] %s\n", message.c_str());
#endif
}

void logDebug(const char* message, ...) {
#if LOG_LEVEL >= LOG_DEBUG
    va_list args;
    va_start(args, message);
    printf("[debug] ");
    vprintf(message, args);
    printf("\n");
    va_end(args);
#endif
}

void logInfo(std::string message) {
#if LOG_LEVEL >= LOG_INFO
    printf("[info] %s\n", message.c_str());
#endif
}

void logInfo(const char* message, ...) {
#if LOG_LEVEL >= LOG_INFO
    va_list args;
    va_start(args, message);
    printf("[info] ");
    vprintf(message, args);
    printf("\n");
    va_end(args);
#endif
}

void logWarn(std::string message) {
#if LOG_LEVEL >= LOG_WARN
    printf("[warn] %s\n", message.c_str());
#endif
}

void logWarn(const char* message, ...) {
#if LOG_LEVEL >= LOG_WARN
    va_list args;
    va_start(args, message);
    printf("[warn] ");
    vprintf(message, args);
    printf("\n");
    va_end(args);
#endif
}

void logError(std::string message) {
#if LOG_LEVEL >= LOG_ERROR
    printf("[error] %s\n", message.c_str());
#endif
}

void logError(const char* message, ...) {
#if LOG_LEVEL >= LOG_ERROR
    va_list args;
    va_start(args, message);
    printf("[error] ");
    vprintf(message, args);
    printf("\n");
    va_end(args);
#endif
}

#endif
