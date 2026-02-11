#pragma once

// #define REVERB_BUFFER_SIZE 2048
#define REVERB_BUFFER_SIZE 4096
// #define REVERB_BUFFER_SIZE 8192
// #define REVERB_BUFFER_SIZE 16384
// #define REVERB_BUFFER_SIZE 32768

#include "audio/Clock.h"
#include "audio/Sequencer.h"
#include "draw/drawPrimitives.h"
#include "helpers/clamp.h"
#include "stm32/platform.h"
#include "StepEditView.h"
#include "MainListView.h"
#include "engines.h"
#include <array>
#include <cstdint>
#include <cstdio>
#include <functional>

#ifdef IS_STM32
#include "main.h"
#endif


// --- 5. CORE ---
class Core {
    DrawPrimitives& display;
    
    EngineType currentEngineType = KICK;

    Clock clock;
    Sequencer sequencer;
    float volume = 0.8f;
    float bpm = 140.0f;
    bool isMuted = false;
    int stepCountIdx = 2;
    const int stepCountValues[6] = { 4, 8, 16, 32, 64, 128 };
    uint32_t currentPlayheadStep = 0;
    bool needsRedraw = true;
    
    MainListView mainView;
    StepEditView stepEditView;
    IView* currentView;
    
    bool isTriggered = false;
    uint32_t triggerVisualCounter = 0;
    uint32_t lastBtnTime = 0;

public:
    Core(DrawPrimitives& display)
        : display(display)
        , clock(SAMPLE_RATE)
        , mainView(engines, currentEngineType, clock, volume, bpm, isMuted, needsRedraw, [this]() {
            stepEditView.reset();
            currentView = &stepEditView;
            needsRedraw = true;
        })
        , stepEditView(sequencer, needsRedraw, currentPlayheadStep, stepCountIdx, stepCountValues, [this]() {
            currentView = &mainView;
            needsRedraw = true;
        })
    {
        // Link the interface pointers
        engines[KICK] = &kick;
        engines[CLAP] = &clap;
        engines[SNARE] = &snare;

        currentView = &mainView;
        clock.setBpm(bpm);
        sequencer.setStepCount(stepCountValues[stepCountIdx]);
        
        // Initialize all parameters for all engines
        kick.init();
        snare.init();
        clap.init();

        for (int i = 0; i < 128; i++)
            if (i % 4 == 0) sequencer.getStep(i).enabled = true;
    }

    void encBtn() {
#ifdef IS_STM32
        uint32_t currentTime = HAL_GetTick();
        if (currentTime - lastBtnTime < 200) return;
        lastBtnTime = currentTime;
#endif
        currentView->onButton();
    }

    void onEncoder(int dir) { currentView->onEncoder(dir); }

    float sample() {
        uint32_t tick = clock.getClock();
        if (tick > 0 && tick % 6 == 0) {
            sequencer.onStep([&](const Sequencer::Event& ev) {
                if (ev.type == Sequencer::EventType::NoteOn) {
                    currentPlayheadStep = (tick / 6) % sequencer.getStepCount();
                    // Unified Note Trigger
                    engines[currentEngineType]->noteOn(ev.stepRef->notes[0], ev.stepRef->velocity * volume);
                    isTriggered = true;
                    triggerVisualCounter = 2000;
                    needsRedraw = true;
                }
            });
        }
        
        if (triggerVisualCounter > 0 && --triggerVisualCounter == 0) {
            isTriggered = false;
            needsRedraw = true;
        }

        // Single Polymorphic Call
        return isMuted ? 0.0f : engines[currentEngineType]->sample();
    }

    bool render() {
        if (!needsRedraw) return false;
        display.filledRect({ 0, 0 }, { 160, 80 }, { { 0, 0, 0 } });
        currentView->render(display);
        if (currentView == &mainView && isTriggered) display.filledCircle({ 150, 7 }, 4, { { 0, 255, 0 } });
        needsRedraw = false;
        return true;
    }
};
