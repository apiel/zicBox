cmake_minimum_required(VERSION 3.13)

# Set paths
set(PICO_SDK_PATH ${CMAKE_CURRENT_LIST_DIR}/pico-sdk)
set(PICO_EXTRAS_PATH ${CMAKE_CURRENT_LIST_DIR}/pico-extras)

# Import pico-sdk
include(${PICO_SDK_PATH}/external/pico_sdk_import.cmake)

# Import pico-extras (for audio, etc.)
include(${PICO_EXTRAS_PATH}/external/pico_extras_import.cmake)

project(zic)

pico_sdk_init()

add_executable(zic
    main.cpp
)

pico_enable_stdio_usb(zic 1)
pico_enable_stdio_uart(zic 0)

# Link with stdlib + I2S audio from pico-extras
target_link_libraries(zic
    pico_stdlib
    pico_audio_i2s
)

# If your board has Wi-Fi (Pico W), also link the Wi-Fi driver
if (PICO_CYW43_SUPPORTED)
    target_link_libraries(zic pico_cyw43_arch_none)
endif()

# Create UF2 / bin / etc.
pico_add_extra_outputs(zic)
