#include "W25QXX.h"

#define CMD_READ_ID       0x9F
#define CMD_READ_DATA     0x03
#define CMD_PAGE_PROGRAM  0x02
#define CMD_SECTOR_ERASE  0x20
#define CMD_WRITE_ENABLE  0x06
#define CMD_READ_STATUS   0x05
#define STATUS_BUSY       0x01

// ---------- low-level helpers ----------

static void spi_write_byte(SPI_HandleTypeDef* hspi, uint8_t b)
{
    HAL_SPI_Transmit(hspi, &b, 1, HAL_MAX_DELAY);
}

static uint8_t spi_read_byte(SPI_HandleTypeDef* hspi)
{
    uint8_t rx = 0;
    HAL_SPI_Receive(hspi, &rx, 1, HAL_MAX_DELAY);
    return rx;
}

static void send_addr(SPI_HandleTypeDef* hspi, uint32_t addr)
{
    spi_write_byte(hspi, (addr >> 16) & 0xFF);
    spi_write_byte(hspi, (addr >>  8) & 0xFF);
    spi_write_byte(hspi,  addr        & 0xFF);
}

// ---------- public API ----------

bool W25QXX_WaitBusy(SPI_HandleTypeDef* hspi, uint32_t timeout_ms)
{
    uint32_t start = HAL_GetTick();
    while (1) {
        FLASH_CS_LOW();
        spi_write_byte(hspi, CMD_READ_STATUS);
        uint8_t status = spi_read_byte(hspi);
        FLASH_CS_HIGH();

        if (!(status & STATUS_BUSY)) return true;
        if ((HAL_GetTick() - start) >= timeout_ms) return false;
    }
}

bool W25QXX_ReadID(SPI_HandleTypeDef* hspi, uint8_t* id3)
{
    FLASH_CS_LOW();
    spi_write_byte(hspi, CMD_READ_ID);
    id3[0] = spi_read_byte(hspi); // manufacturer  (0xEF for Winbond)
    id3[1] = spi_read_byte(hspi); // memory type   (0x40)
    id3[2] = spi_read_byte(hspi); // capacity      (0x17 = W25Q64)
    FLASH_CS_HIGH();
    return (id3[0] == 0xEF);
}

bool W25QXX_Init(SPI_HandleTypeDef* hspi)
{
    // Init CS pin as output, idle high
    __HAL_RCC_GPIOB_CLK_ENABLE();
    GPIO_InitTypeDef g = {0};
    g.Pin   = FLASH_CS_PIN;
    g.Mode  = GPIO_MODE_OUTPUT_PP;
    g.Pull  = GPIO_NOPULL;
    g.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(FLASH_CS_PORT, &g);
    FLASH_CS_HIGH();

    HAL_Delay(5); // Flash startup time

    uint8_t id[3];
    return W25QXX_ReadID(hspi, id);
}

bool W25QXX_Read(SPI_HandleTypeDef* hspi, uint32_t addr, uint8_t* buf, uint32_t len)
{
    FLASH_CS_LOW();
    spi_write_byte(hspi, CMD_READ_DATA);
    send_addr(hspi, addr);
    HAL_SPI_Receive(hspi, buf, len, HAL_MAX_DELAY);
    FLASH_CS_HIGH();
    return true;
}

bool W25QXX_EraseSector(SPI_HandleTypeDef* hspi, uint32_t addr)
{
    // Write enable
    FLASH_CS_LOW();
    spi_write_byte(hspi, CMD_WRITE_ENABLE);
    FLASH_CS_HIGH();

    // Erase sector containing addr
    FLASH_CS_LOW();
    spi_write_byte(hspi, CMD_SECTOR_ERASE);
    send_addr(hspi, addr);
    FLASH_CS_HIGH();

    return W25QXX_WaitBusy(hspi, 500); // sector erase up to ~400ms
}

bool W25QXX_WritePage(SPI_HandleTypeDef* hspi, uint32_t addr,
                      const uint8_t* buf, uint32_t len)
{
    // len must be <= 256, must not cross a 256-byte page boundary
    FLASH_CS_LOW();
    spi_write_byte(hspi, CMD_WRITE_ENABLE);
    FLASH_CS_HIGH();

    FLASH_CS_LOW();
    spi_write_byte(hspi, CMD_PAGE_PROGRAM);
    send_addr(hspi, addr);
    HAL_SPI_Transmit(hspi, (uint8_t*)buf, len, HAL_MAX_DELAY);
    FLASH_CS_HIGH();

    return W25QXX_WaitBusy(hspi, 10); // page program up to ~3ms
}