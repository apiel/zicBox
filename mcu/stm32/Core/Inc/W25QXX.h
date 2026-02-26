#pragma once
#include "stm32h7xx_hal.h"
#include <stdbool.h>
#include <stdint.h>

#define W25QXX_SECTOR_SIZE  4096
#define W25QXX_PAGE_SIZE    256

// CS pin — adjust port/pin if yours differs
#define FLASH_CS_PORT  GPIOB
#define FLASH_CS_PIN   GPIO_PIN_6

#define FLASH_CS_LOW()   HAL_GPIO_WritePin(FLASH_CS_PORT, FLASH_CS_PIN, GPIO_PIN_RESET)
#define FLASH_CS_HIGH()  HAL_GPIO_WritePin(FLASH_CS_PORT, FLASH_CS_PIN, GPIO_PIN_SET)

bool    W25QXX_Init(SPI_HandleTypeDef* hspi);
bool    W25QXX_ReadID(SPI_HandleTypeDef* hspi, uint8_t* id3);
bool    W25QXX_Read(SPI_HandleTypeDef* hspi, uint32_t addr, uint8_t* buf, uint32_t len);
bool    W25QXX_WritePage(SPI_HandleTypeDef* hspi, uint32_t addr, const uint8_t* buf, uint32_t len);
bool    W25QXX_EraseSector(SPI_HandleTypeDef* hspi, uint32_t addr);
bool    W25QXX_WaitBusy(SPI_HandleTypeDef* hspi, uint32_t timeout_ms);