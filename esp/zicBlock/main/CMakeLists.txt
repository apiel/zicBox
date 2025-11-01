idf_component_register(
    SRCS "main.cpp"
    REQUIRES driver spi_flash
    INCLUDE_DIRS
        "."
        "../../.."
)

littlefs_create_partition_image(storage ../flash_data FLASH_IN_PROJECT)