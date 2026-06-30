# zicXYv2 Skill

## Purpose
This skill describes the `zicXYv2` project for AI agents working on code implementation and UI/audio behavior.
The goal is to provide enough context so generated code matches the target platform, hardware, interaction model, and performance constraints.

## Target
- `zicXYv2` is a groovebox application running on Raspberry Pi Zero 2 W.
- It is audio-first. UI rendering must never interfere with audio processing.
- The implementation is in the `zicXYv2` folder of the `zicBox` repository.

## Hardware
- Display: 320x240 pixels.
- Buttons: 11 physical buttons.
  - First row: 5 buttons.
  - Second row: 8 buttons.
- Encoders: 4 rotary encoders.
- Audio output: DAC.
- No mouse on actual hardware.
- Desktop development can support mouse scroll and click, but code must not assume mouse is available on hardware.

## UI design principles
- Always prefer hardware interaction patterns: buttons, encoders, and small screen layout.
- Avoid designs that require a mouse or large free-form pointing interaction.
- Avoid unnecessary full-screen redraws.
- Only redraw UI elements that changed.
- Keep refresh and render work minimal so CPU time remains available for audio.
- Use a simple UI flow and clear visible state for the selected view.

## Audio priority
- Audio must always remain responsive and have priority over UI updates.
- UI rendering should be lightweight and non-blocking.
- Prefer incremental updates and state diffing rather than repeated full-screen redraws.
- Do not add expensive UI operations that could impact audio timing.

## Desktop vs hardware behavior
- Desktop mode may support SFML windowing, mouse movement, mouse button, and mouse wheel events.
- Real hardware is not using SFML and does not include mouse input.
- Code should support desktop aids only when running in a desktop environment and retain hardware-friendly control paths.

## zicXYv2 structure notes
- `windowSFML.h` integrates desktop event handling and uses `drawUI(...)`.
- `ui.h` composes UI views from: `uiMasterFx.h`, `uiTopBar.h`, `uiTrack.h`, `uiSeq.h`, `uiClips.h`, `uiProject.h`.
- `studio.h` defines the core app state, audio engine, tracks, and view enum values.
- `draw.h` and related draw helpers are used to render to the screen buffer.

## Agent guidance
- When modifying UI code, prefer updating only the affected region instead of clearing and redrawing the entire display every frame.
- When adding new interaction modes, map them to buttons/encoders first, then add desktop mouse support only as an optional convenience.
- Keep code paths for hardware and desktop separate where necessary to avoid introducing mouse-only dependencies.
- Document assumptions clearly in code comments, especially around input handling, redraw logic, and performance constraints.

## Useful constraints
- Screen size is fixed at 320x240.
- There are 4 encoders and 11 physical buttons; design interactions around these controls.
- The application already uses multiple views: track view, sequencer view, clips view, master/effects view, project view.
- Make sure new features or UI screens fit within the existing navigation model.

## Recommended behavior for this skill
- Treat `zicXYv2` as an embedded groovebox UI system with strict hardware controls.
- When asked to implement code, prioritize correctness for buttons/encoders and low-cost rendering.
- Use desktop event handling only to ease development and test flows, not as the main input model.
- Ensure any new code can run on the Pi Zero 2 W without relying on desktop-only features.
