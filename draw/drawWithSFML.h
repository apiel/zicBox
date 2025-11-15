#pragma once

#include <SFML/Graphics.hpp>

#include "./draw.h"
#include "helpers/getTicks.h"
#include "motion.h"
#include "plugins/components/EventInterface.h"

#ifndef MAX_SCREEN_MOTION
#define MAX_SCREEN_MOTION 5
#endif

uint8_t mapSfmlToSdlScancode(sf::Keyboard::Key key);

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
        windowX = windowY = 0; // default

        sf::VideoMode mode(styles.screen.w, styles.screen.h);

        window.create(mode, "Zic", sf::Style::Titlebar | sf::Style::Close);
        window.setPosition({ windowX, windowY });

        // Create texture for your framebuffer
        texture.create(styles.screen.w, styles.screen.h);
        sprite.setTexture(texture);
        // sprite.setColor(sf::Color(255, 255, 255, 255)); // full opacity

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
        // Fix screenBuffer alpha not being 255
        for (int y = 0; y < styles.screen.h; ++y)
            for (int x = 0; x < styles.screen.w; ++x)
                screenBuffer[y][x].a = 255;

        for (int y = 0; y < styles.screen.h; ++y) {
            texture.update(
                reinterpret_cast<const sf::Uint8*>(&screenBuffer[y][0]),
                styles.screen.w,
                1,
                0, y);
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
                    getTicks());
                return true;

            case sf::Event::TouchMoved:
                handleMotion(
                    view,
                    event.touch.x * styles.screen.w,
                    event.touch.y * styles.screen.h,
                    event.touch.finger);
                return true;

            case sf::Event::TouchBegan:
                handleMotionDown(
                    view,
                    event.touch.x * styles.screen.w,
                    event.touch.y * styles.screen.h,
                    event.touch.finger);
                return true;

            case sf::Event::TouchEnded:
                handleMotionUp(
                    view,
                    event.touch.x * styles.screen.w,
                    event.touch.y * styles.screen.h,
                    event.touch.finger);
                return true;

            case sf::Event::KeyPressed:
                view->onKey(0, mapSfmlToSdlScancode(event.key.code), 1);
                return true;

            case sf::Event::KeyReleased:
                view->onKey(0, mapSfmlToSdlScancode(event.key.code), 0);
                return true;
            }
        }

        return true;
    }
};

// Map SFML logical keys to SDL scancode numbers (USB HID usage codes)
uint8_t mapSfmlToSdlScancode(sf::Keyboard::Key key)
{
    switch (key) {
    // Letters
    case sf::Keyboard::A:
        return 4;
    case sf::Keyboard::B:
        return 5;
    case sf::Keyboard::C:
        return 6;
    case sf::Keyboard::D:
        return 7;
    case sf::Keyboard::E:
        return 8;
    case sf::Keyboard::F:
        return 9;
    case sf::Keyboard::G:
        return 10;
    case sf::Keyboard::H:
        return 11;
    case sf::Keyboard::I:
        return 12;
    case sf::Keyboard::J:
        return 13;
    case sf::Keyboard::K:
        return 14;
    case sf::Keyboard::L:
        return 15;
    case sf::Keyboard::M:
        return 16;
    case sf::Keyboard::N:
        return 17;
    case sf::Keyboard::O:
        return 18;
    case sf::Keyboard::P:
        return 19;
    case sf::Keyboard::Q:
        return 20;
    case sf::Keyboard::R:
        return 21;
    case sf::Keyboard::S:
        return 22;
    case sf::Keyboard::T:
        return 23;
    case sf::Keyboard::U:
        return 24;
    case sf::Keyboard::V:
        return 25;
    case sf::Keyboard::W:
        return 26;
    case sf::Keyboard::X:
        return 27;
    case sf::Keyboard::Y:
        return 28;
    case sf::Keyboard::Z:
        return 29;

    // Numbers
    case sf::Keyboard::Num0:
        return 39;
    case sf::Keyboard::Num1:
        return 30;
    case sf::Keyboard::Num2:
        return 31;
    case sf::Keyboard::Num3:
        return 32;
    case sf::Keyboard::Num4:
        return 33;
    case sf::Keyboard::Num5:
        return 34;
    case sf::Keyboard::Num6:
        return 35;
    case sf::Keyboard::Num7:
        return 36;
    case sf::Keyboard::Num8:
        return 37;
    case sf::Keyboard::Num9:
        return 38;

    // Special keys
    case sf::Keyboard::Return:
        return 40;
    case sf::Keyboard::Escape:
        return 41;
    case sf::Keyboard::Backspace:
        return 42;
    case sf::Keyboard::Tab:
        return 43;
    case sf::Keyboard::Space:
        return 44;
    case sf::Keyboard::Dash:
        return 45;
    case sf::Keyboard::Equal:
        return 46;
    case sf::Keyboard::LBracket:
        return 47;
    case sf::Keyboard::RBracket:
        return 48;
    case sf::Keyboard::BackSlash:
        return 49;
    case sf::Keyboard::SemiColon:
        return 51;
    case sf::Keyboard::Quote:
        return 52;
    case sf::Keyboard::Comma:
        return 54;
    case sf::Keyboard::Period:
        return 55;
    case sf::Keyboard::Slash:
        return 56;

    // Modifier keys
    case sf::Keyboard::LControl:
        return 224;
    case sf::Keyboard::LShift:
        return 225;
    case sf::Keyboard::LAlt:
        return 226;
    case sf::Keyboard::LSystem:
        return 227;
    case sf::Keyboard::RControl:
        return 228;
    case sf::Keyboard::RShift:
        return 229;
    case sf::Keyboard::RAlt:
        return 230;
    case sf::Keyboard::RSystem:
        return 231;

    // Arrow keys
    case sf::Keyboard::Up:
        return 82;
    case sf::Keyboard::Down:
        return 81;
    case sf::Keyboard::Left:
        return 80;
    case sf::Keyboard::Right:
        return 79;

    // Function keys
    case sf::Keyboard::F1:
        return 58;
    case sf::Keyboard::F2:
        return 59;
    case sf::Keyboard::F3:
        return 60;
    case sf::Keyboard::F4:
        return 61;
    case sf::Keyboard::F5:
        return 62;
    case sf::Keyboard::F6:
        return 63;
    case sf::Keyboard::F7:
        return 64;
    case sf::Keyboard::F8:
        return 65;
    case sf::Keyboard::F9:
        return 66;
    case sf::Keyboard::F10:
        return 67;
    case sf::Keyboard::F11:
        return 68;
    case sf::Keyboard::F12:
        return 69;

    // Other keys you might need
    case sf::Keyboard::Insert:
        return 73;
    case sf::Keyboard::Home:
        return 74;
    case sf::Keyboard::PageUp:
        return 75;
    case sf::Keyboard::Delete:
        return 76;
    case sf::Keyboard::End:
        return 77;
    case sf::Keyboard::PageDown:
        return 78;
    case sf::Keyboard::Pause:
        return 119;

    default:
        return 0xFF; // unsupported key
    }
}
