#pragma once

#include <SFML/Graphics.hpp>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <mutex>
#include <string>

#include "audio/engines/PotKick.h"
#include "draw/draw.h"
#include "sequenceBrain.h"
#if __has_include("emu/uiPot.h")
#include "emu/uiPot.h"
#else
#include "uiPot.h"
#endif

static constexpr int BUFFER_SIZE = 2048;
extern std::atomic<bool> keep_running;
extern std::mutex audioMutex;

inline void runDesktopSFML(Draw& d, bool& needFullRedraw, UiPot& ui, SequenceBrain& brain, PotKick& kick)
{
    sf::RenderWindow window(sf::VideoMode(600, 760), "zicPot - PotKick Drum Engine & MIDI Master Clock");
    window.setFramerateLimit(60);
    window.setKeyRepeatEnabled(false);

    sf::Texture screenTexture;
    screenTexture.create(BUFFER_SIZE, BUFFER_SIZE);
    sf::Sprite screenSprite(screenTexture);
    std::vector<sf::Uint8> pixelBuffer(BUFFER_SIZE * BUFFER_SIZE * 4, 15);

    sf::Vector2u winSize = window.getSize();

    // Check for screenshot environment variable
    const char* screenshotEnv = std::getenv("ZIC_SCREENSHOT");
    if (screenshotEnv != nullptr) {
        if (!window.isOpen()) {
            std::cout << "Error: Cannot take screenshot, window is not open\n";
            return;
        }
        std::string basePath(screenshotEnv);
        sf::Texture captureTexture;
        captureTexture.create(winSize.x, winSize.y);

        winSize = window.getSize();
        d.setScreenSize({ (int)winSize.x, (int)winSize.y });

        if (ui.draw(d, winSize.x, winSize.y, needFullRedraw)) {
            for (unsigned int y = 0; y < winSize.y; ++y) {
                std::memcpy(&pixelBuffer[y * BUFFER_SIZE * 4], d.screenBuffer[y], winSize.x * 4);
            }
            screenTexture.update(pixelBuffer.data());
        }

        window.clear();
        window.draw(screenSprite);
        window.display();

        captureTexture.update(window);
        sf::Image screenshot = captureTexture.copyToImage();
        screenshot.saveToFile(basePath + "_zicPot.png");

        window.close();
        return;
    }

    bool isMouseDraggingPot = false;
    int draggingPotIdx = -1;
    float dragStartY = 0.0f;
    float dragStartVal = 0.0f;

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
                    float mx = (float)event.mouseButton.x;
                    float my = (float)event.mouseButton.y;

                    std::lock_guard<std::mutex> lock(audioMutex);

                    // Check Encoder Button Click inside Cell (Row 0, Col 1) (x=235..365, y=90..140)
                    if (mx >= 235 && mx <= 365 && my >= 90 && my <= 140) {
                        ui.handleEncoderClick();
                        needFullRedraw = true;
                    }

                    // Check 64-Step Buttons Click (y=710..745) -> Toggle step
                    if (my >= 710 && my <= 745) {
                        for (int i = 0; i < 64; ++i) {
                            float bx = 40.0f + i * 8.1f;
                            if (mx >= bx && mx <= (bx + 6.0f) && i < (int)brain.kickSequence.size()) {
                                brain.kickSequence[i].active = !brain.kickSequence[i].active;
                                needFullRedraw = true;
                                break;
                            }
                        }
                    }

                    // Check 10 Potentiometer Cards Drag Start inside 4x3 grid cells
                    struct PotPos { PotIndex idx; float x; float y; float w; float h; };
                    PotPos pots[10] = {
                        { POT_DRIVE,       400, 30,  160, 150 }, // Row 0, Col 2: A1
                        { POT_DURATION,    40,  200, 160, 150 }, { POT_VCO_MORPH,  220, 200, 160, 150 }, { POT_WAVEFOLD,    400, 200, 160, 150 }, // Row 1: A10 | A6 | A0
                        { POT_SWEEP_DEPTH, 40,  370, 160, 150 }, { POT_FM_DEPTH,   220, 370, 160, 150 }, { POT_PUNCH,       400, 370, 160, 150 }, // Row 2: A11 | A5 | A2
                        { POT_SWEEP_SHP,   40,  540, 160, 150 }, { POT_FM_SNAP,    220, 540, 160, 150 }, { POT_CRUSH,       400, 540, 160, 150 }  // Row 3: A8  | A4 | A3
                    };

                    for (int i = 0; i < 10; ++i) {
                        if (mx >= pots[i].x && mx <= (pots[i].x + pots[i].w) && my >= pots[i].y && my <= (pots[i].y + pots[i].h)) {
                            isMouseDraggingPot = true;
                            draggingPotIdx = (int)pots[i].idx;
                            dragStartY = my;
                            dragStartVal = ui.potValues[pots[i].idx];
                            break;
                        }
                    }
                }
            } else if (event.type == sf::Event::MouseButtonReleased) {
                if (event.mouseButton.button == sf::Mouse::Left) {
                    isMouseDraggingPot = false;
                    draggingPotIdx = -1;
                }
            } else if (event.type == sf::Event::MouseMoved) {
                if (isMouseDraggingPot && draggingPotIdx >= 0) {
                    float dy = dragStartY - (float)event.mouseMove.y;
                    float newVal = dragStartVal + (dy / 150.0f);
                    std::lock_guard<std::mutex> lock(audioMutex);
                    ui.applyPotValue((PotIndex)draggingPotIdx, newVal);
                    needFullRedraw = true;
                }
            } else if (event.type == sf::Event::MouseWheelScrolled) {
                std::lock_guard<std::mutex> lock(audioMutex);
                float mx = (float)event.mouseWheelScroll.x;
                float my = (float)event.mouseWheelScroll.y;
                float delta = event.mouseWheelScroll.delta;

                struct PotPos { PotIndex idx; float x; float y; float w; float h; };
                PotPos pots[10] = {
                    { POT_DRIVE,       400, 30,  160, 150 }, // Row 0, Col 2: A1
                    { POT_DURATION,    40,  200, 160, 150 }, { POT_VCO_MORPH,  220, 200, 160, 150 }, { POT_WAVEFOLD,    400, 200, 160, 150 }, // Row 1: A10 | A6 | A0
                    { POT_SWEEP_DEPTH, 40,  370, 160, 150 }, { POT_FM_DEPTH,   220, 370, 160, 150 }, { POT_PUNCH,       400, 370, 160, 150 }, // Row 2: A11 | A5 | A2
                    { POT_SWEEP_SHP,   40,  540, 160, 150 }, { POT_FM_SNAP,    220, 540, 160, 150 }, { POT_CRUSH,       400, 540, 160, 150 }  // Row 3: A8  | A4 | A3
                };

                bool scrolledOverPot = false;
                for (int i = 0; i < 10; ++i) {
                    if (mx >= pots[i].x && mx <= (pots[i].x + pots[i].w) && my >= pots[i].y && my <= (pots[i].y + pots[i].h)) {
                        float step = (delta > 0) ? 0.05f : -0.05f;
                        float curVal = ui.app.potValues[pots[i].idx];
                        ui.applyPotValue(pots[i].idx, curVal + step);
                        scrolledOverPot = true;
                        break;
                    }
                }

                if (!scrolledOverPot) {
                    int dir = (delta > 0) ? 1 : -1;
                    ui.handleEncoderTurn(dir);
                }
                needFullRedraw = true;
            } else if (event.type == sf::Event::KeyPressed) {
                std::lock_guard<std::mutex> lock(audioMutex);
                if (event.key.code == sf::Keyboard::Space || event.key.code == sf::Keyboard::Return) {
                    ui.handleEncoderClick();
                    needFullRedraw = true;
                } else if (event.key.code == sf::Keyboard::Up || event.key.code == sf::Keyboard::Right) {
                    ui.handleEncoderTurn(1);
                    needFullRedraw = true;
                } else if (event.key.code == sf::Keyboard::Down || event.key.code == sf::Keyboard::Left) {
                    ui.handleEncoderTurn(-1);
                    needFullRedraw = true;
                } else if (event.key.code >= sf::Keyboard::Num1 && event.key.code <= sf::Keyboard::Num9) {
                    int potIdx = event.key.code - sf::Keyboard::Num1;
                    float curVal = ui.potValues[potIdx];
                    ui.applyPotValue((PotIndex)potIdx, curVal >= 1.0f ? 0.0f : curVal + 0.1f);
                    needFullRedraw = true;
                } else if (event.key.code == sf::Keyboard::Num0) {
                    float curVal = ui.potValues[POT_CRUSH];
                    ui.applyPotValue(POT_CRUSH, curVal >= 1.0f ? 0.0f : curVal + 0.1f);
                    needFullRedraw = true;
                }
            }
        }

        // Pot overlay timer countdown logic
        if (ui.potOverlayTimer > 0) {
            ui.potOverlayTimer -= 16;
            if (ui.potOverlayTimer < 0) ui.potOverlayTimer = 0;
            needFullRedraw = true;
        }

        if (needFullRedraw) {
            winSize = window.getSize();
            d.setScreenSize({ (int)winSize.x, (int)winSize.y });
        }

        // Draw and update SFML texture buffer from Draw interface
        if (ui.draw(d, winSize.x, winSize.y, needFullRedraw)) {
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
