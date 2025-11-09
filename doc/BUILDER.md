# 30 🧰 Build Your Own Design

The zicBox community has been growing — and lately, more people are not just using existing builds but creating their **own designs**.  
Building your own version means working with both **hardware** and **software**, and zicBox was made to give you the flexibility to do just that.

## ⚙️ Supported Platforms

zicBox currently runs on:

- **Raspberry Pi Zero 2W**
- **Raspberry Pi 3 & 4**
- **Desktop (Linux)**
  
There are also **experiments with ESP32**, and ongoing refactoring to extract an independent **audio library** that can run on microcontrollers as well.  
Optimizing for MCUs is challenging — limited RAM and the lack of full floating-point support make things tricky compared to the RPi, which has more memory and processing power.

> 💡 If you want to target an MCU, prefer boards that handle floating point efficiently, such as the **ESP32-P4**.

### 🖥️ Desktop Development

Development is primarily done on **Linux desktop**, which is the most convenient environment to start.  
Before deploying on the Raspberry Pi, it’s recommended to first **build and test on desktop**.  
That’s how all development is done here:  
90% of the time is spent developing and testing on desktop, and once stable, it’s cross-compiled and uploaded to the Pi.

zicBox also runs on **Windows via WSL**, though Mac support is currently unknown.

For ESP32 development, the process is similar — development is done on desktop using **SDL to simulate** the display and hardware (buttons, encoders, DAC, etc.), which makes iteration much faster. Because microcontrollers are more limited, testing on the real device is still essential to ensure performance and stability.

## 🖼️ Displays & Rendering

A common question is how the display system works.

- On **desktop**, zicBox uses **SDL** for rendering. This is fine for testing but inefficient on low-power devices.  
- On **Raspberry Pi**, it runs **without X server**, rendering directly to the **Linux framebuffer** (this supports **DSI** displays and likely **HDMI**, though untested).
- SPI displays are supported — currently drivers exist for **ST7789**.
- On **ESP32**, there are versions for **monochrome displays** (e.g. SSD1306, SH1107), it should also be usuable on RPi.

## 🧩 Framework Architecture

The zicBox framework is designed with multiple layers for flexibility and reuse:

### Core & Shared Code
- Low-level **audio** and **utility helpers** (shared across Linux and MCU versions)
- **Drawing library** for graphics primitives (also shared)

### Platform-Specific Layers
- For **ESP32**, everything above the shared layer is implemented in **C++ with ESP-IDF**.  
- For **Linux** (RPi & desktop), zicBox uses a **plugin-based modular system** for both **audio** and **UI**.

### Plugin System
All major features are built as **plugins** that follow a defined interface, so you can mix and match them to create your own builds.

Plugin categories include:
- **Audio plugins**
- **UI component plugins**
- **Controller plugins**

Each design is described using a large **JSON configuration file**, which defines tracks, plugins, and the user interface.

Because these files can get complex, it’s often better to **generate them programmatically**.  
You can use any language — for example:
- **TypeScript + JSX** (used here)
- **Lua**
- **Python**
- ...
- Or even pure **C++**, bypassing JSON entirely

## 🧱 UI & Audio Separation

zicBox is built around a **clear separation between audio and UI**, allowing deep customization.  
You can:
- Use the **default UI engine** (lightweight and portable)  
- Or integrate your own, such as **LVGL** or another favorite UI framework.

The system includes:
- A **multithreaded audio engine**, distributing tracks across CPU cores for maximum performance  
- A lightweight **UI view system**, with hardware input (encoders, buttons, etc.) forwarded to manage both UI and audio interactions.
