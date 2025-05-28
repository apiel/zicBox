#pragma once

#include <cstdio>
#include <cstring>
#include <fcntl.h>
#include <limits.h>
#include <pthread.h>
#include <string>
#include <sys/inotify.h>
#include <sys/stat.h>
#include <thread>
#include <unistd.h>
#include <functional>

#include "log.h"

struct ConfigWatcherParams {
    std::string configFilepath;
    bool* running;
    std::function<void()> callback;
};

void* watchConfig(void* params)
{
    auto* p = static_cast<ConfigWatcherParams*>(params);

    std::string configFilename;
    std::string dir;
    size_t lastSlash = p->configFilepath.find_last_of('/');
    if (lastSlash != std::string::npos) {
        dir = p->configFilepath.substr(0, lastSlash);
        configFilename = p->configFilepath.substr(lastSlash + 1);
    } else {
        dir = ".";
        configFilename = p->configFilepath;
    }

    int fd = inotify_init1(IN_NONBLOCK);
    if (fd < 0) {
        perror("inotify_init");
        return nullptr;
    }

    int wd = inotify_add_watch(fd, dir.c_str(),
        IN_MODIFY | IN_CLOSE_WRITE | IN_ATTRIB | IN_DELETE_SELF | IN_MOVE_SELF | IN_MOVED_TO);
    if (wd < 0) {
        perror("inotify_add_watch");
        close(fd);
        return nullptr;
    }

    logInfo("Watching directory: " + dir + " for file: " + configFilename);

    char buffer[sizeof(struct inotify_event) + NAME_MAX + 1];
    while (*(p->running)) {
        ssize_t length = read(fd, buffer, sizeof(buffer));
        if (length < 0) {
            if (errno == EAGAIN) {
                usleep(100000); // sleep 100ms
                continue;
            } else {
                perror("read");
                break;
            }
        }

        for (char* ptr = buffer; ptr < buffer + length;) {
            struct inotify_event* event = (struct inotify_event*)ptr;

            if (event->len > 0 && configFilename == event->name) {
                if (event->mask & (IN_MODIFY | IN_CLOSE_WRITE | IN_ATTRIB | IN_MOVED_TO)) {
                    logInfo("Config file changed: " + p->configFilepath + ", exiting app.");
                    p->callback();
                    *(p->running) = false;
                    close(wd);
                    close(fd);
                    return nullptr;
                }
            }

            ptr += sizeof(struct inotify_event) + event->len;
        }
    }

    close(wd);
    close(fd);
    return nullptr;
}

pthread_t configWatcher(const std::string& filepath, bool* running, std::function<void()> callback)
{
    pthread_t watcherTid;
    const char* watchEnv = std::getenv("WATCH");
    if (watchEnv && (watchEnv[0] == '1' || strcmp(watchEnv, "true") == 0)) {
        logDebug("Starting config watcher...");

        auto* params = new ConfigWatcherParams { filepath, running, callback };

        if (pthread_create(&watcherTid, nullptr, &watchConfig, params) != 0) {
            perror("pthread_create");
            delete params;
        } else {
            pthread_setname_np(watcherTid, "watchConfig");
        }

        return watcherTid;
    }

    return {};
}
