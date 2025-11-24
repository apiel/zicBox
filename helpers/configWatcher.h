/** Description:
This C++ header file defines a robust system for monitoring configuration files in the background. It serves as a real-time watchdog for a critical application setting.

**Basic Idea:**
The system ensures that if the application's configuration file is changed or saved while the application is running, the application immediately detects this change and initiates a specified action, usually a controlled shutdown or reload.

**How it Works:**

1.  **Background Monitoring:** The core monitoring logic runs independently in a separate, dedicated process called a "thread." This allows the application to perform its main duties without being slowed down by constant file checks.
2.  **System Notification:** Instead of polling the file repeatedly, the code utilizes a specialized operating system feature (common in Linux systems) that sets up a direct notification link to the directory containing the file. This is like setting an alarm: the operating system only sends an alert when a specific event occurs.
3.  **Triggering the Action:** If an event, such as a file modification or a save operation, is detected on the watched configuration file, the thread receives an immediate alert.
4.  **Response:** Once the alert is confirmed, the monitoring thread executes a predefined function (an action determined by the main application) and signals the main application to stop its operation.
5.  **Activation Control:** This monitoring feature is optional. It will only start running if a specific environment setting (the `WATCH` environment variable) is enabled, allowing developers to control its behavior in different execution environments.

sha: 2307d1c1c5a960871b59f98e951665bb986eb2ad096fa9459daf50b7b0927e85 
*/
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
