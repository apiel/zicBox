---
name: audio-engine
description: Architecture standards, file layout, EngineBase inheritance, and parameter guidelines for creating, refactoring, or moving audio engines.
---

# Audio Engine Guidelines

Use these rules and guidelines when creating, refactoring, or moving audio engines.

## 1. File Location & Naming
- Engine header files belong in `audio/engines/`.
- File name must match the class name in PascalCase (e.g., `DrumKickFM.h` -> `class DrumKickFM`).
- Engines are typically categorized by type (by prefixing the file name with `Drum`, `Synth`, or `Sampler`) but this is not a strict requirement.

## 2. Parameter Array Size (CRITICAL)
- Engines declare a fixed-size `Param` array matching the exact count of parameters added:
  ```cpp
  Param params[8]; // Exact count of addParam calls!
  ```
- **CRITICAL**: Calling `addParam()` more times than the size `N` of `params[N]` will result in out-of-bounds array access and trigger a **segmentation fault** at runtime.

## 3. Parameter Registration (`addParam`)
- Register parameters in the `public` section using `addParam({ ... })`:
  ```cpp
  Param& baseFreq = addParam({ .key = "baseFreq", .label = "Sub Freq", .value = 52.0f, .min = 30.0f });
  ```
- **Only specify fields when necessary**: `Param` has built-in default values (`min = 0.0f`, `max = 100.0f`, `step = 1.0f`, etc.). Only override properties when departing from default values.
- **Callbacks (`onUpdate`, `graph`, etc.)**: Cast the `void* ctx` pointer to your concrete engine class type:
  ```cpp
  .onUpdate = [](void* ctx, float val) {
      auto* engine = static_cast<MyEngine*>(ctx);
      engine->updateFilter(val);
  }
  ```

## 4. Class Structure & Static Polymorphism (Performance)
Subclass from `EngineBase<Derived>` using CRTP (Curiously Recurring Template Pattern):

> **Performance Note**: On microcontrollers like the STM32H7, virtual function calls inside audio rendering loops incur vtable lookup overhead and prevent compiler inlining. CRTP resolves function calls to `...Impl()` at compile time (static polymorphism), eliminating vtable overhead and enabling compiler inlining in sample-rate loops.

```cpp
#pragma once

#include "audio/engines/EngineBase.h"

class MyEngine : public EngineBase<MyEngine> {
public:
    // Declare exact parameter array size matching addParam calls
    Param params[4];

    Param& pitch = addParam({ .key = "pitch", .label = "Pitch", .value = 60.0f, .max = 127.0f });
    Param& cutoff = addParam({ .key = "cutoff", .label = "Cutoff", .value = 0.5f, .max = 1.0f, .step = 0.01f });
    Param& resonance = addParam({ .key = "reso", .label = "Reso", .value = 0.1f, .max = 0.95f, .step = 0.01f });
    Param& volume = addParam({ .key = "vol", .label = "Volume", .value = 0.8f, .max = 1.0f, .step = 0.01f });

    MyEngine(const float sampleRate = 44100.0f)
        : EngineBase(Synth, "MyEngine", params)
        , sampleRate(sampleRate)
    {
    }

    // Required: Audio generation loop
    float sampleImpl()
    {
        // Compute and return single mono sample [-1.0f, 1.0f]
        return 0.0f;
    }

    // Optional: Note attack callback
    void noteOnImpl(uint8_t note, float velocity)
    {
        pitch.value = note;
    }

    // Optional: Note release callback
    void noteOffImpl(uint8_t note)
    {
    }

private:
    float sampleRate;
};
```

## 5. Integration Checklist
- [ ] Header placed in `audio/engines/EngineName.h`.
- [ ] File name matches class name.
- [ ] `Param params[N]` size `N` matches total count of `addParam` calls.
- [ ] Subclass inherits `EngineBase<EngineName>`.
- [ ] `#include "audio/engines/EngineName.h"` added to host/worker header.
- [ ] Test build and verify clean execution.
