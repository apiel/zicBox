#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstring>
#include <iomanip>
#include <iostream>
#include <memory>
#include <pthread.h>
#include <sstream>
#include <thread>
#include <vector>

#include "audio/Eq.h"
#include "helpers/enc.h"
#include "helpers/midiNote.h"
#include "zic23/audioWorker.h"
#include "zic23/generator.h"
#include "zic23/studio.h"
#include "zic23/uiClip.h"
#include "zic23/uiEq.h"
#include "zic23/uiMasterFx.h"
#include "zic23/uiMessage.h"
#include "zic23/uiPiano.h"
#include "zic23/uiSeq.h"
#include "zic23/uiTop.h"
#include "zic23/uiTrack.h"

void drawHelpOverlay(Draw& d, sf::Vector2u size)
{
    if (!showHelp) return;

    int W = (int)size.x, H = (int)size.y;
    int x = W < 1000 ? 50 : 200, y = 50, rW = W - x * 2, rH = H - y * 2;

    studio.overlayRect = { x, y, rW, rH };
    
    d.filledRect({ x, y }, { rW, rH }, { .color = { 20, 20, 25 } });
    d.rect({ x, y }, { rW, rH }, { .color = { 200, 200, 205 } });
    helpCloseRect = { x + rW - 50, y + 10, 40, 12 };
    d.filledRect({ helpCloseRect.left, helpCloseRect.top }, { 40, 12 }, { .color = { 200, 50, 50 } });
    d.text({ helpCloseRect.left + 6, helpCloseRect.top + 2 }, "Close", 8, { .color = { 255, 255, 255 }, .font = &PoppinsLight_8 });
    d.text({ x + 10, y + 10 }, "KEYBOARD SHORTCUTS", 16, { .color = { 0, 180, 255 }, .font = &PoppinsLight_16 });
    y += 40;
    auto dk = [&](std::string k, std::string v) {
        d.text({ x + 10, y }, k, 12, { .color = { 255, 255, 255 }, .font = &PoppinsLight_12 });
        d.text({ x + 160, y }, v, 12, { .color = { 180, 180, 190 }, .font = &PoppinsLight_12 });
        y += 24;
    };
    dk("SPACE", "Play / Stop");
    dk("H", "Help");
    y += 8;
    dk("1-6", "Trigger note");
    dk("M+1-6", "Mute / unmute");
    dk("G+1-6", "Generate");
    dk("E+1-6", "Open piano roll editor");
    dk("L+1-6 / S+1-6", "Load / Save Clip");
    dk("CTRL+R", "Reload all Clips");
    dk("SHIFT+1-6", "Select track");
    dk("F1-F12", "Scatter effect");
    dk("CTRL+O", "Open project");
    dk("CTRL+S", "Save project");
    dk("CTRL+SHIFT+S", "Save project as");
    y += 8;
    dk("D", "Duplicate sequence");
    dk("DELETE", "De-duplicate / delete page");
    dk("- / +", "Stretch / compress");
    y += 8;
    dk("CTRL+C/V", "Copy / paste step or track");
    dk("SCROLL", "Edit param / step");
    dk("N/V/P/L+SCROLL", "Edit Note / vel / prob / len");
    dk("MIDDLE CLICK", "Cycle step edit mode");
}

void drawStaticUI(Draw& d, sf::Vector2u size)
{
    d.clear();
    const int winW = (int)size.x;

    int currentY = 35;
    currentY = drawTracks(d, size, currentY);
    currentY += 10;
    currentY = drawMixerUI(d, size, currentY);
    currentY += 4;
    currentY = drawSeqUI(d, size, currentY);
    currentY += 4;
    drawEqUI(d, size, currentY);
    drawMasterFxUI(d, size, currentY);

    drawHelpOverlay(d, size);
    drawPianoRoll(d, size);
    drawTopBarUI(d, size);
    drawMessage(d, size);
    drawEngineDropdown(d);
}

void updateWaveforms(std::vector<sf::Uint8>& pixels, int stride)
{
    for (auto& p : studio.tracks) {
        std::lock_guard<std::mutex> lk(p->historyMtx);
        for (int x = 0; x < WAVE_HISTORY; x++) {
            int bH = (int)(std::min(p->history[x], 1.f) * (p->vuRect.height / 2));
            for (int y = 0; y < p->vuRect.height; y++) {
                size_t idx = ((p->vuRect.top + y) * stride + p->vuRect.left + x) * 4;
                bool w = std::abs(y - p->vuRect.height / 2) <= bH;
                pixels[idx] = w ? p->themeColor.r : 20;
                pixels[idx + 1] = w ? p->themeColor.g : 20;
                pixels[idx + 2] = w ? p->themeColor.b : 25;
            }
        }
    }
}

int main()
{
    snd_pcm_t* pcm_h = audioInit();
    pthread_setname_np(pthread_self(), "zicBox_UI");
    sf::RenderWindow window(sf::VideoMode(1080, 1120), "Zic23");
    window.setFramerateLimit(60);
    window.setKeyRepeatEnabled(false);

    Styles appStyles = {
        .screen = { 1080, 1120 }, .margin = 2, .colors = { { 15, 15, 18 }, { 255, 255, 255 }, { 120, 120, 130 }, { 0, 180, 255 }, { 10, 10, 12 }, { 28, 28, 32 }, { 35, 35, 40 } }
    };
    auto drawer = std::make_unique<Draw>(appStyles);
    sf::Texture screenTexture;
    screenTexture.create(BUFFER_SIZE, BUFFER_SIZE);
    sf::Sprite screenSprite(screenTexture);
    std::vector<sf::Uint8> pixelBuffer(BUFFER_SIZE * BUFFER_SIZE * 4, 15);

    bool static_needs_redraw = true;
    std::thread aThread(audioWorker, pcm_h);
    pthread_setname_np(aThread.native_handle(), "zicBox_Audio");

    while (window.isOpen() && keep_running) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) window.close();
            if (event.type == sf::Event::Resized) {
                window.setView(sf::View(sf::FloatRect(0, 0, (float)event.size.width, (float)event.size.height)));
                static_needs_redraw = true;
            }

            if (studio.pianoRollTrack != -1) {
                handelPianoEvent(window, event, static_needs_redraw);
                continue;
            }

            if (event.type == sf::Event::MouseButtonReleased) {
                eqDragging = false;
                eqDragBand = -1;
                filterDragging = false;
            }

            if (event.type == sf::Event::MouseMoved) {
                float mx = (float)event.mouseMove.x, my = (float)event.mouseMove.y;
                if (eqDragging) {
                    studio.tracks[eqActiveTrack]->eq.applyDrag(eqDragBand, mx, my, (float)eqCanvasRect.left, eqCanvasY_f, (float)eqCanvasRect.width, eqCanvasH_f, EQ_DB_RANGE, SAMPLE_RATE);
                    static_needs_redraw = true;
                }
                if (filterDragging) {
                    studio.filter.setCutoff(std::clamp(((mx - filterPadRect.left) / filterPadRect.width) * 2.0f - 1.0f, -1.0f, 1.0f));
                    studio.filter.setResonance(std::clamp(1.0f - ((my - filterPadRect.top) / filterPadRect.height), 0.0f, 1.0f));
                    static_needs_redraw = true;
                }
            }

            if (event.type == sf::Event::KeyReleased) {
                if (event.key.code >= sf::Keyboard::Num1 && event.key.code <= sf::Keyboard::Num6) {
                    int trkIdx = event.key.code - sf::Keyboard::Num1;
                    int note = (studio.selTrack == trkIdx && studio.selStep != -1) ? studio.tracks[trkIdx]->sequence[studio.selStep].note : 60;
                    std::lock_guard<std::mutex> lock(studio.audioMutex);
                    studio.tracks[trkIdx]->engine->noteOff(note);
                }
                if (event.key.code >= sf::Keyboard::F1 && event.key.code <= sf::Keyboard::F12) studio.activeScatterMode = 0;
            }
            if (event.type == sf::Event::KeyPressed) {
                handelSeqKeyPressed(window, event, static_needs_redraw);
                if (sf::Keyboard::isKeyPressed(sf::Keyboard::LControl) || sf::Keyboard::isKeyPressed(sf::Keyboard::RControl)) {
                    if (event.key.code == sf::Keyboard::S) {
                        if (sf::Keyboard::isKeyPressed(sf::Keyboard::LShift) || sf::Keyboard::isKeyPressed(sf::Keyboard::RShift)) {
                            saveAsProject();
                        } else {
                            saveProject();
                        }
                        showProjectMenu = false;
                        static_needs_redraw = true;
                    }
                    if (event.key.code == sf::Keyboard::O) {
                        loadProject();
                        showProjectMenu = false;
                        static_needs_redraw = true;
                    }
                    if (event.key.code == sf::Keyboard::R) {
                        reloadAllClips();
                        static_needs_redraw = true;
                    }
                }
                if (event.key.code >= sf::Keyboard::F1 && event.key.code <= sf::Keyboard::F12) studio.activeScatterMode = (event.key.code - sf::Keyboard::F1) + 1;
                if (event.key.code == sf::Keyboard::H || (showHelp && event.key.code == sf::Keyboard::Escape)) {
                    showHelp = !showHelp;
                    static_needs_redraw = true;
                }
                if (event.key.code == sf::Keyboard::Space) {
                    studio.isPlaying = !studio.isPlaying;
                    static_needs_redraw = true;
                }

                if (event.key.code >= sf::Keyboard::Num1 && event.key.code <= sf::Keyboard::Num6) {
                    int trkIdx = event.key.code - sf::Keyboard::Num1;
                    if (sf::Keyboard::isKeyPressed(sf::Keyboard::E)) {
                        studio.pianoRollTrack = trkIdx;
                        static_needs_redraw = true;
                    } else if (sf::Keyboard::isKeyPressed(sf::Keyboard::G)) {
                        runGeneration(trkIdx);
                        static_needs_redraw = true;
                    } else if (sf::Keyboard::isKeyPressed(sf::Keyboard::M)) {
                        studio.tracks[trkIdx]->isMuted = !studio.tracks[trkIdx]->isMuted;
                        static_needs_redraw = true;
                    } else if (sf::Keyboard::isKeyPressed(sf::Keyboard::S)) {
                        saveClip(trkIdx, studio.tracks[trkIdx]->activeClipIdx);
                    } else if (sf::Keyboard::isKeyPressed(sf::Keyboard::L)) {
                        loadClip(trkIdx, studio.tracks[trkIdx]->activeClipIdx);
                        static_needs_redraw = true;
                    } else if (sf::Keyboard::isKeyPressed(sf::Keyboard::LShift)) {
                        studio.selTrack = trkIdx;
                        studio.selStep = 0;
                        static_needs_redraw = true;
                    } else {
                        int note = (studio.selTrack == trkIdx && studio.selStep != -1) ? studio.tracks[trkIdx]->sequence[studio.selStep].note : 60;
                        std::lock_guard<std::mutex> lock(studio.audioMutex);
                        studio.tracks[trkIdx]->engine->noteOn(note, 1.0f);
                    }
                }
            }
            if (event.type == sf::Event::MouseMoved) {
                handelSeqMouseMoved(window, event, static_needs_redraw);
            }
            if (event.type == sf::Event::MouseButtonReleased) {
                handelSeqMouseReleased(window, event, static_needs_redraw);
            }
            if (event.type == sf::Event::MouseButtonPressed) {
                handelSeqMousePressed(window, event, static_needs_redraw);
                handelTracksMousePressed(window, event, static_needs_redraw);
                int mx = event.mouseButton.x, my = event.mouseButton.y;
                if (showHelp) {
                    if (helpCloseRect.contains(mx, my) || !studio.overlayRect.contains(mx, my)) {
                        showHelp = false;
                        static_needs_redraw = true;
                    }
                    continue;
                }

                if (menuBtnRect.contains(mx, my)) {
                    showProjectMenu = !showProjectMenu;
                    static_needs_redraw = true;
                } else if (showProjectMenu) {
                    if (menuSaveRect.contains(mx, my)) {
                        saveProject();
                        showProjectMenu = false;
                    } else if (menuSaveAsRect.contains(mx, my)) {
                        saveAsProject();
                        showProjectMenu = false;
                    } else if (menuOpenRect.contains(mx, my)) {
                        loadProject();
                        showProjectMenu = false;
                    } else {
                        showProjectMenu = false; // Close if clicked elsewhere
                    }
                    static_needs_redraw = true;
                }

                // Clip interactions
                for (int t = 0; t < MAX_TRACKS; t++) {
                    for (int i = 0; i < 32; i++) {
                        if (clipRects[t][i].contains(mx, my)) {
                            studio.tracks[t]->selectedClipIdx = i;
                            static_needs_redraw = true;
                        }
                    }
                    if (saveBtnRects[t].contains(mx, my)) {
                        saveClip(t, studio.tracks[t]->selectedClipIdx);
                        showMessage("Clip saved");
                        static_needs_redraw = true;
                    }
                    if (loadBtnRects[t].contains(mx, my)) {
                        loadClip(t, studio.tracks[t]->selectedClipIdx);
                        showMessage("Clip loaded");
                        static_needs_redraw = true;
                    }
                }
                if (reloadAllBtnRect.contains(mx, my)) {
                    reloadAllClips();
                    static_needs_redraw = true;
                }

                if (filterPadRect.contains(mx, my)) {
                    filterDragging = true;
                    studio.filter.setCutoff(std::clamp(((float)(mx - filterPadRect.left) / filterPadRect.width) * 2.0f - 1.0f, -1.0f, 1.0f));
                    studio.filter.setResonance(std::clamp(1.0f - ((float)(my - filterPadRect.top) / filterPadRect.height), 0.0f, 1.0f));
                    static_needs_redraw = true;
                }
                for (int t = 0; t < MAX_TRACKS; t++)
                    if (eqTrackBtnRects[t].contains(mx, my)) {
                        eqActiveTrack = t;
                        static_needs_redraw = true;
                    }
                if (eqCanvasRect.contains(mx, my)) {
                    auto& eq = studio.tracks[eqActiveTrack]->eq;
                    for (int b = 0; b < EQ::NUM_BANDS; b++) {
                        auto [px, py] = eq.dotPos(b, (float)eqCanvasRect.left, eqCanvasY_f, (float)eqCanvasRect.width, eqCanvasH_f, EQ_DB_RANGE);
                        if ((mx - (int)px) * (mx - (int)px) + (my - (int)py) * (my - (int)py) <= (EQ_DOT_R + 4) * (EQ_DOT_R + 4)) {
                            eqDragging = true;
                            eqDragBand = b;
                            break;
                        }
                    }
                }

                if (studio.transportRect.contains(mx, my)) {
                    studio.isPlaying = !studio.isPlaying;
                    static_needs_redraw = true;
                }
                if (helpBtnRect.contains(mx, my)) {
                    showHelp = true;
                    static_needs_redraw = true;
                }
            }
            if (event.type == sf::Event::MouseWheelScrolled) {
                if (showHelp) continue;
                if (handelSeqMouseWheelScrolled(window, event, static_needs_redraw)) continue;
                handelTracksMouseWheelScrolled(window, event, static_needs_redraw);

                int mx = event.mouseWheelScroll.x, my = event.mouseWheelScroll.y;
                float delta = event.mouseWheelScroll.delta;
                uint32_t now = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now().time_since_epoch()).count();

                if (studio.bpmRect.contains(mx, my)) {
                    int scaled = encGetScaledDirection(delta, now, lastBpmTick);
                    lastBpmTick = now;
                    studio.bpm = std::clamp(studio.bpm + (scaled * (sf::Keyboard::isKeyPressed(sf::Keyboard::LShift) ? 5.0f : 0.5f)), 20.0f, 300.0f);
                    studio.updateClock();
                    static_needs_redraw = true;
                } else {
                    for (int i = 0; i < 4; i++) {
                        if (compRects[i].contains(mx, my)) {
                            float step = (sf::Keyboard::isKeyPressed(sf::Keyboard::LShift)) ? 5.0f : 1.0f;
                            if (i == 0) studio.compressor.threshold = std::clamp(studio.compressor.threshold + (delta > 0 ? step : -step), -60.0f, 0.0f);
                            else if (i == 1) studio.compressor.ratio = std::clamp(studio.compressor.ratio + (delta > 0 ? 0.5f : -0.5f), 1.0f, 20.0f);
                            else if (i == 2) studio.compressor.attack = std::clamp(studio.compressor.attack + (delta > 0 ? 0.001f : -0.001f), 0.0001f, 0.1f);
                            else if (i == 3) studio.compressor.release = std::clamp(studio.compressor.release + (delta > 0 ? 0.01f : -0.01f), 0.01f, 1.0f);
                            static_needs_redraw = true;
                        }
                    }
                }
            }
        }

        if (static_needs_redraw) {
            sf::Vector2u winSize = window.getSize();
            drawer->setScreenSize({ (int)winSize.x, (int)winSize.y });
            drawStaticUI(*drawer, winSize);
            for (unsigned y = 0; y < winSize.y; y++)
                std::memcpy(&pixelBuffer[y * BUFFER_SIZE * 4], drawer->screenBuffer[y], winSize.x * 4);
            static_needs_redraw = false;
        }

        if (statusMsg.active) {
            if (!shouldDrawMessage()) {
                statusMsg.active = false;
                static_needs_redraw = true;
            }
        }

        if (studio.pianoRollTrack != -1) {
            updatePianoRollPixels(pixelBuffer, BUFFER_SIZE);
        } else if (!showHelp) {
            if (!showProjectMenu) {
                updateWaveforms(pixelBuffer, BUFFER_SIZE);
                updateSpectrumPixels(pixelBuffer, BUFFER_SIZE);
            }
            updateSequencerPixels(pixelBuffer, BUFFER_SIZE);
            updateCompressorMeter(pixelBuffer, BUFFER_SIZE);
        }

        screenTexture.update(pixelBuffer.data());
        window.clear();
        window.draw(screenSprite);
        window.display();
    }

    keep_running = false;
    aThread.join();
    snd_pcm_close(pcm_h);
    return 0;
}