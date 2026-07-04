# ZicBox: A Modular Framework for Building Music Applications

**ZicBox** is a modular **C++ framework** for building music applications and custom audio hardware.

The project targets **Raspberry Pi** and **Linux desktop** first, with additional proof-of-concept ports on platforms like **STM32H7** and **ESP32**.

Several hardware builds have been explored over time, and some are fully functional today.

<!-- https://discord.gg/65HTx7z9qg -->

If you have any questions, feel free to ask on **Discord**:

[![](https://dcbadge.limes.pink/api/server/65HTx7z9qg)](https://discord.gg/65HTx7z9qg)

Follow the project on **Instagram**:

[![Instagram](https://img.shields.io/badge/Follow-%40zicbox.audio-E4405F?style=for-the-badge&logo=instagram&logoColor=white)](https://www.instagram.com/zicbox.audio/)

## 🎵 Features 🎶

### **Framework Architecture**
ZicBox is organized as reusable C++ modules so new engines, devices, and interfaces can be added without rewriting the core.

### **Rendering & Platform Support**
- Runs on **Raspberry Pi** and **Linux desktop**.
- On Raspberry Pi, UI rendering can target an **SPI display** or the **Linux framebuffer** directly.
- Includes basic drawing libraries used to build portable UIs across targets.

### **Core Libraries**
- **audio/** – Audio building blocks: synth engines, filters, effects, modulation, envelopes, sequencing, and more.
- **draw/** – Cross-platform UI and rendering utilities.
- **helpers/** – System and utility layer: GPIO, SPI, I2C, filesystem helpers, math helpers, and other low-level tools.

## 🎛 Why Choose ZicBox?

- **🖥️ Hardware-First** – Built to power custom audio instruments on Raspberry Pi and desktop Linux.
- **🧩 Modular by Design** – Extend engines, UI, and hardware support incrementally.
- **🔧 Practical C++ Stack** – One primary language across engine, UI, and platform layers.

ZicBox provides a practical foundation for building **custom music applications and audio hardware**, from compact embedded instruments to desktop-based setups.

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

### Zic XY

<img src='https://github.com/apiel/zicBox/blob/main/hardware/ZicXY/Zic_XY_black.png?raw=true' width='350'> <img src='https://github.com/apiel/zicBox/blob/main/hardware/ZicXY/Zic_XY_blue.png?raw=true' width='350'>

**Zic XY** is the latest hardware iteration in the zicBox family. It features **8 versatile multi-engine tracks**, capable of running drums, synths, or samples — all handled by a single, flexible multi-engine system.

- **8 Multi-Engine Tracks** – Any track can load drums, synths, or sample engines. Independent envelopes, mixer, and FX routing.
- **DIY Friendly** – BOM, PCB files, and build instructions are available for those who want to assemble their own Zic XY.

More details: https://github.com/apiel/zicBox/wiki/20-Zic-XY

### 🛠️ Build Your Own

Feeling adventurous? ZicBox is modular by design, so you can create your own hardware and custom UI layouts:
- Clone the repo and explore the different modules (UI, audio, hardware).
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

