cmake_minimum_required(VERSION 3.13)

# Set path to the pico-sdk
set(PICO_SDK_PATH ${CMAKE_CURRENT_LIST_DIR}/../pico-sdk)

# Initialize the SDK
include(${PICO_SDK_PATH}/external/pico_sdk_import.cmake)

project(zic)

pico_sdk_init()

add_executable(zic
    main.cpp
)

target_link_libraries(zic pico_stdlib)
# If your board has the CYW43 Wi-Fi chip (like Raspberry Pi Pico W), link the Wi-Fi driver.
if (PICO_CYW43_SUPPORTED)
    target_link_libraries(zic pico_cyw43_arch_none)
endif()

pico_add_extra_outputs(zic)
