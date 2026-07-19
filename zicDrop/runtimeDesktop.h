#pragma once

#include <SFML/Graphics.hpp>
#include <cstdlib>
#include <string>
#include <cstring>
#include <iostream>
#include <mutex>

#include "draw/draw.h"
#include "uiDrop.h"
#include "sequenceBrain.h"
#include "audio/engines/drop.h"

// Buffer size for SFML texture update
static constexpr int BUFFER_SIZE = 2048;
extern std::atomic<bool> keep_running;
extern std::mutex audioMutex;

void runDesktopSFML(Draw& d, bool& needFullRedraw, UiDrop& ui, SequenceBrain& brain, Drop& audio)
{
    sf::RenderWindow window(sf::VideoMode(1920, 360), "zicDrop - Generative Groovebox Synth");
    window.setFramerateLimit(60);
    window.setKeyRepeatEnabled(false);

    sf::Texture screenTexture;
    screenTexture.create(BUFFER_SIZE, BUFFER_SIZE);
    sf::Sprite screenSprite(screenTexture);
    std::vector<sf::Uint8> pixelBuffer(BUFFER_SIZE * BUFFER_SIZE * 4, 15);

    sf::Vector2u winSize = window.getSize();

    while (window.isOpen() && keep_running) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                window.close();
            } else if (event.type == sf::Event::Resized) {
                window.setView(sf::View(sf::FloatRect(0, 0, (float)event.size.width, (float)event.size.height)));
                needFullRedraw = true;
            } else if (event.type == sf::Event::MouseButtonPressed) {
                if (event.mouseButton.button == sf::Mouse::Left) {
                    std::lock_guard<std::mutex> lock(audioMutex);
                    if (ui.handleMouseButtonPressed((float)event.mouseButton.x, (float)event.mouseButton.y)) {
                        needFullRedraw = true;
                    }
                }
            } else if (event.type == sf::Event::MouseMoved) {
                std::lock_guard<std::mutex> lock(audioMutex);
                if (ui.handleMouseMoved((float)event.mouseMove.x, (float)event.mouseMove.y)) {
                    needFullRedraw = true;
                }
            } else if (event.type == sf::Event::MouseButtonReleased) {
                if (event.mouseButton.button == sf::Mouse::Left) {
                    std::lock_guard<std::mutex> lock(audioMutex);
                    if (ui.handleMouseButtonReleased()) {
                        needFullRedraw = true;
                    }
                }
            } else if (event.type == sf::Event::MouseWheelScrolled) {
                std::lock_guard<std::mutex> lock(audioMutex);
                float mx = (float)event.mouseWheelScroll.x;
                float my = (float)event.mouseWheelScroll.y;
                float delta = event.mouseWheelScroll.delta;
                if (ui.handleMouseWheel(mx, my, delta)) {
                    needFullRedraw = true;
                }
            } else if (event.type == sf::Event::KeyPressed) {
                std::lock_guard<std::mutex> lock(audioMutex);
                if (event.key.code == sf::Keyboard::Space) {
                    brain.spacebarHeld = true;
                    needFullRedraw = true;
                } else if (event.key.code == sf::Keyboard::F1) {
                    ui.activeSection = SECTION_BRAIN;
                    needFullRedraw = true;
                } else if (event.key.code == sf::Keyboard::F2) {
                    ui.activeSection = SECTION_KICK;
                    needFullRedraw = true;
                } else if (event.key.code == sf::Keyboard::F4) {
                    ui.activeSection = SECTION_SYNTH;
                    needFullRedraw = true;
                } else if (event.key.code == sf::Keyboard::F5) {
                    ui.activeSection = SECTION_MASTER;
                    needFullRedraw = true;
                }
            } else if (event.type == sf::Event::KeyReleased) {
                std::lock_guard<std::mutex> lock(audioMutex);
                if (event.key.code == sf::Keyboard::Space) {
                    brain.spacebarHeld = false;
                    needFullRedraw = true;
                }
            }
        }

        if (needFullRedraw) {
            winSize = window.getSize();
            d.setScreenSize({ (int)winSize.x, (int)winSize.y });
        }

        // Draw and update SFML texture buffer from Draw interface
        if (ui.draw(d, winSize.x, winSize.y, needFullRedraw, brain, audio)) {
            for (unsigned int y = 0; y < winSize.y; ++y) {
                std::memcpy(&pixelBuffer[y * BUFFER_SIZE * 4], d.screenBuffer[y], winSize.x * 4);
            }
            screenTexture.update(pixelBuffer.data());
        }

        window.clear();
        window.draw(screenSprite);
        window.display();
    }
}
