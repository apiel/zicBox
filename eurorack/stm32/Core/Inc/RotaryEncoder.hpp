/**
 * @file RotaryEncoder.hpp
 * @brief Header-only rotary encoder handler with button
 * 
 * Uses TIM4 in encoder mode (PD12, PD13)
 * Button on PD14 with external interrupt
 */

#pragma once

#include "main.h"
#include <cstdint>

class RotaryEncoder {
public:
    enum class Direction {
        NONE = 0,
        CW = 1,
        CCW = -1
    };
    
private:
    TIM_HandleTypeDef* htim;
    
    // Encoder state
    int32_t currentValue = 0;
    int32_t lastCount = 0;
    
    // Button state
    volatile bool buttonPressed = false;
    volatile uint32_t lastButtonTime = 0;
    static constexpr uint32_t DEBOUNCE_MS = 50;
    
    // Callbacks
    void (*onRotate)(int32_t value, Direction dir) = nullptr;
    void (*onButton)() = nullptr;
    
public:
    RotaryEncoder(TIM_HandleTypeDef* timer) : htim(timer) {}
    
    void init() {
        // Start encoder
        HAL_TIM_Encoder_Start(htim, TIM_CHANNEL_ALL);
        
        // Set counter to middle value to allow bidirectional counting
        __HAL_TIM_SET_COUNTER(htim, 32768);
        lastCount = 32768;
        currentValue = 0;
    }
    
    // Set callback for rotation events
    void setRotateCallback(void (*callback)(int32_t value, Direction dir)) {
        onRotate = callback;
    }
    
    // Set callback for button press
    void setButtonCallback(void (*callback)()) {
        onButton = callback;
    }
    
    // Call this regularly (e.g., every 10ms or in main loop)
    void update() {
        // Read current encoder count
        int32_t currentCount = (int32_t)__HAL_TIM_GET_COUNTER(htim);
        
        // Calculate delta
        int32_t delta = currentCount - lastCount;
        
        // Handle overflow/underflow
        if (delta > 32768) {
            delta -= 65536;
        } else if (delta < -32768) {
            delta += 65536;
        }
        
        // Update if changed
        if (delta != 0) {
            currentValue += delta;
            lastCount = currentCount;
            
            // Determine direction
            Direction dir = (delta > 0) ? Direction::CW : Direction::CCW;
            
            // Call callback
            if (onRotate) {
                onRotate(currentValue, dir);
            }
        }
    }
    
    // Get current encoder value
    int32_t getValue() const {
        return currentValue;
    }
    
    // Reset encoder value to zero
    void reset() {
        currentValue = 0;
        __HAL_TIM_SET_COUNTER(htim, 32768);
        lastCount = 32768;
    }
    
    // Set encoder value
    void setValue(int32_t value) {
        currentValue = value;
    }
    
    // Button interrupt handler - call from HAL_GPIO_EXTI_Callback
    void handleButtonInterrupt() {
        uint32_t now = HAL_GetTick();
        
        // Debounce
        if (now - lastButtonTime > DEBOUNCE_MS) {
            lastButtonTime = now;
            buttonPressed = true;
            
            // Call callback
            if (onButton) {
                onButton();
            }
        }
    }
    
    // Check if button was pressed (clears flag)
    bool wasButtonPressed() {
        if (buttonPressed) {
            buttonPressed = false;
            return true;
        }
        return false;
    }
    
    // Get number of clicks since last read
    int32_t getClicks() {
        int32_t clicks = currentValue;
        currentValue = 0;
        return clicks;
    }
    
    // Get direction only (no accumulation)
    Direction getDirection() {
        int32_t currentCount = (int32_t)__HAL_TIM_GET_COUNTER(htim);
        int32_t delta = currentCount - lastCount;
        
        if (delta > 32768) delta -= 65536;
        else if (delta < -32768) delta += 65536;
        
        lastCount = currentCount;
        
        if (delta > 0) return Direction::CW;
        if (delta < 0) return Direction::CCW;
        return Direction::NONE;
    }
};