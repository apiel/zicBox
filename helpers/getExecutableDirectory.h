#pragma once

#include <iostream>
#include <limits.h>
#include <libgen.h>
#include <unistd.h>

std::string getExecutableDirectory() {
    char buffer[PATH_MAX];
    ssize_t len = readlink("/proc/self/exe", buffer, sizeof(buffer) - 1);
    if (len != -1) {
        buffer[len] = '\0';
        char *dir = dirname(buffer);
        return std::string(dir);
    } else {
        std::cerr << "Error getting executable path" << std::endl;
        return "";
    }
}
