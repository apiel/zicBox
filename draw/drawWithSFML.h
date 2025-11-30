/** Description:
This code defines a specialized component responsible for handling all graphical output and user interaction on a desktop environment using the **SFML (Simple and Fast Multimedia Library)**.

This component, named `DrawWithSFML`, acts as a specific "driver" for displaying content. It inherits features from a generic desktop drawing class, allowing the main application to remain independent of the specific graphics library being used.

### How it Works

1.  **Window Management:** The component first initializes itself, reading environment settings to determine the window's starting size and position. It then creates the main graphical window, complete with a title bar and controls for minimizing or closing.

2.  **Displaying Content:** Instead of drawing directly, the class prepares a digital canvas, known as a "texture." The application's internal image data is regularly copied onto this texture. The `render` function then displays this texture in the window, effectively refreshing the screen. It also handles dynamic window resizing, ensuring the content scales correctly if the user adjusts the window size.

3.  **Handling Input:** The most complex task is managing user input. The component continuously monitors the window for events, including:
    *   Closing or resizing the window.
    *   Mouse movements, clicks, and scrolling.
    *   Touchscreen actions (motion, press, release).
    *   Keyboard presses.

4.  **Key Translation:** Since SFML has its own way of identifying keys, a dedicated function translates every detected key press (like the 'A' key) into a standard numerical identifier (a "scancode"). This ensures that the application receives consistent input regardless of whether it's running with SFML or a different graphics engine.

sha: 242430e8d20f30d47724366d1f4a2c77a6aa89ada582c8c73abcc71efdec32f9 
*/
#pragma once

#include <SFML/Graphics.hpp>

#include "draw/drawDesktop.h"
#include "helpers/environment.h"
#include "helpers/getTicks.h"

uint8_t mapSfmlToSdlScancode(sf::Keyboard::Key key);

class DrawWithSFML : public DrawDesktop {
protected:
    sf::RenderWindow window;
    sf::Texture texture;
    sf::Sprite sprite;

public:
    DrawWithSFML(Styles& styles)
        : DrawDesktop(styles)
    {
    }

    void init() override
    {
        Size size = getEnvWindowSize();
        if (size.w != -1 && size.h != -1) {
            screenSize = size;
        }

        sf::VideoMode mode(screenSize.w, screenSize.h);

        window.create(mode, "Zic", sf::Style::Titlebar | sf::Style::Close | sf::Style::Resize);

        Point pos = getEnvWindowPosition();
        if (pos.x != -1 && pos.y != -1) {
            windowX = pos.x;
            windowY = pos.y;
        }
        window.setPosition({ windowX, windowY });

        // Create texture for your framebuffer
        // texture.create(screenSize.w, screenSize.h);
        texture.create(SCREEN_BUFFER_ROWS, SCREEN_BUFFER_COLS);
        sprite.setTexture(texture);

        logDebug("SFML initialized with window %dx%d at position %dx%d", screenSize.w, screenSize.h, windowX, windowY);
    }

    void quit() override
    {
        sf::Vector2i pos = window.getPosition();
        logInfo("Exit on position x: %d, y: %d", pos.x, pos.y);

        window.close();
    }

    Point getWindowPosition() override
    {
        sf::Vector2i pos = window.getPosition();
        return { pos.x, pos.y };
    }

    Size getWindowSize()
    {
        sf::Vector2u size = window.getSize();
        return { (int)size.x, (int)size.y };
    }

    void render() override
    {
        for (int y = 0; y < screenSize.h; ++y) {
            texture.update(
                reinterpret_cast<const sf::Uint8*>(&screenBuffer[y][0]),
                screenSize.w,
                1,
                0, y);
        }

        window.clear(sf::Color::Black);
        window.draw(sprite);
        window.display();
    }

    Size prevSize = { 0, 0 };
    uint64_t needResize = 0;
    void preRender(EventInterface* view, uint64_t now) override
    {
        if (needResize) {
            // Update view so SFML does not auto-stretch your content
            sf::View v(sf::FloatRect(0, 0, screenSize.w, screenSize.h));
            window.setView(v);
            if (needResize < now - 100) { // let's debounce 100ms
                needResize = 0;
                view->resize();
                clear();
            } else {
                // draw background rect on new outbound size till final resize happen
                filledRect({ prevSize.w, 0 }, { screenSize.w - prevSize.w, screenSize.h }, { styles.colors.background });
                filledRect({ 0, prevSize.h }, { screenSize.w, screenSize.h - prevSize.h }, { styles.colors.background });
                renderNext();
            }
        }
    }

    bool handleEvent(EventInterface* view) override
    {
        sf::Event event;
        while (window.pollEvent(event)) {

            switch (event.type) {

            case sf::Event::Closed:
                return false;

            case sf::Event::Resized: {
                prevSize = getScreenSize();
                setScreenSize({ (int)event.size.width, (int)event.size.height });
                needResize = getTicks();

                return true;
            }

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
                    view->getEncoderId(event.mouseWheelScroll.x, event.mouseWheelScroll.y),
                    event.mouseWheelScroll.delta,
                    getTicks());
                return true;

            case sf::Event::TouchMoved:
                handleMotion(
                    view,
                    event.touch.x * screenSize.w,
                    event.touch.y * screenSize.h,
                    event.touch.finger);
                return true;

            case sf::Event::TouchBegan:
                handleMotionDown(
                    view,
                    event.touch.x * screenSize.w,
                    event.touch.y * screenSize.h,
                    event.touch.finger);
                return true;

            case sf::Event::TouchEnded:
                handleMotionUp(
                    view,
                    event.touch.x * screenSize.w,
                    event.touch.y * screenSize.h,
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
