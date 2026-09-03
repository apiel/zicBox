#pragma once

#include <SFML/Graphics.hpp>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <mutex>
#include <string>

#include "audio/engines/DriftKick.h"
#include "draw/draw.h"
#include "sequenceBrain.h"
#include "uiPot.h"

static constexpr int BUFFER_SIZE = 2048;
extern std::atomic<bool> keep_running;
extern std::mutex audioMutex;

inline void runDesktopSFML(Draw& d, bool& needFullRedraw, UiPot& ui, SequenceBrain& brain, DriftKick& kick)
{
    sf::RenderWindow window(sf::VideoMode(960, 620), "zicPot - DriftKick Drum Engine & MIDI Master Clock");
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

                    // Check Encoder Button Click (x=445..555, y=105..145)
                    if (mx >= 445 && mx <= 555 && my >= 105 && my <= 145) {
                        ui.handleEncoderClick();
                        needFullRedraw = true;
                    }

                    // Check 64-Step Buttons Click (y=255..295) -> Toggle step or regenerate
                    if (my >= 255 && my <= 295) {
                        for (int i = 0; i < 64; ++i) {
                            float bx = 40.0f + i * 13.0f;
                            if (mx >= bx && mx <= (bx + 11.0f) && i < (int)brain.kickSequence.size()) {
                                brain.kickSequence[i].active = !brain.kickSequence[i].active;
                                needFullRedraw = true;
                                break;
                            }
                        }
                    }

                    // Check 10 Potentiometer Cards Drag Start
                    struct PotPos { PotIndex idx; float x; float y; };
                    PotPos pots[10] = {
                        { POT_SUB_FREQ, 40, 335 }, { POT_CLICK_AMT, 240, 335 }, { POT_DURATION, 440, 335 },
                        { POT_VCO_MORPH, 40, 425 }, { POT_FM_DEPTH, 240, 425 }, { POT_DRIVE, 440, 425 },
                        { POT_RUMBLE_AMT, 40, 515 }, { POT_RUMBLE_GAP, 240, 515 }, { POT_BPM, 440, 515 }, { POT_MASTER_VOL, 640, 515 }
                    };

                    for (int i = 0; i < 10; ++i) {
                        if (mx >= pots[i].x && mx <= (pots[i].x + 180) && my >= pots[i].y && my <= (pots[i].y + 75)) {
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
                int dir = (event.mouseWheelScroll.delta > 0) ? 1 : -1;
                ui.handleEncoderTurn(dir);
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
                    float curVal = ui.potValues[POT_MASTER_VOL];
                    ui.applyPotValue(POT_MASTER_VOL, curVal >= 1.0f ? 0.0f : curVal + 0.1f);
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
