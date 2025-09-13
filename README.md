> [!TIP]
> ### Help wanted
> I am especially searching help for:
> - 3d design for enclosure (3d printer or CNC machine)
> - PCB design improvement
>
> Support with anything else (C++, TypeScript, doc, design, testing, video...), is also more than welcome ;-)
>
> Have a look to our [contribution page](https://github.com/apiel/zicBox/wiki/40-Contribute-to-zicBox).

# ZicBox: A Modular Framework for Building Music Applications

**ZicBox** is a versatile C++ framework designed to simplify the development of music applications, with a primary focus on creating grooveboxes and drum machines for custom hardware running on **Raspberry Pi**. However, it is not limited to embedded devices, it also runs on **Linux desktops**, making it a basic foundation for building full-fledged music production software.

<!-- https://discord.gg/65HTx7z9qg -->

**If you have any questions, feel free to ask on Discord:**

[![](https://dcbadge.limes.pink/api/server/65HTx7z9qg)](https://discord.gg/65HTx7z9qg)

## 🎵 Features 🎶

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

### **Scripting & UI Configuration with TypeScript & JSX**
For configuring the framework, ZicBox use **JSON**. However, instead of writing pure JSON, developers can use **TypeScript** to generate the configuration file. This allows for a **strongly typed** configuration system while still benefiting from JSON lightweight runtime.

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
apt-get install build-essential librtmidi-dev libsndfile1-dev pulseaudio alsa-utils nlohmann-json3-dev libfreetype6-dev -y
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

Now you should get something like:

<img src="https://raw.githubusercontent.com/apiel/zicBox/main/demo.png" />

**Yes, I know, it sounds boring!** But before we complicate things, let's take a moment to understand how everything works. First, let's take a look at the code:

```tsx
import * as React from '@/libs/react';

import { writeFileSync } from 'fs';
import 'tsconfig-paths/register'; // To solve imported alias

import { audioPlugin } from '@/libs/audio';
import { Keyboard } from '@/libs/nativeComponents/Keyboard';
import { KnobValue } from '@/libs/nativeComponents/KnobValue';
import { List } from '@/libs/nativeComponents/List';
import { Rect } from '@/libs/nativeComponents/Rect';
import { Text } from '@/libs/nativeComponents/Text';
import { Value } from '@/libs/nativeComponents/Value';
import { View } from '@/libs/nativeComponents/View';

const ScreenWidth = 240;
const ScreenHeight = 320;

const demoView = (
    <View name="Demo">
        <Text fontSize={16} text="title" bounds={[0, 0, ScreenWidth, 16]} centered />

        <Rect color="tertiary" filled={false} bounds={[10, 30, 100, 50]} />
        <Rect color="primary" bounds={[120, 30, 110, 20]} />

        <Value
            audioPlugin="Tempo"
            param="BPM"
            bounds={[120, 60, 110, 20]}
            encoderId={0}
            barColor="quaternary"
        />

        <KnobValue
            audioPlugin="Tempo"
            param="BPM"
            bounds={[10, 90, 70, 70]}
            encoderId={1}
            color="secondary"
        />

        <List items={['item1', 'item2', 'item3']} bounds={[120, 100, 110, 60]} />

        <Keyboard bounds={[0, 175, ScreenWidth, 120]} />
    </View>
);

const output = JSON.stringify(
    {
        audio: { tracks: [{ id: 0, plugins: [audioPlugin('Tempo')] }] },
        pixelController: 'rpi3A_4enc_11btn',
        screen: { screenSize: { width: ScreenWidth, height: ScreenHeight } },
        views: [demoView],
    },
    null,
    2
);

writeFileSync('config.json', output);
console.log('done');
```

If you prefer not to use TypeScript for the configuration file, you directly edit the JSON file or use any scripting language, Lua, Python, or even define everything directly in C++. However, I personally find TypeScript/JSX to be the most convenient option due to its strong typing, ease of use, and developer-friendly tooling (and maybe also because I am orginally web developer ^^).

Now, you might be wondering about performance, don’t worry! The configuration process is purely declarative, meaning TypeScript is only used to define settings and structure. Once the JSON configurations are initialized, the actual execution happens entirely in C++, ensuring optimal performance.

In short, you get the best of both worlds, TypeScript for a clean and manageable configuration setup, and C++ for high-performance execution.

More [documentation here](https://github.com/apiel/zicBox/wiki).

---

🎧 Built for musicians & developers

<table>
    <tr>
        <td><img src='https://github.com/apiel/zicBox/blob/main/images/zicpad.png?raw=true' width='480'></td>
        <td><img src='https://github.com/apiel/zicBox/blob/main/hardware/ZicPixel/pixel3A.png?raw=true' width='480'></td>
        <td><img src='https://github.com/apiel/zicBox/blob/main/hardware/ZicPixel/pixel.png?raw=true' width='480'></td>
    </tr>
        <tr>
        <td><img src='https://github.com/apiel/zicBox/blob/main/hardware/ZicPixel/pixel_12btn.png?raw=true' height='480'></td>
        <td><img src='https://github.com/apiel/zicBox/blob/main/images/demo3.png?raw=true' width='480'></td>
        <td><img src='https://github.com/apiel/zicBox/blob/main/hardware/ZicPixel/pixel3A_2.png?raw=true' width='480'></td>
    </tr>
</table>

## Video

https://www.youtube.com/playlist?list=PLikTjJf4SwPpBYi4iVVPOwZ3j4N9gT0Sy

https://github.com/apiel/zicBox/assets/5873198/ce7851f0-9692-4132-8f6f-98f3bc18849a

https://github.com/apiel/zicBox/assets/5873198/230f3da5-50a4-4377-aa6f-c81a4099012f

https://github.com/apiel/zicBox/assets/5873198/e99b061a-aea1-4e77-b8cd-2412d9264760

