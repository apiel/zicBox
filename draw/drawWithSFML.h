#pragma once

#include <SFML/Graphics.hpp>

#include "./draw.h"
#include "helpers/getTicks.h"
#include "motion.h"
#include "plugins/components/EventInterface.h"

#ifndef MAX_SCREEN_MOTION
#define MAX_SCREEN_MOTION 5
#endif

class DrawWithSFML : public Draw {
protected:
    sf::RenderWindow window;
    sf::Texture texture;
    sf::Sprite sprite;

    int windowX = 0;
    int windowY = 0;

    Motion motions[MAX_SCREEN_MOTION];

#if SFML_VERSION_MAJOR == 2
    uint8_t emulateEncoderId = 0;
#endif

    // ---- Motion handling (same as your SDL version) ----

    MotionInterface* getMotion(int id)
    {
        for (int i = 0; i < MAX_SCREEN_MOTION; ++i) {
            if (motions[i].id == id) {
                return &motions[i];
            }
        }
        return nullptr;
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

    void handleMotion(EventInterface* view, int x, int y, int id)
    {
        if (id < 0) return;

        MotionInterface* motion = getMotion(id);
        if (motion) {
            motion->move(x, y);
            view->onMotion(*motion);
        }
    }

    void handleMotionDown(EventInterface* view, int x, int y, int id)
    {
        if (id < 0) return;

        MotionInterface* motion = getOldestMotion();
        motion->init(id, x, y);
        view->onMotion(*motion);
    }

    void handleMotionUp(EventInterface* view, int x, int y, int id)
    {
        if (id < 0) return;

        MotionInterface* motion = getMotion(id);
        if (motion) {
            motion->move(x, y);
            view->onMotionRelease(*motion);
            motion->setId(-1);
        }
    }

    // --- Encoder emulation zones ---
    std::vector<Rect> zoneEncoders = {};

    uint8_t getEmulatedEncoderId(int32_t x, int32_t y)
    {
        for (uint8_t i = 0; i < zoneEncoders.size(); i++) {
            if (inRect(zoneEncoders[i], { x, y })) {
                return i;
            }
        }
        return 255;
    }

public:
    DrawWithSFML(Styles& styles)
        : Draw(styles)
    {
    }

    // ------------------------------------------------------------
    // INIT
    // ------------------------------------------------------------
    void init() override
    {
        windowX = windowY = 0;  // default

        sf::VideoMode mode(styles.screen.w, styles.screen.h);

        window.create(mode, "Zic", sf::Style::Titlebar | sf::Style::Close);
        window.setPosition({ windowX, windowY });

        // Create texture for your framebuffer
        texture.create(styles.screen.w, styles.screen.h);
        sprite.setTexture(texture);

        logDebug("SFML initialized with window %dx%d",
                 styles.screen.w, styles.screen.h);
    }

    // ------------------------------------------------------------
    // QUIT
    // ------------------------------------------------------------
    void quit() override
    {
        sf::Vector2i pos = window.getPosition();
        logInfo("Exit on position x: %d, y: %d", pos.x, pos.y);

        window.close();
    }

    // ------------------------------------------------------------
    // RENDER
    // ------------------------------------------------------------
    void render() override
    {
        // Upload framebuffer row per row (RGBA)
        // screenBuffer[y][x] is contiguous per row
        for (int y = 0; y < styles.screen.h; ++y) {
            texture.update(
                reinterpret_cast<const sf::Uint8*>(&screenBuffer[y][0]),
                styles.screen.w,
                1,
                0, y
            );
        }

        window.clear(sf::Color::Black);
        window.draw(sprite);
        window.display();
    }

    // ------------------------------------------------------------
    // CONFIG
    // ------------------------------------------------------------
    void config(nlohmann::json& config) override
    {
        if (config.contains("windowPosition")) {
            auto wp = config["windowPosition"];
            if (wp.contains("x") && wp.contains("y")) {
                windowX = wp["x"];
                windowY = wp["y"];
            }
        }

        if (config.contains("zonesEncoders")) {
            for (auto zoneEncoder : config["zonesEncoders"]) {
                int x = zoneEncoder[0];
                int y = zoneEncoder[1];
                int w = zoneEncoder[2];
                int h = zoneEncoder[3];
                zoneEncoders.push_back({ { x, y }, { w, h } });
            }
        }

        Draw::config(config);
    }

    // ------------------------------------------------------------
    // EVENTS
    // ------------------------------------------------------------
    bool handleEvent(EventInterface* view) override
    {
        sf::Event event;
        while (window.pollEvent(event)) {

            switch (event.type) {

            case sf::Event::Closed:
                return false;

            case sf::Event::MouseMoved:
                handleMotion(view, event.mouseMove.x, event.mouseMove.y, 0);
                return true;

            case sf::Event::MouseButtonPressed:
                handleMotionDown(view, event.mouseButton.x, event.mouseButton.y, 0);
                return true;

            case sf::Event::MouseButtonReleased:
                handleMotionUp(view, event.mouseButton.x, event.mouseButton.y, 0);
                return true;

            case sf::Event::MouseWheelScrolled:
                view->onEncoder(
                    getEmulatedEncoderId(event.mouseWheelScroll.x, event.mouseWheelScroll.y),
                    event.mouseWheelScroll.delta,
                    getTicks()
                );
                return true;

            case sf::Event::TouchMoved:
                handleMotion(
                    view,
                    event.touch.x * styles.screen.w,
                    event.touch.y * styles.screen.h,
                    event.touch.finger
                );
                return true;

            case sf::Event::TouchBegan:
                handleMotionDown(
                    view,
                    event.touch.x * styles.screen.w,
                    event.touch.y * styles.screen.h,
                    event.touch.finger
                );
                return true;

            case sf::Event::TouchEnded:
                handleMotionUp(
                    view,
                    event.touch.x * styles.screen.w,
                    event.touch.y * styles.screen.h,
                    event.touch.finger
                );
                return true;

            case sf::Event::KeyPressed:
                view->onKey(0, event.key.scancode, 1);
                return true;

            case sf::Event::KeyReleased:
                view->onKey(0, event.key.scancode, 0);
                return true;
            }
        }

        return true;
    }
};
