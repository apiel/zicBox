idf_component_register(
    SRCS "main.cpp"
    REQUIRES driver spi_flash
    INCLUDE_DIRS
        "."
        "../../.."
)