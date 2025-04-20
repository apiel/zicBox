CC?=g++

ifeq ($(cc),arm)
    CC := arm-linux-gnueabihf-g++
endif

# if uname -m is x86_64 and CC is not arm-linux-gnueabihf-g++, then we are on x86
TARGET_PLATFORM := $(shell \
    if [ "$$(uname -m)" = "x86_64" ] && [ "$(CC)" != "arm-linux-gnueabihf-g++" ]; then \
        echo x86; \
    else \
        echo arm; \
    fi)

ifeq ($(TARGET_PLATFORM),arm)
    RPI := -DIS_RPI=1
endif