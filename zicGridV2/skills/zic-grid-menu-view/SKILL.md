---
name: zic-grid-menu-view
description: Architecture standards, single encoder navigation, and 4-pad control layout (Z=Up, X=Down, C=OK, V=Cancel/Back) for zicGridV2 MenuView.
---

# zicGridV2 MenuView Skill

This skill defines the control, navigation, and visual standards for `MenuView` in **zicGridV2** (located in `zicGridV2/views/MenuView.h`).

## Core Principles & Design Rules

### 1. Single Parameter Navigation Control
- `MenuView` must use **only 1 active parameter/encoder** (Encoder `0`) to navigate or select options within the active menu list/sub-menu.
- Encoders `1` through `11` must remain cleared/hidden:
  ```cpp
  for (int i = 1; i < TOTAL_ENCODERS; ++i) {
      gridState.setEncoder(i, "", 0.0f, 0.0f, 1.0f, 1.0f, nullptr, { 0, 0, 0, 0 });
  }
  ```

### 2. Standard 4-Pad Utility Controls (Z, X, C, V)
Navigation and actions in `MenuView` are bound to the 4 utility pads in Row 3 (Columns 8 through 11):

| Pad | Key / Position | Icon / Label | Behavior |
|---|---|---|---|
| **Z** | `utilCol 0` (`gridState.pads[8][3]`) | `&icon::arrowUp::filled` | Move selection **Up** (decrement index) |
| **X** | `utilCol 1` (`gridState.pads[9][3]`) | `&icon::arrowDown::filled` | Move selection **Down** (increment index) |
| **C** | `utilCol 2` (`gridState.pads[10][3]`) | `"OK"` | Confirm / Select option |
| **V** | `utilCol 3` (`gridState.pads[11][3]`) | `"Back"` / `"Cancel"` / *Hidden* | Go back or cancel operation |

#### Conditional Visibility of Pad V (Cancel / Back)
- **Pad V is visible ONLY when necessary**:
  - Visible in sub-menus (e.g. `MODE_AUDIO_SELECT` -> `"Back"`).
  - Visible during confirmation prompts (e.g. `confirmShutdown` -> `"Cancel"`).
  - **Hidden / Empty** when at the top-level main menu with no active modal or sub-view.
- Implementation pattern for Pad V:
  ```cpp
  if (confirmShutdown) {
      gridState.pads[11][3].label = "Cancel";
      gridState.pads[11][3].color = { 220, 60, 60, 255 };
  } else if (currentMode == MODE_AUDIO_SELECT) {
      gridState.pads[11][3].label = "Back";
      gridState.pads[11][3].color = { 220, 100, 60, 255 };
  } else {
      // Hidden when not needed
      gridState.pads[11][3].label = "";
      gridState.pads[11][3].color = { 25, 30, 40, 255 };
  }
  ```

---

## Input Event Handling

### Encoder Input (`handleEncoder`)
```cpp
void handleEncoder(int encoderId, int delta) override
{
    if (encoderId == 0) {
        // Increment/decrement option using delta and clamp bounds
        selectedOption = std::clamp(selectedOption + delta, 0, OPTION_COUNT - 1);
        updateEncoderLabels();
        updatePadLeds();
    }
}
```

### Pad Input (`handleUtilityPadPress`)
```cpp
void handleUtilityPadPress(int utilCol, bool pressed) override
{
    if (!pressed) return;

    if (utilCol == 0) {
        // Pad Z: Move Up
    } else if (utilCol == 1) {
        // Pad X: Move Down
    } else if (utilCol == 2) {
        // Pad C: Confirm / OK
    } else if (utilCol == 3) {
        // Pad V: Cancel / Back (Only process when active/visible)
    }
}
```

---

## Lifecycle Cleanup
When deactivating `MenuView`, utility pads must be restored to defaults so other views can take control:
```cpp
void onDeactivate() override
{
    restoreDefaultUtilityPads();
}
```
