# ZicBox: A Modular Framework for Building Music Applications

**ZicBox** is a versatile C++ framework designed to simplify the development of music applications, with a primary focus on creating grooveboxes and drum machines for custom hardware running on **Raspberry Pi**. However, it is not limited to embedded devices, it also runs on **Linux desktops**, making it a basic foundation for building full-fledged music production software.

## 🚀 Features

### **Modular Architecture**
ZicBox is built around a highly modular system that ensures flexibility and extensibility:

- **UI Components as Modules** – Every UI element is encapsulated as a module, allowing for easy customization and expansion.
- **Audio Processing as Modules** – Synth engines (such as drum synthesizers, sample players, ...) and effect processors (like filters, distortion, etc.) are also modular, enabling seamless integration of new sound-processing capabilities.
- **Hardware Interfaces as Modules** – The system abstracts hardware interactions into modules, making it simple to add support for new devices without modifying the core application.

This modular design allows developers to extend ZicBox effortlessly by adding new modules without altering the framework’s core logic.

### **Real-Time Performance & Multi-Threading**
ZicBox efficiently manages system resources to ensure **low-latency audio performance**. It achieves this by:

- Running the **UI in a dedicated thread** to ensure smooth rendering and interaction.
- **Routing all control data** between the UI and the audio engine efficiently.
- Running each **audio track in a separate thread**, ensuring optimal real-time processing.
- Handling **audio thread synchronization** to maintain precise timing and tempo accuracy.

### **Scripting & UI Configuration with TypeScript-to-Lua & JSX**
For configuring the framework, ZicBox provides a **Lua scripting interface**. However, instead of writing pure Lua, developers can use **TypeScript**, thanks to the **TypeScript-to-Lua** transpiler. This allows for a **strongly typed** configuration system while still benefiting from Lua's lightweight runtime.

Additionally, ZicBox leverages **JSX (React-style syntax)** for UI configuration, making it intuitive to define interface components. Unlike traditional React, there is no component state management involved, JSX is used purely for declarative UI structure.

## 🎛 Why Choose ZicBox?

- **🖥️ Designed for Custom Hardware** – Optimized for Raspberry Pi but also runs on Linux desktop.
- **🧩 Extensible & Modular** – Easily add new UI components, synth engines, effects, and hardware integrations.
- **⚡ High-Performance Audio** – Multi-threaded design ensures real-time audio processing with precise synchronization.
- **💡 Type-Safe Configuration** – Use TypeScript instead of Lua for a structured, maintainable setup.
- **🎨 JSX-Based UI** – Define interfaces declaratively with React-style syntax.

ZicBox is an ideal solution for developers looking to build **custom music applications**, whether for standalone hardware or full desktop music production environments.

---

## 📥 Get Started

Clone the repo and start building with **ZicBox**! 🎵

```sh
git clone --recurse-submodules https://github.com/apiel/zicBox.git
cd zicBox
```

> **Note:** If the repository has already been cloned but submodules are missing, run the following command to pull them:
>
> ```sh
> git submodule update --init
> ```
>
> For SSH submodules, use:
>
> ```sh
> git submodule update --init .gitmodules_ssh
> ```

### Install Dependencies
Update package lists and install required dependencies:

```sh
apt-get update
apt-get install build-essential librtmidi-dev libsndfile1-dev pulseaudio alsa-utils liblua5.4-dev -y
```

### Additional Dependencies for Desktop
If you are setting up on a desktop, install SDL2 development libraries:

```sh
sudo apt-get install libsdl2-dev
```

### Setup for Raspberry Pi
For Raspberry Pi, a setup script is available. Run the following commands:

```sh
cd hardware/rpi
sudo ./setup.sh
```

[View the setup script here](https://github.com/apiel/zicBox/blob/main/hardware/rpi/setup.sh)

## Run the Demo Project
Once everything is set up, you can run the demo project using:

```sh
make pixel
```

More [documentation here](https://github.com/apiel/zicBox/wiki).

---

🎧 Built for musicians & developers


| <img src='https://github.com/apiel/zicBox/blob/main/images/zicpad.png?raw=true' width='480'> | :arrow_forward: see [Video](https://github.com/apiel/zicBox/#Video) | <img src='https://github.com/apiel/zicBox/blob/main/hardware/ZicPixel/pixel.png?raw=true' width='480'> | 
| :---:   | :---: | :---: |
| <img src='https://github.com/apiel/zicBox/blob/main/images/demo3.png?raw=true' width='480'> | <img src='https://github.com/apiel/zicBox/blob/main/images/demo4.png?raw=true' width='480'> | :book: See [Documentation](https://github.com/apiel/zicBox/wiki) :books: | 
| :speech_balloon: [Discussions](https://github.com/apiel/zicBox/discussions) | <img src='https://github.com/apiel/zicBox/blob/main/hardware/ZicPixel/pixel.png?raw=true' width='480'> | [<img src='https://github.com/apiel/zicBox/blob/main/images/proto2.png?raw=true' width='480'>](https://github.com/apiel/zicBox/blob/main/images/proto2.jpg?raw=true) | 

## Video

https://github.com/apiel/zicBox/assets/5873198/ce7851f0-9692-4132-8f6f-98f3bc18849a

https://github.com/apiel/zicBox/assets/5873198/230f3da5-50a4-4377-aa6f-c81a4099012f

https://github.com/apiel/zicBox/assets/5873198/e99b061a-aea1-4e77-b8cd-2412d9264760

