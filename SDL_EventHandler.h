#ifndef _UI_SDL_EVENT_HANDLER_H_
#define _UI_SDL_EVENT_HANDLER_H_

#include "motion.h"
#include "viewManager.h"
#include "timer.h"

#ifndef MAX_SCREEN_MOTION
// The current display only support 5 touch point
#define MAX_SCREEN_MOTION 5
#endif

class EventHandler {
protected:
    Motion motions[MAX_SCREEN_MOTION];
    ViewManager& viewManager = ViewManager::get();

#if SDL_MINOR_VERSION <= 24
    uint8_t emulateEncoderId = 0;
#endif

    static EventHandler* instance;
    EventHandler() { }

    MotionInterface* getMotion(int id)
    {
        for (int i = 0; i < MAX_SCREEN_MOTION; ++i) {
            if (motions[i].id == id) {
                return &motions[i];
            }
        }
        return NULL;
    }

    MotionInterface* getOldestMotion()
    {
        MotionInterface* oldest = &motions[0];
        for (int i = 1; i < MAX_SCREEN_MOTION; ++i) {
            if (motions[i].id < oldest->id) {
                oldest = &motions[i];
            }
        }
        return oldest;
    }

    void handleMotion(int x, int y, int id)
    {
        if (id < 0) {
            return;
        }

#ifndef IS_RPI
#if SDL_MINOR_VERSION <= 24
        emulateEncoderId = getEmulatedEncoderId(x, y);
#endif
#endif

        MotionInterface* motion = getMotion(id);
        if (motion) {
            motion->move(x, y);
            viewManager.view->onMotion(*motion);
        }
    }

    void handleMotionUp(int x, int y, int id)
    {
        if (id < 0) {
            return;
        }

        MotionInterface* motion = getMotion(id);
        if (motion) {
            motion->move(x, y);
            viewManager.view->onMotionRelease(*motion);
            motion->setId(-1);
        }
    }

    void handleMotionDown(int x, int y, int id)
    {
        if (id < 0) {
            return;
        }

        MotionInterface* motion = getOldestMotion();
        motion->init(id, x, y);
        viewManager.view->onMotion(*motion);
    }

    std::vector<Rect> zoneEncoders = {};
    uint8_t getEmulatedEncoderId(int32_t x, int32_t y)
    {
        for (uint8_t i = 0; i < zoneEncoders.size(); i++) {
            // if (x >= zoneEncoders[i].position.x && x < zoneEncoders[i].position.x + zoneEncoders[i].size.w &&
            //     y >= zoneEncoders[i].position.y && y < zoneEncoders[i].position.y + zoneEncoders[i].size.h) {
            if (inRect(zoneEncoders[i], { x, y })) {
                return i;
            }
        }

        return 255;
    }

    void emulateEncoder(SDL_MouseWheelEvent wheel)
    {
#if SDL_MINOR_VERSION > 24
        uint8_t emulateEncoderId = getEmulatedEncoderId(wheel.mouseX, wheel.mouseY);
#endif
        viewManager.view->onEncoder(emulateEncoderId, wheel.y, getTicks());
    }

public:
    static EventHandler& get()
    {
        if (!instance) {
            instance = new EventHandler();
        }
        return *instance;
    }

    bool config(char* key, char* value)
    {
        if (strcmp(key, "ADD_ZONE_ENCODER") == 0) {
            int x = atoi(strtok(value, " "));
            int y = atoi(strtok(NULL, " "));
            int w = atoi(strtok(NULL, " "));
            int h = atoi(strtok(NULL, " "));
            zoneEncoders.push_back({ { x, y }, { w, h } });
            return true;
        }

        return false;
    }

    bool handle()
    {
        SDL_Event event;

        while (SDL_PollEvent(&event)) {
            switch (event.type) {
            case SDL_QUIT:
                SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "Quit");
                return false;

            case SDL_MOUSEMOTION:
                handleMotion(event.motion.x, event.motion.y, event.motion.which);
                return true;

            case SDL_MOUSEBUTTONDOWN:
                handleMotionDown(event.motion.x, event.motion.y, event.motion.which);
                return true;

            case SDL_MOUSEBUTTONUP:
                handleMotionUp(event.motion.x, event.motion.y, event.motion.which);
                return true;

            case SDL_MOUSEWHEEL:
                emulateEncoder(event.wheel);
                return true;

            case SDL_FINGERMOTION:
                handleMotion(event.tfinger.x * styles.screen.w, event.tfinger.y * styles.screen.h, event.tfinger.fingerId);
                return true;

            case SDL_FINGERDOWN:
                handleMotionDown(event.tfinger.x * styles.screen.w, event.tfinger.y * styles.screen.h, event.tfinger.fingerId);
                return true;

            case SDL_FINGERUP:
                handleMotionUp(event.tfinger.x * styles.screen.w, event.tfinger.y * styles.screen.h, event.tfinger.fingerId);
                return true;

            case SDL_KEYDOWN: {
                if (event.key.repeat) {
                    return true;
                }
                // printf("key %d\n", event.key.keysym.scancode);
                viewManager.view->onKey(0, event.key.keysym.scancode, 1);
                return true;
            }

            case SDL_KEYUP: {
                if (event.key.repeat) {
                    return true;
                }
                viewManager.view->onKey(0, event.key.keysym.scancode, 0);
                return true;
            }
            }
        }
        return true;
    }
};

EventHandler* EventHandler::instance = NULL;

#endif