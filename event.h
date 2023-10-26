#ifndef _UI_EVENT_H_
#define _UI_EVENT_H_

#include "state.h"
#include "viewManager.h"
#include "motion.h"

#ifndef MAX_SCREEN_MOTION
// The current display only support 5 touch point
#define MAX_SCREEN_MOTION 5
#endif

#ifndef ENCODER_COUNT
#define ENCODER_COUNT 8
#endif

class EventHandler
{
protected:
    Motion motions[MAX_SCREEN_MOTION];
    ViewManager &viewManager = ViewManager::get();
    int encoderWidth = styles.screen.w / ENCODER_COUNT;

#if SDL_MINOR_VERSION <= 24
    uint8_t emulateEncoderId = 0;
#endif

    static EventHandler *instance;
    EventHandler() {}

    MotionInterface *getMotion(int id)
    {
        for (int i = 0; i < MAX_SCREEN_MOTION; ++i)
        {
            if (motions[i].id == id)
            {
                return &motions[i];
            }
        }
        return NULL;
    }

    MotionInterface *getOldestMotion()
    {
        MotionInterface *oldest = &motions[0];
        for (int i = 1; i < MAX_SCREEN_MOTION; ++i)
        {
            if (motions[i].id < oldest->id)
            {
                oldest = &motions[i];
            }
        }
        return oldest;
    }

    void handleMotion(int x, int y, int id)
    {
        if (id < 0)
        {
            return;
        }

#ifndef IS_RPI
#if SDL_MINOR_VERSION <= 24
        emulateEncoderId = x / encoderWidth;
#endif
#endif

        MotionInterface *motion = getMotion(id);
        if (motion)
        {
            motion->move(x, y);
            viewManager.onMotion(*motion);
        }
    }

    void handleMotionUp(int x, int y, int id)
    {
        if (id < 0)
        {
            return;
        }

        MotionInterface *motion = getMotion(id);
        if (motion)
        {
            motion->move(x, y);
            viewManager.onMotionRelease(*motion);
            motion->setId(-1);
        }
    }

    void handleMotionDown(int x, int y, int id)
    {
        if (id < 0)
        {
            return;
        }

        MotionInterface *motion = getOldestMotion();
        motion->init(id, x, y);
        viewManager.onMotion(*motion);
    }

    void emulateEncoder(SDL_MouseWheelEvent wheel)
    {
#if SDL_MINOR_VERSION > 24
        uint8_t emulateEncoderId = wheel.mouseX / encoderWidth;
#endif
        viewManager.onEncoder(emulateEncoderId, wheel.y);
    }

public:
    static EventHandler &get()
    {
        if (!instance)
        {
            instance = new EventHandler();
        }
        return *instance;
    }

    bool handle()
    {
        SDL_Event event;

        while (SDL_PollEvent(&event))
        {
            switch (event.type)
            {
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
            }
        }
        return true;
    }
};

EventHandler *EventHandler::instance = NULL;

#endif