---
name: audio-sequencer
description: Architecture standards, file layout, Step models, pattern generators, clip chains, and project JSON persistence guidelines for audio/sequencer library components.
---

# Audio Sequencer Guidelines

Use these rules and guidelines when creating, refactoring, or integrating sequence, pattern generation, clip chain, or project persistence components.

---

## 1. Directory & File Layout

All reusable sequencer components belong in `audio/sequencer/`:

```
audio/sequencer/
├── Step.h           # Step data model & SEQ_STEPS override guard
├── Clip.h           # Clip & ParamValue data structures
├── Generator.h      # Algorithmic pattern generators (Kick, Bass, Drum, Perc, Snare, Hat, Clap)
├── SequenceUtils.h  # Sequence manipulation (stretch, compress, clear)
├── ClipChain.h      # Clip chain helper utilities (add, remove, clear, toggle)
└── ProjectIO.h      # JSON project & clip persistence (saveClip, loadClip, saveProject, loadProject)
```

---

## 2. Step Data Model (`Step.h`)

* `SEQ_STEPS`: Standard sequence step count (default: `64`). Always wrap `SEQ_STEPS` with `#ifndef` so compiler options (`-DSEQ_STEPS=32`) can override it at compile time:
  ```cpp
  #pragma once

  #ifndef SEQ_STEPS
  #define SEQ_STEPS 64
  #endif

  struct Step {
      bool active = false;
      int note = 60;
      float velocity = 0.8f;
      float condition = 1.0f; // 0.0f..1.0f probability check
      float len = 1.0f;       // step duration (1.0f = 1 step)
  };
  ```

---

## 3. Clip Data Model (`Clip.h`)

* `ParamValue`: Key-value pair storing saved parameter state (`key`, `value`, `string` representation).
* `Clip`: Storage container for track sequences, saved parameters, engine ID, note repeat rate, and save state:
  ```cpp
  #pragma once

  #include <string>
  #include <vector>
  #include "audio/sequencer/Step.h"

  struct ParamValue {
      std::string key;
      float value;
      std::string string;
  };

  struct Clip {
      bool validated = false;
      std::vector<ParamValue> paramValues;
      std::vector<Step> sequence;
      bool saved = false;
      uint8_t engineId = 0;
      std::string name;
      int noteRepeat = 2;
  };
  ```

---

## 4. Algorithmic Pattern Generators (`Generator.h`)

* All pattern generators live in `namespace Generator`.
* Provide both 4-parameter overloads (`p1`, `p2`, `p3`) for UI parameter knobs and 1-parameter default overloads (`sequence`) for exact function pointer type matching:
  ```cpp
  namespace Generator {
      inline float rand01();
      inline int randInt(int min, int max);
      inline void clearSequence(std::vector<Step>& sequence);
      inline void generateKick(std::vector<Step>& sequence, float p1, float p2, float p3);
      inline void generateKick(std::vector<Step>& sequence); // 1-arg overload wrapper
      inline void generateBass(std::vector<Step>& sequence, float p1, float p2, float p3);
      inline void generateBass(std::vector<Step>& sequence); // 1-arg overload wrapper
      inline void generateDrum(std::vector<Step>& sequence, float p1, float p2, float p3);
      inline void generateDrum(std::vector<Step>& sequence); // 1-arg overload wrapper
  }
  ```

---

## 5. Sequence Manipulation (`SequenceUtils.h`)

Use inline helper functions for track sequence transformations:
* `stretchSequence(sequence, genLen, isSynth, setLen)`: Expands step spacing by 2x.
* `compressSequence(sequence, genLen, isSynth, setLen)`: Compresses step spacing by 2x.
* `clearSequence(sequence)`: Resets step activity across `SEQ_STEPS`.

---

## 6. Clip Chain Management (`ClipChain.h`)

Manipulate clip chain vectors using shared utility functions:
* `addClipToChain(chain, clipIdx)`: Appends clip index to track chain.
* `removeClipFromChain(chain, index)`: Erases item at index from track chain.
* `clearChain(chain)`: Erases all items from track chain.
* `toggleChainPlaying(chainPlaying, chainActiveIdx)`: Toggles chain playback mode.

---

## 7. Project & Clip JSON Persistence (`ProjectIO.h`)

* Uses `nlohmann/json.hpp` for project file I/O.
* Utilizes C++17 SFINAE member-detection type traits (`has_noteRepeat`, `has_chain`) to support any application `Track` or `Studio` structure without tight coupling:
  ```cpp
  saveClip(trk, clipIdx);                                    // Serializes clip params & steps
  loadClip(trk, clipIdx);                                    // Restores engine, params & steps
  loadProjectFromJSON(studio, path);                         // Deserializes project JSON
  saveProjectToJSON(studio, path);                         // Serializes project JSON
  getCurrentLoadedProject(projectFolder);                    // Reads .current file
  setCurrentLoadedProject(filename, projectFolder);          // Writes .current file
  ```

---

## 8. Integration Checklist

When adding sequencer features to a new or existing application:

- [ ] Include `#include "audio/sequencer/Step.h"` for step definitions.
- [ ] Include `#include "audio/sequencer/Clip.h"` for clip data storage.
- [ ] Include `#include "audio/sequencer/Generator.h"` for pattern generation.
- [ ] Include `#include "audio/sequencer/SequenceUtils.h"` for sequence stretch/compress.
- [ ] Include `#include "audio/sequencer/ClipChain.h"` for clip chain management.
- [ ] Include `#include "audio/sequencer/ProjectIO.h"` for JSON project saving/loading.
- [ ] Add `project.h` to application directory wrapping `ProjectIO.h` with application-specific `PROJECT_FOLDER`.
- [ ] Verify clean build across all target platforms (`make build_zic` or `make build_studio`).
