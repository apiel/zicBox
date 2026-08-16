#pragma once

#include <SFML/Graphics.hpp>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <string>

#include "zicGridImpact/gridState.h"
#include "zicGridImpact/studio.h"
#include "zicGridImpact/ui.h"
#include "zicGridImpact/audioWorker.h"

#ifndef BUFFER_SIZE
#define BUFFER_SIZE 4096
#endif

inline bool mapKeyboardToPad(sf::Keyboard::Key key, int& col, int& row) {
    switch (key) {
        // Row 0: F1..F12
        case sf::Keyboard::F1:  col = 0;  row = 0; return true;
        case sf::Keyboard::F2:  col = 1;  row = 0; return true;
        case sf::Keyboard::F3:  col = 2;  row = 0; return true;
        case sf::Keyboard::F4:  col = 3;  row = 0; return true;
        case sf::Keyboard::F5:  col = 4;  row = 0; return true;
        case sf::Keyboard::F6:  col = 5;  row = 0; return true;
        case sf::Keyboard::F7:  col = 6;  row = 0; return true;
        case sf::Keyboard::F8:  col = 7;  row = 0; return true;
        case sf::Keyboard::F9:  col = 8;  row = 0; return true;
        case sf::Keyboard::F10: col = 9;  row = 0; return true;
        case sf::Keyboard::F11: col = 10; row = 0; return true;
        case sf::Keyboard::F12: col = 11; row = 0; return true;

        // Row 1: 1, 2, 3, 4, 5, 6, 7, 8, 9, 0, -, =
        case sf::Keyboard::Num1:   col = 0;  row = 1; return true;
        case sf::Keyboard::Num2:   col = 1;  row = 1; return true;
        case sf::Keyboard::Num3:   col = 2;  row = 1; return true;
        case sf::Keyboard::Num4:   col = 3;  row = 1; return true;
        case sf::Keyboard::Num5:   col = 4;  row = 1; return true;
        case sf::Keyboard::Num6:   col = 5;  row = 1; return true;
        case sf::Keyboard::Num7:   col = 6;  row = 1; return true;
        case sf::Keyboard::Num8:   col = 7;  row = 1; return true;
        case sf::Keyboard::Num9:   col = 8;  row = 1; return true;
        case sf::Keyboard::Num0:   col = 9;  row = 1; return true;
        case sf::Keyboard::Dash:   col = 10; row = 1; return true;
        case sf::Keyboard::Equal:  col = 11; row = 1; return true;

        // Row 2: Q, W, E, R, T, Y, U, I, O, P, [, ]
        case sf::Keyboard::Q:        col = 0;  row = 2; return true;
        case sf::Keyboard::W:        col = 1;  row = 2; return true;
        case sf::Keyboard::E:        col = 2;  row = 2; return true;
        case sf::Keyboard::R:        col = 3;  row = 2; return true;
        case sf::Keyboard::T:        col = 4;  row = 2; return true;
        case sf::Keyboard::Y:        col = 5;  row = 2; return true;
        case sf::Keyboard::U:        col = 6;  row = 2; return true;
        case sf::Keyboard::I:        col = 7;  row = 2; return true;
        case sf::Keyboard::O:        col = 8;  row = 2; return true;
        case sf::Keyboard::P:        col = 9;  row = 2; return true;
        case sf::Keyboard::LBracket: col = 10; row = 2; return true;
        case sf::Keyboard::RBracket: col = 11; row = 2; return true;

        // Row 3: A, S, D, F, G, H, J, K, L, ;, ', Enter
        case sf::Keyboard::A:         col = 0;  row = 3; return true;
        case sf::Keyboard::S:         col = 1;  row = 3; return true;
        case sf::Keyboard::D:         col = 2;  row = 3; return true;
        case sf::Keyboard::F:         col = 3;  row = 3; return true;
        case sf::Keyboard::G:         col = 4;  row = 3; return true;
        case sf::Keyboard::H:         col = 5;  row = 3; return true;
        case sf::Keyboard::J:         col = 6;  row = 3; return true;
        case sf::Keyboard::K:         col = 7;  row = 3; return true;
        case sf::Keyboard::L:         col = 8;  row = 3; return true;
        case sf::Keyboard::SemiColon: col = 9;  row = 3; return true;
        case sf::Keyboard::Quote:     col = 10; row = 3; return true;
        case sf::Keyboard::Return:    col = 11; row = 3; return true;

        default: return false;
    }
}

inline void runDesktopSFML(Draw& d, bool& needFullRedraw)
{
    // Check for headless screenshot mode
    const char* screenshotEnv = std::getenv("ZIC_SCREENSHOT");
    if (screenshotEnv != nullptr) {
        std::string basePath(screenshotEnv);
        sf::Image screenshot;
        std::vector<sf::Uint8> pixelBuf(SCREEN_W * SCREEN_H * 4, 255);

        for (int viewIdx = 0; viewIdx < VIEW_COUNT; ++viewIdx) {
            gridState.activeView = viewIdx;
            updateActiveViewEncoders();
            needFullRedraw = true;

            d.setScreenSize({ SCREEN_W, SCREEN_H });
            if (drawUI(d, SCREEN_W, SCREEN_H, needFullRedraw)) {
                for (unsigned y = 0; y < SCREEN_H; y++) {
                    std::memcpy(&pixelBuf[y * SCREEN_W * 4], d.screenBuffer[y], SCREEN_W * 4);
                }
                screenshot.create(SCREEN_W, SCREEN_H, pixelBuf.data());
                std::string filePath = basePath + "_" + std::to_string(viewIdx) + ".png";
                screenshot.saveToFile(filePath);
                std::cout << "[Screenshot] Generated: " << filePath << std::endl;
            }
        }
        return;
    }

    sf::RenderWindow window(sf::VideoMode(SCREEN_W, SCREEN_H), "Zic Grid Impact - 480x640 (Tekno Kick & Performance Matrix)");
    window.setFramerateLimit(60);
    window.setKeyRepeatEnabled(false);

    sf::Texture screenTexture;
    screenTexture.create(BUFFER_SIZE, BUFFER_SIZE);
    sf::Sprite screenSprite(screenTexture);
    std::vector<sf::Uint8> pixelBuffer(BUFFER_SIZE * BUFFER_SIZE * 4, 15);

    int hoveredEncoder = -1;

    while (window.isOpen() && keep_running) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                window.close();
                keep_running = false;
            } else if (event.type == sf::Event::Resized) {
                window.setView(sf::View(sf::FloatRect(0, 0, (float)event.size.width, (float)event.size.height)));
                needFullRedraw = true;
            } else if (event.type == sf::Event::MouseMoved) {
                int mx = event.mouseMove.x;
                int my = event.mouseMove.y;
                int margin = 4;
                int usableW = SCREEN_W - margin * 2;
                int encoderH = 150;

                if (mx >= margin && mx <= margin + usableW && my >= margin && my <= margin + encoderH) {
                    int colW = usableW / 4;
                    int rowH = encoderH / 3;
                    int c = (mx - margin) / colW;
                    int r = (my - margin) / rowH;
                    if (c >= 0 && c < 4 && r >= 0 && r < 3) {
                        hoveredEncoder = r * 4 + c;
                    }
                } else {
                    hoveredEncoder = -1;
                }
            } else if (event.type == sf::Event::MouseWheelScrolled) {
                if (hoveredEncoder >= 0 && hoveredEncoder < TOTAL_ENCODERS) {
                    int delta = (event.mouseWheelScroll.delta > 0) ? 1 : -1;
                    handleEncoderInput(hoveredEncoder, delta);
                    needFullRedraw = true;
                }
            } else if (event.type == sf::Event::MouseButtonPressed) {
                int mx = event.mouseButton.x;
                int my = event.mouseButton.y;
                int margin = 4;
                int usableW = SCREEN_W - margin * 2;
                int padGridH = 180;
                int padGridY = SCREEN_H - padGridH - margin;

                if (my >= padGridY && my <= padGridY + padGridH && mx >= margin && mx <= margin + usableW) {
                    int col = (mx - margin) / (usableW / PAD_COLS);
                    int row = (my - padGridY) / (padGridH / PAD_ROWS);
                    col = std::clamp(col, 0, PAD_COLS - 1);
                    row = std::clamp(row, 0, PAD_ROWS - 1);
                    handlePadPress(col, row, true);
                    needFullRedraw = true;
                }
            } else if (event.type == sf::Event::MouseButtonReleased) {
                int mx = event.mouseButton.x;
                int my = event.mouseButton.y;
                int margin = 4;
                int usableW = SCREEN_W - margin * 2;
                int padGridH = 180;
                int padGridY = SCREEN_H - padGridH - margin;

                if (my >= padGridY && my <= padGridY + padGridH && mx >= margin && mx <= margin + usableW) {
                    int col = (mx - margin) / (usableW / PAD_COLS);
                    int row = (my - padGridY) / (padGridH / PAD_ROWS);
                    col = std::clamp(col, 0, PAD_COLS - 1);
                    row = std::clamp(row, 0, PAD_ROWS - 1);
                    handlePadPress(col, row, false);
                    needFullRedraw = true;
                }
            } else if (event.type == sf::Event::KeyPressed) {
                int col = -1, row = -1;
                if (mapKeyboardToPad(event.key.code, col, row)) {
                    handlePadPress(col, row, true);
                    needFullRedraw = true;
                }
            } else if (event.type == sf::Event::KeyReleased) {
                int col = -1, row = -1;
                if (mapKeyboardToPad(event.key.code, col, row)) {
                    handlePadPress(col, row, false);
                    needFullRedraw = true;
                }
            }
        }

        d.setScreenSize({ SCREEN_W, SCREEN_H });
        if (drawUI(d, SCREEN_W, SCREEN_H, needFullRedraw)) {
            for (unsigned y = 0; y < SCREEN_H; y++) {
                std::memcpy(&pixelBuffer[y * BUFFER_SIZE * 4], d.screenBuffer[y], SCREEN_W * 4);
            }
            screenTexture.update(pixelBuffer.data());
        }

        window.clear(sf::Color(15, 15, 18));
        sf::Vector2u winSize = window.getSize();
        float scaleX = (float)winSize.x / SCREEN_W;
        float scaleY = (float)winSize.y / SCREEN_H;
        float scale = std::min(scaleX, scaleY);

        screenSprite.setScale(scale, scale);
        screenSprite.setPosition((winSize.x - SCREEN_W * scale) / 2.0f, (winSize.y - SCREEN_H * scale) / 2.0f);
        screenSprite.setTextureRect(sf::IntRect(0, 0, SCREEN_W, SCREEN_H));

        window.draw(screenSprite);
        window.display();
    }
}
