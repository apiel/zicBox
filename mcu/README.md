# 25 Microcontroller

## STM32H7

This page documents the current state of **zicBox** running on **STM32H7** microcontrollers.  
The implementation serves as a **proof of concept (POC)** demonstrating that zicBox can run efficiently on a modern MCU with real-time audio synthesis capabilities.

### Overview

zicBox has been successfully ported to the STM32H7 family and is currently running on a **WeActStudio MiniSTM32H723** board.

The goal of this work is to validate STM32H7 as a solid platform for:
- Real-time drum synthesis
- Low-latency audio output
- Embedded UI (display + encoder)
- Future expansion toward sample playback and modular synth (Eurorack) compatibility

At this stage, the system is functional but still requires significant work to become feature-complete.

### Hardware Platform

- **MCU**: STM32H723
- **Board**: WeActStudio MiniSTM32H723
- **Core**: Cortex-M7
- **Clock**: High-frequency internal + PLL
- **DAC**: Built-in 12-bit DAC
- **ADC**: Multiple 16-bit ADCs
- **Storage**:
  - On-chip SPI Flash (for settings)
  - Built-in SD card interface
  - OSPI Flash (for sample playback)

### Current Features

#### Audio Engine

- One active **drum voice engine** running in real time
- Support for **multiple drum engine types** (architecture in place)
- Audio output via **built-in 12-bit DAC**
- Audio quality is sufficient for:
  - Drum synthesis
  - Percussive sounds
  - Embedded drum machine use cases

Despite the DAC being limited to 12-bit, it sound great for this application.

#### User Interface

- Display is fully operational
- Rotary encoder support
- Basic navigation and parameter control working
- UI loop integrated with the audio engine without audible glitches

#### Performance

- Real-time synthesis runs reliably on STM32H7
- No noticeable audio dropouts under current load
- Confirms STM32H7 is powerful enough for multi-voice drum synthesis

### Work in Progress / Planned Features

#### Persistent Storage

- Persisting settings in **on-chip SPI Flash**
- Presets, configuration, and state restoration

#### Sample Playback

- SD card support for loading samples
- Preloading samples into **OSPI Flash**
- Hybrid engine:
  - Synthesized drums
  - Sample-based playback

#### Eurorack Compatibility

- Add **output op-amp** stage
  - Match Eurorack signal levels
  - Improve output impedance and signal quality

#### Control Voltage (CV) Input

- Use built-in **16-bit ADCs**
- Multiple CV inputs
- Target use cases:
  - Trigger inputs
  - Modulation (pitch, decay, timbre, etc.)
  - External clock / sync

### Current Limitations

- Single active drum voice at the moment
- No persistent storage yet
- No sample playback
- No analog front-end (op-amp / CV conditioning)
- Still a POC

### Code Reference

The STM32 implementation lives here:

https://github.com/apiel/zicBox/tree/main/mcu/stm32

### Conclusion

This STM32H7 port demonstrates that zicBox can run efficiently on a microcontroller while maintaining real-time audio performance.

While still a proof of concept, it provides a **strong foundation** for:
- A standalone embedded drum machine
- A Eurorack-compatible drum module
- A hybrid synth + sampler platform

Further work will focus on storage, sample playback, analog I/O, and expanding the synthesis engine.
