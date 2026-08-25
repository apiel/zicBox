#pragma once

#include <SFML/Graphics.hpp>
#include <atomic>
#include <cstring>
#include <iostream>
#include <vector>

#include "zicTeK/audioWorker.h"
#include "zicTeK/studio.h"
#include "zicTeK/ui.h"

extern std::atomic<bool> keep_running;

inline void runDesktopSFML(Draw& d)
{
    unsigned int winW = 1024;
    unsigned int winH = 720;

    sf::RenderWindow window(sf::VideoMode(winW, winH), "zicTeK - Kick Synth & 16-Step Sequencer", sf::Style::Titlebar | sf::Style::Close | sf::Style::Resize);
    window.setFramerateLimit(60);
    window.setKeyRepeatEnabled(false);

    d.setScreenSize({ (int)winW, (int)winH });

    sf::Texture screenTexture;
    screenTexture.create(winW, winH);
    sf::Sprite screenSprite(screenTexture);
    std::vector<sf::Uint8> pixelBuffer(winW * winH * 4, 0);

    bool isMousePressed = false;

    while (window.isOpen() && keep_running) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                window.close();
                keep_running = false;
            } else if (event.type == sf::Event::Resized) {
                winW = event.size.width;
                winH = event.size.height;
                window.setView(sf::View(sf::FloatRect(0.0f, 0.0f, (float)winW, (float)winH)));
                d.setScreenSize({ (int)winW, (int)winH });

                screenTexture.create(winW, winH);
                screenSprite.setTexture(screenTexture, true);
                pixelBuffer.resize(winW * winH * 4);
            } else if (event.type == sf::Event::MouseButtonPressed) {
                if (event.mouseButton.button == sf::Mouse::Left) {
                    isMousePressed = true;
                    uiZicTeK.onMouseDown(event.mouseButton.x, event.mouseButton.y);
                }
            } else if (event.type == sf::Event::MouseMoved) {
                if (isMousePressed) {
                    uiZicTeK.onMouseDrag(event.mouseMove.x, event.mouseMove.y);
                }
            } else if (event.type == sf::Event::MouseButtonReleased) {
                if (event.mouseButton.button == sf::Mouse::Left) {
                    isMousePressed = false;
                    uiZicTeK.onMouseUp();
                }
            } else if (event.type == sf::Event::MouseWheelScrolled) {
                uiZicTeK.onMouseWheel(event.mouseWheelScroll.x, event.mouseWheelScroll.y, event.mouseWheelScroll.delta);
            } else if (event.type == sf::Event::KeyPressed) {
                if (event.key.code == sf::Keyboard::Space) {
                    studio.isPlaying = !studio.isPlaying;
                } else if (event.key.code == sf::Keyboard::Num1) {
                    std::lock_guard<std::mutex> lock(studio.audioMutex);
                    studio.track0.kick.noteOn(60, 0.9f);
                }
            }
        }

        // 1. Draw UI onto Draw's screen buffer
        uiZicTeK.draw(d);

        // 2. Copy screen buffer into pixel buffer for SFML texture
        int curW = d.screenSize.w;
        int curH = d.screenSize.h;
        if ((size_t)(curW * curH * 4) <= pixelBuffer.size()) {
            for (int y = 0; y < curH; y++) {
                std::memcpy(&pixelBuffer[y * curW * 4], d.screenBuffer[y], curW * 4);
            }
            screenTexture.update(pixelBuffer.data());
        }

        window.clear();
        window.draw(screenSprite);
        window.display();
    }

    keep_running = false;
}
