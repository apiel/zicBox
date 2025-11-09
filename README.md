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

### **Performance & Multi-Threading**
ZicBox efficiently manages system resources:
- Running the **UI in a dedicated thread** to ensure smooth rendering and interaction.
- **Routing all control data** between the UI and the audio engine efficiently.
- Running each **audio track in a separate thread**, ensuring optimal processing.
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

The fastest way to dive into **zicBox** is to start with one of the existing builds.

### ZicPixel – The Most Accessible Build

<img width="350" alt="image" src="https://github.com/user-attachments/assets/679664b5-7d3b-4592-9c9b-cd8171b85a40" /> <img width="395" alt="image" src="https://llllllll.co/uploads/default/original/3X/e/8/e8b3dba18e61384f4f33f62e848d35b5399a47c4.jpeg" />

If you want to try zicBox with minimal effort, zicPixel is the best entry point:

- **Simple & Affordable** – Only a handful of componen-ts to solder onto the PCBs.
- **Pre-Built OS** – A ready-to-flash system image is available in the Releases.
- **Enclosures** – Multiple 3D-printable cases exist.
- **Plug & Play** – Just assemble the hardware, flash the OS, and you’ll be making music in no time.

ZicPixel is the most polished and finished version of zicBox, designed to give you a clear idea of what the framework can already do today.

The **Pixel Groovebox** is a portable, performance-focused digital workstation combining **drums**, **samples**, and **synths** — all in one compact instrument.

#### 🧩 8 Tracks

- **6 Multi engine Tracks**
  - Drum: Kicks, metallic hits, claps, bass drums, and FM percussion...
  - Synth: Build lush pads, sharp leads, or evolving textures with 9 synth engines.
- **2 Sample Tracks**
  - Play, stretch, and reshape any audio using mono, granular, AM, or time-stretch engines.  

Each track runs its own **engine**, with independent envelopes, modulation, and FX.  

#### ⚙️ 22 Custom Sound Engines

- **🥁 Drums**: Kick, Clap, Bass, Metallic, Percussion, ER-1 PCM, Volca-style, FM Drum, String Drum.
- **🎹 Synth**: FM, Wavetable 1 & 2, Additive 1 & 2, SuperSaw, SpaceShip, Bass, String.
- **🎚️ Samples**: Mono, Grain, AM, Stretch.

From tight kicks to shimmering pads and evolving textures, Pixel Groovebox is both classic and experimental.  
Shape every transient, morph every tone, and push boundaries without losing groove.

Built with the mindset to keep things simple, Pixel focuses on
**hands-on control**, **instant sound design**, and  
**a world of simple, musical engines** that are easy to use.


### ZicGrid – More Advanced, More Powerful

<img src='https://github.com/apiel/zicBox/blob/main/images/zicpad.png?raw=true' width='480'>

For those looking for something bigger, **ZicGrid** takes things further:
- **RGB Grid Pad** to improve the workflow.
- **Large Screen** for a richer UI experience.
- **12 Encoders** to speed up tweaking of the sound engines.

While ZicGrid already has a strong base and working firmware, it’s more complex and expensive to build. Development is ongoing, but most efforts are currently focused on ZicPixel, so expect some rough edges.

### 🛠️ Build Your Own

Feeling adventurous? ZicBox is modular by design, so you can create your own hardware and custom UI layouts:
- Clone the repo and explore the different modules (UI, audio, hardware).
- Define your own layout and track arrangement in JSON/TypeScript.
- Build a completely custom groovebox or drum machine suited to your needs.

This option gives you maximum flexibility, but keep in mind that interest has been lower in this area, so documentation and ready-made examples is limited.


```sh
git clone --recurse-submodules https://github.com/apiel/zicBox.git
cd zicBox
```

> **Note:** If the repository has already been cloned but submodules are missing, run the following command to pull them:
>
> ```sh
> git submodule update --init
> ```

👉 Whichever path you choose, ZicBox gives you a strong foundation to build music applications – from easy-to-assemble hardware to fully custom instruments.

More [documentation here](https://github.com/apiel/zicBox/wiki).

---

🎧 Built for musicians & developers

**Example of old prototype:**
<table>
    <tr>
        <td><img src='https://github.com/apiel/zicBox/blob/main/hardware/ZicPixel/pixel3A_2.png?raw=true' width='480'></td>
        <td><img src='https://github.com/apiel/zicBox/blob/main/hardware/ZicPixel/pixel3A.png?raw=true' width='480'></td>
        <td><img src='https://github.com/apiel/zicBox/blob/main/hardware/ZicPixel/pixel.png?raw=true' width='480'></td>
    </tr>
</table>

## Video

https://www.youtube.com/watch?v=WyLNWE1CIYA&list=PLikTjJf4SwPpBYi4iVVPOwZ3j4N9gT0Sy

https://www.youtube.com/playlist?list=PLikTjJf4SwPpBYi4iVVPOwZ3j4N9gT0Sy

https://github.com/apiel/zicBox/assets/5873198/ce7851f0-9692-4132-8f6f-98f3bc18849a

https://github.com/apiel/zicBox/assets/5873198/230f3da5-50a4-4377-aa6f-c81a4099012f

https://github.com/apiel/zicBox/assets/5873198/e99b061a-aea1-4e77-b8cd-2412d9264760

