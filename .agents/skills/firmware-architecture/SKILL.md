---
name: firmware-architecture
description: Architectural standards, target hardware abstraction (runtime*.h), thread priority management, multi-threaded audio rendering (TrackRenderPool), and file layout guidelines for firmware and applications.
---

# Firmware & Application Architecture Guidelines

Use these rules and guidelines when creating, refactoring, or structuring applications and firmwares.

---

## 1. Core Architecture Overview

Applications follow a 3-tier decoupled architecture:
1. **Target Hardware Abstraction Layer (HAL)** (`runtime*.h`): Handles display drivers, physical/virtual input events, and target-specific runtime environments.
2. **Generic Application & UI Layer** (`ui.h`, `draw.h`, component files): Pure presentation and user interaction logic, completely decoupled from target hardware details.
3. **High-Priority Audio Engine & Sequencer** (`audioWorker.h`, `TrackRenderPool.h`, audio engines): Audio synthesis, sequencing, and DSP processing running on high-priority realtime threads.

```
+-------------------------------------------------------------+
|                      Main Entry (zic.cpp)                   |
+------------------------------+------------------------------+
                               |
        +----------------------+----------------------+
        |                                             |
        v (#ifdef DRAW_SMFL)                          v (embedded target)
+-------------------------------+             +-------------------------------+
|     runtimeDesktopSFML.h      |             |       runtimeHardware.h       |
|  - SFML Window & Input        |             |  - GPIO Keys (GpioKey)        |
|  - Headless Screenshot Mode   |             |  - GPIO Encoders (GpioEncoder)|
|  - Window Pixel Buffer        |             |  - ST7789 SPI Display Driver  |
+---------------+---------------+             +---------------+---------------+
                |                                             |
                +----------------------+----------------------+
                                       | Unified Input & Draw
                                       v
                     +----------------------------------+
                     |        Generic UI (ui.h)         |
                     | - Draws to virtual Draw buffer   |
                     | - Input callbacks (keyPressed,   |
                     |   onEncoder, etc.)               |
                     +-----------------+----------------+
                                       |
                                       | Shared State & Mutexes
                                       v
                     +----------------------------------+
                     |    Audio Thread (audioWorker.h)  |
                     | - SCHED_FIFO Priority 30         |
                     | - TrackRenderPool (Multi-core)   |
                     | - ALSA Audio Stream Output       |
                     +----------------------------------+
```

---

## 2. Target Hardware Abstraction (`runtime*.h`)

Every application must strictly isolate platform-specific input drivers and display rendering behind runtime headers. The main application (`zic.cpp`) selects the runtime implementation at compile time:

```cpp
#ifdef DRAW_SMFL
#include "runtimeDesktopSFML.h"
#else
#include "runtimeHardware.h"
#endif
```

### Desktop Runtime (`runtimeDesktopSFML.h`)
- Uses **SFML** (`sf::RenderWindow`) for window management, keyboard events, and mouse wheel simulation.
- Maps PC keyboard keys to unified hardware key defines (`KEY_F1`..`KEY_F5`, `KEY_1`..`KEY_8`).
- Passes user events into generic UI handlers (e.g., `TopBar::keyPressed`, `UiTrack::onEncoder`).
- **Headless Screenshot Pipeline**: Supports environment variable `ZIC_SCREENSHOT=<path_prefix>`. When set, the application renders every view index sequentially, captures screen frames to PNG images, and exits automatically. This enables automated documentation and UI artifact generation.

### Embedded Hardware Runtime (`runtimeHardware.h`)
- Runs on Raspberry Pi or target embedded hardware.
- Driver integration:
  - **GPIO Key Matrix**: Reads physical push buttons using `GpioKey`.
  - **Rotary Encoders**: Reads quadrature rotary encoders using `GpioEncoder` with speed-scaled movement via `encGetScaledDirection()`.
  - **Physical Display**: Renders virtual pixel buffers directly to physical screens using drivers like ST7789 (`DrawToST7789`).
- **Dynamic Input Remapping**: Attempts to load custom GPIO pin mappings from `config.json` (or path specified by `ZIC_CONFIG_PATH`), gracefully falling back to hardcoded GPIO defaults if unreadable.
- **Lock-Free Event Queueing**: Receives GPIO interrupts on background threads, pushes events into lock-guarded deques (`HwKeyEvent`, `HwEncoderEvent`), and flushes them safely on the main loop iteration to prevent concurrency race conditions.

---

## 3. Thread Isolation & Real-Time Scheduling

UI rendering and Audio processing must **ALWAYS** run on separate threads.

### Real-Time Priority Allocation
Audio processing must never drop samples due to heavy UI redrawing or input processing. The audio thread MUST request POSIX Realtime scheduling (`SCHED_FIFO` priority 30):

```cpp
void setThreadRealtime(pthread_t thread, int priority, const char* name)
{
    sched_param sch {};
    sch.sched_priority = priority;
    int rc = pthread_setschedparam(thread, SCHED_FIFO, &sch);
    if (rc != 0) {
        logWarn("Unable to set realtime priority for %s (need CAP_SYS_NICE/root)", name);
    } else {
        logInfo("Realtime priority set for %s", name);
    }
}
```

### Main vs Audio Thread Roles

| Thread | Name | Priority | Responsibilities |
|---|---|---|---|
| Main Thread | `zicBox_UI` | Normal (`SCHED_OTHER`) | Hardware/desktop event loop, UI component state updates, graphics rendering to screen buffer. |
| Audio Thread | `zicBox_Audio` | Realtime (`SCHED_FIFO`, 30) | Clock timing, sequencer event processing, DSP synth rendering, master FX, ALSA output writing. |

### Thread Synchronization Rules
- Synchronize UI thread parameter changes and Audio thread rendering using lightweight, targeted mutexes (e.g., `studio.audioMutex` or per-track `historyMtx`).
- Never perform blocking file I/O, dynamic memory allocation (`malloc`/`new`), or heavy UI operations on the audio thread loop.

---

## 4. Multi-Core Audio Rendering (`TrackRenderPool`)

To maximize CPU utilization on multi-core processors without starving the UI or OS scheduler:

### CPU Core Reservation
Reserve CPU cores for the UI thread and primary audio driver thread before allocating worker pool threads:

```cpp
const size_t hw = std::thread::hardware_concurrency() == 0 ? 2 : std::thread::hardware_concurrency();
// Reserve cores for UI + main audio thread to avoid contention spikes during redraw/input
const size_t maxWorkersByHw = (hw > 2) ? (hw - 2) : 1;
const size_t workers = std::min<size_t>(4, std::max<size_t>(1, maxWorkersByHw));
```

### 2-Phase Audio Block Pipeline
Audio block rendering in `audioWorker.h` executes in two distinct phases:

1. **Phase 1: Serial Sequencer Tick (Lock Held)**
   - Lock `audioMutex`.
   - Advance global sample counter and step clock (`studio.currentStep`).
   - Populate array of per-track frame events (`TrackFrameEvent`: noteOn, noteOff, loadClip, velocity, length).

2. **Phase 2: Parallel Track Render (`TrackRenderPool<Track>`)**
   - Dispatch rendering of `N` tracks across `workers` worker threads.
   - Each worker renders its assigned tracks into a worker-local mix buffer (`partialMix_[workerIdx]`), avoiding lock contention.
   - Combine partial mixes serially into final master mix.
   - Apply master FX chain (Scatter, Filter, Compressor, Tape saturation, Soft clipping).
   - Convert `float` samples [-1.0f, 1.0f] to 16-bit PCM integer buffers and write to ALSA (`snd_pcm_writei`).

---

## 5. Standard Application Directory & File Layout

When creating a new application, organize code files according to this canonical structure:

```
myApp/
├── zic.cpp                 # Main entry point (initializes ALSA, starts audio thread, runs desktop/hardware loop)
├── audioWorker.h           # Audio thread loop, ALSA management, sequencer tick, multi-track rendering
├── runtimeDesktopSFML.h    # Desktop SFML window driver, keyboard/mouse mapping, headless screenshot pipeline
├── runtimeHardware.h       # Target hardware GPIO keys, rotary encoders, ST7789 display driver, config loading
├── studio.h (or state.h)   # Application state container, track lists, global audio mutexes
├── ui.h                    # Main UI coordinator, screen drawing dispatcher, unified event handlers
├── ui*.h                   # Individual modular UI view components (e.g. uiTrack.h, uiSeq.h, uiMenu.h)
└── makefile                # Build setup defining DRAW_SMFL for desktop or embedded targets
```

---

## 6. Architecture Verification Checklist

When adding a new firmware feature or creating a new application, verify:

- [ ] Target hardware specific code is strictly contained inside `runtimeDesktopSFML.h` or `runtimeHardware.h`.
- [ ] UI components draw to abstract `Draw` buffers without direct dependencies on SFML or hardware display APIs.
- [ ] Main entry `zic.cpp` names UI and Audio threads (`pthread_setname_np`).
- [ ] Audio thread requests `SCHED_FIFO` realtime priority with fallback logging.
- [ ] Audio state updates from UI thread use proper mutex locking (`studio.audioMutex`).
- [ ] Multi-track rendering uses `TrackRenderPool` with CPU core reservation (`hw - 2`).
- [ ] Headless screenshot mode (`ZIC_SCREENSHOT`) works as expected for desktop builds.
