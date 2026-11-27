---
name: zic-studio-ui-perf
description: UI performance and rendering architecture rules for studio/zicStudio, so the desktop rack stays cheap when idle and only repaints animated regions.
---

# zicStudio UI Performance

Use this skill whenever you add or change anything drawn by `studio/zicStudio/emu/zic.cpp`.

The CPU meter in the sidebar shows process usage. At idle the shell should do
almost no work: a still frame must not rebuild the whole UI every tick.

## Rendering layers

Keep the frame split into layers with different repaint policies:

| Layer | Owner | Repaint policy |
| --- | --- | --- |
| Static shell | `StudioUi::drawChrome()` | Only when `chromeDirty` is set: layout, selection, engine, transport, page/scroll, hover, input, or the slow CPU-text timer. Blit it as two region sprites (sidebar + sequencer), never as one full-window sprite. |
| Instrument panel | `StudioUi::drawPanelView()` / `panelRT` | Only on change or a few fps for scopes. |
| Tape | `drawTape()` | Per frame while recording/fading; it is an isolated cheap ribbon. |
| Playhead | `drawSeqCursor()` | Per frame, one small rect. |
| Live meters | `drawLiveMeters()` | Per frame, up to 8 tiny rects. |

The main loop also skips the whole frame when nothing is animating and
`StudioUi::wantsRedraw()` is false (`transportPlaying`, meters, tape fade,
input, or a 1 s safety refresh keep it alive). Do not remove that gate: a
no-op frame must not call `ui.draw()` or `window.display()`.

## Rules

1. **Never add a full-window redraw path.** New static chrome belongs in
   `drawChrome()` and must be covered by an invalidation trigger.
2. **Invalidate deliberately.** Call `chromeDirty = true` on every input or
   state change that affects the shell (`mousePressed`, `mouseReleased`,
   `wheel`, `key`, `relayout`, selection, engine, transport). If you add a new
   state source, add the invalidation at the same time.
3. **Do not bake animation into the chrome.** Meters, tape, the playhead and
   other per-frame values must be overlays, never part of `chromeRT`.
4. **Cache at the right granularity.** Prefer a small cached region plus an
   overlay over a full redraw. The panel already has `panelRT`; reuse that
   pattern instead of inventing a second panel path.
5. **Avoid per-frame allocation.** Reuse member buffers (see `tapeRibbon`) and
   avoid constructing large `VertexArray`s or `sf::Text` objects in hot paths.
6. **Keep framerate honest.** Idle runs at 6 fps, playing at 20 fps. Raise it
   only with a measured reason.
7. **Respect the 1:1 pixel layout.** Do not add scaling/smoothing when drawing
   cached textures; the shell is laid out in real window pixels.
8. **Verify visually.** After UI changes run:
   - `make -C studio/zicStudio build_zic`
   - `make -C studio/zicStudio screenshots`

   All screenshots must stay non-empty and must show the playhead and meters.

## Debugging checklist

- CPU high while idle: check that `chromeDirty` is not being set every frame and
  that `drawChrome()` is not called unconditionally.
- Stale text/rows: a state change is missing an invalidation.
- Playhead not moving: it was accidentally baked into the chrome; move it to
  `drawSeqCursor()`.
- Meters frozen: they were added to the cached sidebar; move them to
  `drawLiveMeters()` or the slow refresh path.
- Mouse hover lag: verify `mouseMoved()` invalidates chrome only for the
  sidebar / sequencer / splitter regions.
