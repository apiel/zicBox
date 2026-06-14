#pragma once

#include <SFML/Graphics.hpp>

#include "zicXYv2/ui.h"
#include "zicXYv2/project.h"

void windowSFML(Draw& d, bool& needFullRedraw)
{
    sf::RenderWindow window(sf::VideoMode(SCREEN_W, 480), "ZicRack");
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
                MasterFx::mouseMoved({ event.mouseMove.x, event.mouseMove.y });
                UiTrack::mouseMoved({ event.mouseMove.x, event.mouseMove.y }, winSize.x);
            } else if (event.type == sf::Event::Closed) {
                window.close();
            } else if (event.type == sf::Event::Resized) {
                window.setView(sf::View(sf::FloatRect(0, 0, (float)event.size.width, (float)event.size.height)));
                needFullRedraw = true;
            } else if (event.type == sf::Event::MouseButtonReleased) {
                MasterFx::mouseButtonReleased();
                UiTrack::mouseButtonReleased();
            } else if (event.type == sf::Event::KeyReleased) {
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
                if (event.key.code == sf::Keyboard::LShift && studio.currentView == ViewTrack) {
                    studio.currentView = ViewTrackShift;
                    needFullRedraw = true;
                }
                if (event.key.code == sf::Keyboard::Space) {
                    studio.isPlaying = !studio.isPlaying;
                    TopBar::needsRedraw = true;
                }

                if (event.key.code >= sf::Keyboard::Num1 && event.key.code <= sf::Keyboard::Num6) {
                    int trkIdx = event.key.code - sf::Keyboard::Num1;
                    if (sf::Keyboard::isKeyPressed(sf::Keyboard::M)) {
                        studio.tracks[trkIdx]->isMuted = !studio.tracks[trkIdx]->isMuted;
                        // needRedraw = true;
                    } else if (sf::Keyboard::isKeyPressed(sf::Keyboard::LShift)) {
                        studio.selTrack = trkIdx;
                        studio.selStep = 0;
                        studio.currentView = ViewTrack;
                        needFullRedraw = true;
                        // needRedraw = true;
                    } else {
                        int note = (studio.selTrack == trkIdx && studio.selStep != -1) ? studio.tracks[trkIdx]->sequence[studio.selStep].note : 60;
                        std::lock_guard<std::mutex> lock(studio.audioMutex);
                        studio.tracks[trkIdx]->engine->noteOn(note, 1.0f);
                        // if (studio.selTrack != trkIdx || studio.currentView != ViewTrack) {
                        //     studio.selTrack = trkIdx;
                        //     studio.currentView = ViewTrack;
                        //     needFullRedraw = true;
                        // }
                    }
                }

                if (event.key.code == sf::Keyboard::S && sf::Keyboard::isKeyPressed(sf::Keyboard::LControl)) {
                    if (saveProject()) {
                    UiMessage::show("Saved", needFullRedraw);
                    } else if (studio.currentView == ViewMenu) {
                        UiMenu::currentView = UiMenu::VIEW_KEYBOARD;
                    } else {
                        studio.currentView == ViewMenu;
                        UiMenu::currentView = UiMenu::VIEW_PROJECTS;
                    }
                    needFullRedraw = true;
                }

                if (event.key.code == sf::Keyboard::R && sf::Keyboard::isKeyPressed(sf::Keyboard::LControl)) {
                    Track& trk = *studio.tracks[studio.selTrack];
                    loadClip(studio.selTrack, trk.activeClipIdx);
                    UiMessage::show("Loaded", needFullRedraw);
                    needFullRedraw = true;
                }
            } else if (event.type == sf::Event::MouseButtonPressed) {
                int mx = event.mouseButton.x, my = event.mouseButton.y;

                TopBar::mouseButtonPressed({ mx, my }, needFullRedraw);
                MasterFx::mouseButtonPressed({ mx, my });
                UiTrack::mouseButtonPressed({ mx, my });
                UiMenu::mouseButtonPressed({ mx, my });
            } else if (event.type == sf::Event::MouseWheelScrolled) {
                int mx = event.mouseWheelScroll.x, my = event.mouseWheelScroll.y;
                float delta = event.mouseWheelScroll.delta;
                uint32_t now = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now().time_since_epoch()).count();
                bool shifted = sf::Keyboard::isKeyPressed(sf::Keyboard::LShift);

                if (!TopBar::mouseWheelScrolled({ mx, my }, delta, now, shifted)
                    && !MasterFx::mouseWheelScrolled({ mx, my }, delta, winSize.x, now, shifted)
                    && !UiTrack::mouseWheelScrolled({ mx, my }, delta, winSize.x, now, shifted)
                    && !UiMenu::mouseWheelScrolled(delta
                    && !UiTrackShift::mouseWheelScrolled({ mx, my }, delta))) {
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