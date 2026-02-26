#pragma once
#include <cstdint>
#include <cstring>

extern "C" {
#include "W25QXX.h"
}

// Last sector of W25Q64 (8MB - 4KB = 0x7FF000)
#define SETTINGS_ADDR   0x7FF000
#define SETTINGS_MAGIC  0xBEEF4242

struct SettingsData {
    uint32_t magic;
    float    volume;
    float    bpm;
    uint8_t  engineType;
    uint8_t  _pad[7];  // keep struct <= 256 bytes (one page write)
};

class SettingsManager {
    SPI_HandleTypeDef* hspi;

public:
    explicit SettingsManager(SPI_HandleTypeDef* h) : hspi(h) {}

    // Call once at boot — fills defaults if no saved data
    void load(float& volume, float& bpm, uint8_t& engineType)
    {
        SettingsData s;
        W25QXX_Read(hspi, SETTINGS_ADDR,
                    reinterpret_cast<uint8_t*>(&s), sizeof(s));

        if (s.magic == SETTINGS_MAGIC) {
            volume     = s.volume;
            bpm        = s.bpm;
            engineType = s.engineType;
        }
        // else: caller keeps whatever defaults were set before calling load()
    }

    bool save(float volume, float bpm, uint8_t engineType)
    {
        SettingsData s;
        s.magic      = SETTINGS_MAGIC;
        s.volume     = volume;
        s.bpm        = bpm;
        s.engineType = engineType;
        memset(s._pad, 0, sizeof(s._pad));

        if (!W25QXX_EraseSector(hspi, SETTINGS_ADDR)) return false;
        return W25QXX_WritePage(hspi, SETTINGS_ADDR,
                                reinterpret_cast<const uint8_t*>(&s), sizeof(s));
    }
};