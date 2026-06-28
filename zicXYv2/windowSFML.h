#pragma once

#include <SFML/Graphics.hpp>

#ifndef KEY_DEF
#define KEY_DEF

#define KEY_F1 sf::Keyboard::F1
#define KEY_F2 sf::Keyboard::F2
#define KEY_F3 sf::Keyboard::F3
#define KEY_F4 sf::Keyboard::F4
#define KEY_F5 sf::Keyboard::F5

#define KEY_1 sf::Keyboard::Num1
#define KEY_2 sf::Keyboard::Num2
#define KEY_3 sf::Keyboard::Num3
#define KEY_4 sf::Keyboard::Num4
#define KEY_5 sf::Keyboard::Num5
#define KEY_6 sf::Keyboard::Num6
#define KEY_7 sf::Keyboard::Num7
#define KEY_8 sf::Keyboard::Num8

#endif

#include "zicXYv2/project.h"
#include "zicXYv2/ui.h"

void windowSFML(Draw& d, bool& needFullRedraw)
{
    sf::RenderWindow window(sf::VideoMode(SCREEN_W, SCREEN_H), "ZicXY");
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
            if (event.type == sf::Event::MouseMoved) {
                UiTrack::mouseMoved({ event.mouseMove.x, event.mouseMove.y }, winSize.x);
            } else if (event.type == sf::Event::Closed) {
                window.close();
            } else if (event.type == sf::Event::Resized) {
                window.setView(sf::View(sf::FloatRect(0, 0, (float)event.size.width, (float)event.size.height)));
                needFullRedraw = true;
            } else if (event.type == sf::Event::MouseButtonReleased) {
                UiTrack::mouseButtonReleased();
            } else if (event.type == sf::Event::KeyReleased) {
                TopBar::keyReleased(event.key.code, needFullRedraw);
                UiSeq::keyReleased(event.key.code, needFullRedraw);
                UiClips::keyReleased(event.key.code, needFullRedraw);
                if (event.key.code == sf::Keyboard::LShift && studio.currentView == ViewTrackShift) {
                    studio.currentView = ViewTrack;
                    needFullRedraw = true;
                }
                if (event.key.code >= sf::Keyboard::Num1 && event.key.code <= sf::Keyboard::Num6) {
                    int trkIdx = event.key.code - sf::Keyboard::Num1;
                    int note = (studio.selTrack == trkIdx && studio.selStep != -1) ? studio.tracks[trkIdx]->sequence[studio.selStep].note : 60;
                    std::lock_guard<std::mutex> lock(studio.audioMutex);
                    studio.tracks[trkIdx]->engine->noteOff(note);
                }
            } else if (event.type == sf::Event::KeyPressed) {
                UiTrack::keyPressed(event.key.code, needFullRedraw);
                UiSeq::keyPressed(event.key.code, needFullRedraw);
                UiClips::keyPressed(event.key.code, needFullRedraw);
                TopBar::keyPressed(event.key.code, needFullRedraw);
            } else if (event.type == sf::Event::MouseButtonPressed) {
                int mx = event.mouseButton.x, my = event.mouseButton.y;

                TopBar::mouseButtonPressed({ mx, my }, needFullRedraw);
                UiTrack::mouseButtonPressed({ mx, my });
                UiMenu::mouseButtonPressed({ mx, my });
                UiSeq::mouseButtonPressed({ mx, my }, (int)winSize.x, needFullRedraw);
                UiClips::mouseButtonPressed({ mx, my }, (int)winSize.x, needFullRedraw);
            } else if (event.type == sf::Event::MouseWheelScrolled) {
                int mx = event.mouseWheelScroll.x, my = event.mouseWheelScroll.y;
                float delta = event.mouseWheelScroll.delta;
                uint32_t now = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now().time_since_epoch()).count();
                bool shifted = sf::Keyboard::isKeyPressed(sf::Keyboard::LShift);

                if (!TopBar::mouseWheelScrolled({ mx, my }, delta, now, shifted)
                    && !MasterFx::mouseWheelScrolled({ mx, my }, delta, winSize.x, now, shifted)
                    && !UiTrack::mouseWheelScrolled({ mx, my }, delta, winSize.x, now, shifted, needFullRedraw)
                    && !UiMenu::mouseWheelScrolled(delta)
                    && !UiSeq::mouseWheelScrolled({ mx, my }, delta, winSize.x, now, shifted)
                    && !UiClips::mouseWheelScrolled({ mx, my }, (int)delta, winSize.x, now, shifted)) {
                }
            }
        }

        if (needFullRedraw) {
            winSize = window.getSize();
            d.setScreenSize({ (int)winSize.x, (int)winSize.y });
            // std::cout << "window size: " << winSize.x << "x" << winSize.y << std::endl;
        }
        if (drawUI(d, winSize.x, winSize.y, needFullRedraw)) {
            for (unsigned y = 0; y < winSize.y; y++)
                std::memcpy(&pixelBuffer[y * BUFFER_SIZE * 4], d.screenBuffer[y], winSize.x * 4);

            screenTexture.update(pixelBuffer.data());
        }
        window.clear();
        window.draw(screenSprite);
        window.display();
    }
}