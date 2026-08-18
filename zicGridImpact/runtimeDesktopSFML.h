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
        // Row 0: 1, 2, 3, 4, 5, 6, 7, 8, 9, 0, -, =
        case sf::Keyboard::Num1:   col = 0;  row = 0; return true;
        case sf::Keyboard::Num2:   col = 1;  row = 0; return true;
        case sf::Keyboard::Num3:   col = 2;  row = 0; return true;
        case sf::Keyboard::Num4:   col = 3;  row = 0; return true;
        case sf::Keyboard::Num5:   col = 4;  row = 0; return true;
        case sf::Keyboard::Num6:   col = 5;  row = 0; return true;
        case sf::Keyboard::Num7:   col = 6;  row = 0; return true;
        case sf::Keyboard::Num8:   col = 7;  row = 0; return true;
        case sf::Keyboard::Num9:   col = 8;  row = 0; return true;
        case sf::Keyboard::Num0:   col = 9;  row = 0; return true;
        case sf::Keyboard::Dash:   col = 10; row = 0; return true;
        case sf::Keyboard::Equal:  col = 11; row = 0; return true;

        // Row 1: Q, W, E, R, T, Y, U, I, O, P, [, ]
        case sf::Keyboard::Q:        col = 0;  row = 1; return true;
        case sf::Keyboard::W:        col = 1;  row = 1; return true;
        case sf::Keyboard::E:        col = 2;  row = 1; return true;
        case sf::Keyboard::R:        col = 3;  row = 1; return true;
        case sf::Keyboard::T:        col = 4;  row = 1; return true;
        case sf::Keyboard::Y:        col = 5;  row = 1; return true;
        case sf::Keyboard::U:        col = 6;  row = 1; return true;
        case sf::Keyboard::I:        col = 7;  row = 1; return true;
        case sf::Keyboard::O:        col = 8;  row = 1; return true;
        case sf::Keyboard::P:        col = 9;  row = 1; return true;
        case sf::Keyboard::LBracket: col = 10; row = 1; return true;
        case sf::Keyboard::RBracket: col = 11; row = 1; return true;

        // Row 2: A, S, D, F, G, H, J, K, L, ;, ', Enter
        case sf::Keyboard::A:         col = 0;  row = 2; return true;
        case sf::Keyboard::S:         col = 1;  row = 2; return true;
        case sf::Keyboard::D:         col = 2;  row = 2; return true;
        case sf::Keyboard::F:         col = 3;  row = 2; return true;
        case sf::Keyboard::G:         col = 4;  row = 2; return true;
        case sf::Keyboard::H:         col = 5;  row = 2; return true;
        case sf::Keyboard::J:         col = 6;  row = 2; return true;
        case sf::Keyboard::K:         col = 7;  row = 2; return true;
        case sf::Keyboard::L:         col = 8;  row = 2; return true;
        case sf::Keyboard::SemiColon: col = 9;  row = 2; return true;
        case sf::Keyboard::Quote:     col = 10; row = 2; return true;
        case sf::Keyboard::Return:    col = 11; row = 2; return true;

        // Row 3: Z, X, C, V, B, N, M, Comma, ., /
        case sf::Keyboard::Z:         col = 0;  row = 3; return true;
        case sf::Keyboard::X:         col = 1;  row = 3; return true;
        case sf::Keyboard::C:         col = 2;  row = 3; return true;
        case sf::Keyboard::V:         col = 3;  row = 3; return true;
        case sf::Keyboard::B:         col = 4;  row = 3; return true;
        case sf::Keyboard::N:         col = 5;  row = 3; return true;
        case sf::Keyboard::M:         col = 6;  row = 3; return true;
        case sf::Keyboard::Comma:     col = 7;  row = 3; return true;
        case sf::Keyboard::Period:    col = 8;  row = 3; return true;
        case sf::Keyboard::Slash:     col = 9;  row = 3; return true;
        case sf::Keyboard::RShift:
        case sf::Keyboard::LShift:    col = 10; row = 3; return true;
        case sf::Keyboard::Space:     col = 11; row = 3; return true;

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
    int mousePressedCol = -1;
    int mousePressedRow = -1;

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
                sf::Vector2u winSize = window.getSize();
                float scaleX = (float)winSize.x / SCREEN_W;
                float scaleY = (float)winSize.y / SCREEN_H;
                float scale = std::min(scaleX, scaleY);
                float offsetX = (winSize.x - SCREEN_W * scale) / 2.0f;
                float offsetY = (winSize.y - SCREEN_H * scale) / 2.0f;

                int mx = (int)((event.mouseMove.x - offsetX) / scale);
                int my = (int)((event.mouseMove.y - offsetY) / scale);
                int margin = 4;
                int usableW = SCREEN_W - margin * 2;
                int encoderH = 3 * UiParams::ROW_H; // 108px matching ui.h

                if (mx >= margin && mx <= margin + usableW && my >= margin && my <= margin + encoderH) {
                    int colW = usableW / ENCODER_COLS;
                    int rowH = encoderH / ENCODER_ROWS;
                    int c = (mx - margin) / std::max(1, colW);
                    int r = (my - margin) / std::max(1, rowH);
                    if (c >= 0 && c < ENCODER_COLS && r >= 0 && r < ENCODER_ROWS) {
                        hoveredEncoder = r * ENCODER_COLS + c;
                    }
                } else {
                    hoveredEncoder = -1;
                }
            } else if (event.type == sf::Event::MouseWheelScrolled) {
                sf::Vector2u winSize = window.getSize();
                float scaleX = (float)winSize.x / SCREEN_W;
                float scaleY = (float)winSize.y / SCREEN_H;
                float scale = std::min(scaleX, scaleY);
                float offsetX = (winSize.x - SCREEN_W * scale) / 2.0f;
                float offsetY = (winSize.y - SCREEN_H * scale) / 2.0f;

                int mx = (int)((event.mouseWheelScroll.x - offsetX) / scale);
                int my = (int)((event.mouseWheelScroll.y - offsetY) / scale);
                int margin = 4;
                int usableW = SCREEN_W - margin * 2;
                int encoderH = 3 * UiParams::ROW_H; // 108px

                int targetEnc = hoveredEncoder;
                if (mx >= margin && mx <= margin + usableW && my >= margin && my <= margin + encoderH) {
                    int colW = usableW / ENCODER_COLS;
                    int rowH = encoderH / ENCODER_ROWS;
                    int c = (mx - margin) / std::max(1, colW);
                    int r = (my - margin) / std::max(1, rowH);
                    if (c >= 0 && c < ENCODER_COLS && r >= 0 && r < ENCODER_ROWS) {
                        targetEnc = r * ENCODER_COLS + c;
                    }
                }

                if (targetEnc >= 0 && targetEnc < TOTAL_ENCODERS) {
                    int delta = (event.mouseWheelScroll.delta > 0) ? 1 : -1;
                    handleEncoderInput(targetEnc, delta);
                    needFullRedraw = true;
                }
            } else if (event.type == sf::Event::MouseButtonPressed) {
                sf::Vector2u winSize = window.getSize();
                float scaleX = (float)winSize.x / SCREEN_W;
                float scaleY = (float)winSize.y / SCREEN_H;
                float scale = std::min(scaleX, scaleY);
                float offsetX = (winSize.x - SCREEN_W * scale) / 2.0f;
                float offsetY = (winSize.y - SCREEN_H * scale) / 2.0f;

                int mx = (int)((event.mouseButton.x - offsetX) / scale);
                int my = (int)((event.mouseButton.y - offsetY) / scale);

                int margin = 4;
                int usableW = SCREEN_W - margin * 2;
                int encoderH = 3 * UiParams::ROW_H; // 108px
                int padGridH = 145; // MATCH ui.h (145px)
                int padGridY = SCREEN_H - padGridH - margin;

                if (my >= padGridY && my <= padGridY + padGridH && mx >= margin && mx <= margin + usableW) {
                    int padW = usableW / PAD_COLS;
                    int padH = padGridH / PAD_ROWS;
                    int col = std::clamp((mx - margin) / std::max(1, padW), 0, PAD_COLS - 1);
                    int row = std::clamp((my - padGridY) / std::max(1, padH), 0, PAD_ROWS - 1);
                    mousePressedCol = col;
                    mousePressedRow = row;
                    handlePadPress(col, row, true);
                    needFullRedraw = true;
                } else if (my >= margin && my <= margin + encoderH && mx >= margin && mx <= margin + usableW) {
                    int colW = usableW / ENCODER_COLS;
                    int rowH = encoderH / ENCODER_ROWS;
                    int c = std::clamp((mx - margin) / std::max(1, colW), 0, ENCODER_COLS - 1);
                    int r = std::clamp((my - margin) / std::max(1, rowH), 0, ENCODER_ROWS - 1);
                    int encId = r * ENCODER_COLS + c;
                    int delta = (event.mouseButton.button == sf::Mouse::Right) ? -1 : 1;
                    handleEncoderInput(encId, delta);
                    needFullRedraw = true;
                }
            } else if (event.type == sf::Event::MouseButtonReleased) {
                if (mousePressedCol >= 0 && mousePressedRow >= 0) {
                    int c = mousePressedCol;
                    int r = mousePressedRow;
                    mousePressedCol = -1;
                    mousePressedRow = -1;
                    handlePadPress(c, r, false);
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
