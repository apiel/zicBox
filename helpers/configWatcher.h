#pragma once

#include <cstdio>
#include <fcntl.h>
#include <limits.h>
#include <string>
#include <sys/inotify.h>
#include <sys/stat.h>
#include <thread>
#include <unistd.h>

#include "log.h"
#include <string.h>

struct ConfigWatcherParams {
    std::string& configFilepath;
    bool& running;
};

void* watchConfig(void* params)
{
    ConfigWatcherParams* p = (ConfigWatcherParams*)params;

    int fd = inotify_init1(IN_NONBLOCK);
    if (fd < 0) {
        perror("inotify_init");
        return NULL;
    }

    int wd = inotify_add_watch(fd, p->configFilepath.c_str(), IN_MODIFY);
    if (wd < 0) {
        perror("inotify_add_watch");
        close(fd);
        return NULL;
    }

    logInfo("Watching for changes to " + p->configFilepath + "...");

    char buffer[sizeof(struct inotify_event) + NAME_MAX + 1];
    while (true) {
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
            if (event->mask & IN_MODIFY) {
                printf("CONFIG_CHANGED\n");
                p->running = false;
                close(wd);
                close(fd);
                return NULL; // exit app
            }
            ptr += sizeof(struct inotify_event) + event->len;
        }
    }

    close(wd);
    close(fd);
    return NULL;
}

pthread_t configWatcher(ConfigWatcherParams params)
{
    pthread_t watcherTid;
    const char* watchEnv = std::getenv("WATCH");
    if (watchEnv && (watchEnv[0] == '1' || strcmp(watchEnv, "true") == 0)) {
        logDebug("Starting config watcher.");
        pthread_create(&watcherTid, NULL, &watchConfig, &params);
        pthread_setname_np(watcherTid, "watchConfig");
    }

    return watcherTid;
}