idf_component_register(
    SRCS "main.cpp"
    PRIV_REQUIRES
        esp_driver_i2c
        esp_driver_gpio
        esp_driver_i2s
        spi_flash
        esp_timer
    INCLUDE_DIRS
        "."
        "../../.."
)
