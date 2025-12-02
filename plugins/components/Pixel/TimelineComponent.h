/** Description:
This code defines a critical visual interface component, acting as the main arrangement editor for an audio or music sequencing application. Its primary function is to visualize, manage, and allow interaction with the project’s musical timeline.

### Core Functionality

1.  **Timeline Management:** The component connects to the project's overall timeline data structure. It handles "Clips," which are reusable blocks of musical information (like patterns or loops). It loads detailed data for these clips, including the notes contained within them, their length, and the specific sound instrument (engine) they use.

2.  **Visualization:** It is responsible for drawing the timeline view. This includes:
    *   A rhythmic **Grid** showing musical steps and bars.
    *   Displaying the various Clips as colored rectangles on the timeline, annotated with basic information.
    *   A miniature, piano-roll style preview within each clip block to visually represent the arrangement of notes, mapped across a standard musical range (C0 to B9).

3.  **User Interaction:** The component is highly interactive, allowing users to:
    *   **Scroll** horizontally across the timeline, either by dragging the screen or using a dedicated hardware control.
    *   **Select** clips by touching or clicking them.
    *   **Drag and Reposition** selected clips along the timeline.
    *   Navigate between different musical **Tracks**.

4.  **Synchronization:** When a user selects a clip, changes track, or scrolls the view, the component shares this information immediately with other parts of the application. This ensures that all related interfaces (like a detailed note editor) are synchronized to display the currently selected musical material.

In essence, this header defines the "brain" for the project's arrangement view, turning complex audio event data into a navigable, editable visual interface.

sha: 3b6842696bb7bf2cccb6502be3aec3f3f07d3685565b8d8978b0cf7da5ce80ad
*/
#pragma once

#include "helpers/midiNote.h"
#include "plugins/components/component.h"
#include "plugins/components/utils/color.h"

#include "plugins/audio/stepInterface.h"
#include "plugins/audio/utils/Clip.h"
#include "plugins/audio/utils/Timeline.h"

#include <climits>
#include <string>
#include <vector>

class TimelineComponent : public Component {
protected:
    AudioPlugin* clipSequencerPlugin = nullptr;
    Timeline* timeline = nullptr;
    uint8_t loadClipDataId = 1;
    Clip clip;

    // Viewport
    int32_t viewStepStart = 0; // first visible step
    int32_t viewStepCount = 128; // steps visible on screen (auto-filled on resize)
    // int stepPixel = 8; // size of each timeline step
    int stepPixel = 4;
    int laneHeight = 12;
    int clipPreviewHeight = 20;

    // Colors
    Color background;
    Color gridColor;
    Color barColor;
    Color clipColor;
    Color loopColor;
    Color textColor;
    Color selectedColor;

    void* fontLane = NULL;
    int fontLaneSize = 8;

    std::string sequencerPlugin = "Sequencer";
    std::string enginePlugin = "Track";
    struct ClipData {
        int index = 0;
        std::vector<Step> steps;
        uint16_t stepCount = 64;
        std::string engine = "";
        std::string engineType = "";
    };

    // Encoder config
    int8_t scrollEncoder = -1;

    // ---- Fixed MIDI range C0 (12) to B9 (119) = 108 semitones ----
    const int MIDI_MIN = 12;
    const int MIDI_MAX = 119;
    const float MIDI_RANGE = (float)(MIDI_MAX - MIDI_MIN);

    uint8_t trackContextId = 0;
    uint8_t stepContextId = 0;
    uint8_t viewStepStartContextId = 0;

    int16_t selectedTrack = 1;
    int32_t selectedStep = 0;

    int16_t trackMin = 1;
    int16_t trackMax = 1;

    Timeline::Event* selectedClipEvent = nullptr;
    int clipCount = 0;

    void loadClips()
    {
        if (!timeline) return;

        int clipIndex = 0;
        for (auto& event : timeline->events) {
            if (event.type == Timeline::EventType::LOAD_CLIP) {
                auto json = clip.hydrate(clip.getFilename(event.value));

                if (!json.contains(sequencerPlugin)) continue;
                auto& seqJson = json[sequencerPlugin];

                if (!seqJson.contains("STEPS")) continue;

                ClipData* data = new ClipData();
                for (auto& s : seqJson["STEPS"]) {
                    Step step;
                    step.hydrateJson(s);
                    if (step.enabled && step.len > 0)
                        data->steps.push_back(step);
                }

                data->stepCount = seqJson.value("STEP_COUNT", 64);

                if (json.contains(enginePlugin)) {
                    auto& engineJson = json[enginePlugin];
                    data->engine = engineJson.value("engine", "");
                    data->engineType = engineJson.value("engineType", "");
                }
                data->index = clipIndex++;
                event.data = data;
            }
        }
        clipCount = clipIndex;
    }

    Timeline::Event* getClipEvent(int index)
    {
        if (!timeline) return nullptr;
        for (auto& event : timeline->events) {
            if (event.type == Timeline::EventType::LOAD_CLIP) {
                auto* data = static_cast<ClipData*>(event.data);
                if (data && data->index == index) {
                    return &event;
                }
            }
        }
        return nullptr;
    }

    bool fitClipOnScreen(Timeline::Event* ev) { return fitClipOnScreen(ev->step, static_cast<ClipData*>(ev->data)->stepCount); }
    bool fitClipOnScreen(uint32_t stepStart, uint32_t stepCount)
    {
        int targetEnd = stepStart + stepCount;
        int viewEnd = viewStepStart + viewStepCount;
        if (targetEnd < viewStepStart || stepStart > viewEnd) {
            // Scroll so the clip appears fully on screen
            viewStepStart = stepStart - viewStepCount / 4;
            if (viewStepStart < 0) viewStepStart = 0;
            return true;
        } else {
            int clipEnd = static_cast<int>(stepStart + stepCount);
            int viewEnd = viewStepStart + viewStepCount;
            if (stepStart < viewStepStart || clipEnd > viewEnd) { // partiallyVisible
                // Recentre clip if half-visible
                int clipCenter = stepStart + stepCount / 2;
                viewStepStart = clipCenter - viewStepCount / 2;
                if (viewStepStart < 0) viewStepStart = 0;
                return true;
            }
        }
        return false;
    }

    void clipNext(int8_t direction)
    {
        if (!selectedClipEvent) return;

        int currentIndex = static_cast<ClipData*>(selectedClipEvent->data)->index;
        currentIndex += (direction > 0 ? 1 : -1);
        currentIndex = std::clamp(currentIndex, 0, clipCount - 1);

        Timeline::Event* nextEvent = getClipEvent(currentIndex);
        if (!nextEvent) return;
        auto* nextData = static_cast<ClipData*>(nextEvent->data);
        selectedStep = nextEvent->step;
        fitClipOnScreen(nextEvent);

        loadClip(nextEvent);

        setContext(viewStepStartContextId, viewStepStart);
        renderNext();
    }

    void trackNext(int8_t direction)
    {
        selectedTrack += (direction > 0 ? 1 : -1);
        selectedTrack = std::clamp(selectedTrack, trackMin, trackMax);
        setContext(trackContextId, selectedTrack);
        renderNext();
    }

    void loadClip(Timeline::Event* ev)
    {
        if (!ev || !clipSequencerPlugin) return;
        clipSequencerPlugin->data(loadClipDataId, &ev->value);
    }

    void selectClip(Timeline::Event* ev)
    {
        if (!ev || !ev->data) return;

        auto* data = static_cast<ClipData*>(ev->data);

        selectedStep = ev->step;
        selectedClipEvent = ev;

        // Scroll if needed
        fitClipOnScreen(ev->step, data->stepCount);

        // Notify other components
        setContext(stepContextId, selectedStep);
        setContext(viewStepStartContextId, viewStepStart);
        if (selectedTrack != track) setContext(trackContextId, track);

        loadClip(ev);

        renderNext();
    }

    Timeline::Event* getClipEventAtCoordinates(int x, int y)
    {
        if (!timeline) return nullptr;
        // Check every clip event
        // for (auto& ev : timeline->events) {
        // Iterate in reverse so top-most clip is found first
        for (int i = (int)timeline->events.size() - 1; i >= 0; --i) {
            auto& ev = timeline->events[i];
            if (ev.type != Timeline::EventType::LOAD_CLIP || !ev.data)
                continue;

            ClipData* clipData = static_cast<ClipData*>(ev.data);
            int clipStart = ev.step;
            int clipEnd = ev.step + clipData->stepCount;

            // Convert clip bounds to screen pixel positions
            int visibleStart = std::max(clipStart, viewStepStart);
            int visibleEnd = std::min(clipEnd, viewStepStart + viewStepCount);
            if (visibleEnd <= visibleStart)
                continue;

            int xA = relativePosition.x + (visibleStart - viewStepStart) * stepPixel;
            int xB = relativePosition.x + (visibleEnd - viewStepStart) * stepPixel;
            int boxWidth = xB - xA;

            if (x >= xA && x <= xA + boxWidth) {
                return &ev;
            }
        }
        return nullptr;
    }

    Timeline::Event* getClosestClipToViewStart(bool reverse = false)
    {
        if (!timeline) return nullptr;
        if (reverse) {
            for (int i = (int)timeline->events.size() - 1; i >= 0; --i) {
                auto& ev = timeline->events[i];
                if (ev.type != Timeline::EventType::LOAD_CLIP || !ev.data)
                    continue;

                if (ev.step <= viewStepStart) {
                    return &ev;
                }
            }
        } else {
            for (auto& ev : timeline->events) {
                if (ev.type != Timeline::EventType::LOAD_CLIP || !ev.data)
                    continue;

                if (ev.step >= viewStepStart) {
                    return &ev;
                }
            }
        }

        return nullptr;
    }

    bool isClipVisible(Timeline::Event* ev)
    {
        if (!ev || !ev->data) return false;

        int clipStart = ev->step;
        int clipLength = static_cast<ClipData*>(ev->data)->stepCount;

        int clipEnd = clipStart + clipLength;
        int viewEnd = viewStepStart + viewStepCount;

        return clipStart >= viewStepStart && clipEnd <= viewEnd;
    }

public:
    ~TimelineComponent()
    {
        if (timeline) {
            for (auto& event : timeline->events) {
                if (event.type == Timeline::EventType::LOAD_CLIP && event.data) {
                    delete static_cast<ClipData*>(event.data);
                }
            }
        }
    }

    TimelineComponent(ComponentInterface::Props props)
        : Component(props, [&](std::string action) {
            std::function<void(KeypadLayout::KeyMap&)> func = NULL;
            if (action.rfind(".clipNext:") == 0) {
                int8_t direction = std::stoi(action.substr(10));
                func = [this, direction](KeypadLayout::KeyMap& keymap) {
                    if (selectedTrack == track && KeypadLayout::isReleased(keymap)) {
                        clipNext(direction);
                    }
                };
            }
            if (action.rfind(".trackNext:") == 0) {
                int8_t direction = std::stoi(action.substr(11));
                func = [this, direction](KeypadLayout::KeyMap& keymap) {
                    if (KeypadLayout::isReleased(keymap)) { // selectedTrack == track &&
                        trackNext(direction);
                    }
                };
            }
            return func;
        })
    {
        nlohmann::json& config = props.config;

        /// The sequencer audio plugin.
        sequencerPlugin = config.value("sequencerPlugin", sequencerPlugin);

        /// The engine audio plugin.
        enginePlugin = config.value("enginePlugin", enginePlugin);

        /// The timeline audio plugin.
        clipSequencerPlugin = getPluginPtr(config, "timelinePlugin", track); //eq: "audio_plugin_name"
        if (clipSequencerPlugin) {
            timeline = (Timeline*)clipSequencerPlugin->data(clipSequencerPlugin->getDataId(config.value("timelineDataId", "TIMELINE")));
            loadClipDataId = clipSequencerPlugin->getDataId(config.value("loadClipDataId", "LOAD_CLIP"));
        }

        // TODO use workspace config from timeline...
        clip.config(config);

        loadClips(); // <-------- // TODO how to deal with reload workspace event? // would have to delete clips first

        /// Encoder to scroll left/right
        scrollEncoder = config.value("scrollEncoderId", scrollEncoder);

        // Colors
        background = draw.getColor(config["background"], styles.colors.background);
        gridColor = draw.getColor(config["gridColor"], lighten(styles.colors.background, 0.4));
        barColor = draw.getColor(config["barColor"], lighten(styles.colors.background, 1.0));
        clipColor = draw.getColor(config["clipColor"], styles.colors.primary);
        loopColor = draw.getColor(config["loopColor"], styles.colors.secondary);
        textColor = draw.getColor(config["textColor"], styles.colors.white);
        selectedColor = draw.getColor(config["selectedColor"], styles.colors.white);

        fontLane = draw.getFont(config.value("fontLane", "PoppinsLight_8").c_str()); //eg: "PoppinsLight_8"
        int fontSize = draw.getDefaultFontSize(fontLane);
        if (fontSize > 0) {
            fontLaneSize = fontSize;
        }

        /// Set context id shared between components to show selected track, must be different than 0.
        trackContextId = config.value("trackContextId", trackContextId); //eg: 10

        /// Set context id shared between components to show selected step, must be different than 0.
        stepContextId = config.value("stepContextId", stepContextId); //eg: 11

        /// Default selected track
        selectedTrack = config.value("defaultSelectedTrack", selectedTrack);

        /// Set context id shared between components to show selected step, must be different than 0.
        viewStepStartContextId = config.value("viewStepStartContextId", viewStepStartContextId);

        /// Set the min track number
        trackMin = config.value("trackMin", trackMin);

        /// Set the max track number
        trackMax = config.value("trackMax", trackMax);

        // if (config.contains("isInitialTrackContext")) {
        if (selectedTrack == track) {
            setContext(trackContextId, selectedTrack);
        }
    }

    void resize() override
    {
        viewStepCount = size.w / stepPixel;

        clipPreviewHeight = size.h - laneHeight - 4;
    }

    void onContext(uint8_t index, float value) override
    {
        if (index == viewStepStartContextId) {
            if ((int)value != viewStepStart) {
                viewStepStart = (int)value;
                renderNext();
            }
        } else if (index == trackContextId) {
            if ((int)value != selectedTrack) {
                bool needRender = selectedTrack == track || (int)value == track;
                selectedTrack = std::clamp((int16_t)value, trackMin, trackMax);
                if (selectedTrack == track && !isClipVisible(selectedClipEvent)) {
                    selectedClipEvent = getClosestClipToViewStart();
                    if (!selectedClipEvent) {
                        selectedClipEvent = getClosestClipToViewStart(true);
                    }
                    if (selectedClipEvent) {
                        selectedStep = selectedClipEvent->step;
                        if (fitClipOnScreen(selectedClipEvent)) {
                            setContext(viewStepStartContextId, viewStepStart);
                        }
                    }
                }
                if (needRender) {
                    renderNext();
                }
            }
        }
        Component::onContext(index, value);
    }

    void render()
    {
        draw.filledRect(relativePosition, size, { background });

        // GRID lines
        for (int i = 0; i <= viewStepCount; i++) {
            int step = viewStepStart + i;
            int x = relativePosition.x + i * stepPixel;
            Color col = (step % 16 == 0)
                ? barColor
                : (step % 4 == 0) ? gridColor
                                  : darken(gridColor, 0.5);

            draw.line({ x, relativePosition.y }, { x, relativePosition.y + size.h }, { col });
        }

        for (int i = 0; i <= viewStepCount; i++) {
            int step = viewStepStart + i;
            int x = relativePosition.x + i * stepPixel;
            if (step % 16 == 0) {
                draw.text({ x + 2, relativePosition.y }, std::to_string(step), fontLaneSize, { barColor, .font = fontLane });
            }
        }

        if (!timeline) return;

        // RENDER EVENTS
        for (auto& ev : timeline->events) {
            if (ev.step < viewStepStart && ev.step > viewStepStart + viewStepCount)
                continue;

            int x = relativePosition.x + (ev.step - viewStepStart) * stepPixel;

            if (ev.type == Timeline::EventType::LOAD_CLIP) {
                if (ev.data) {
                    ClipData* clipData = static_cast<ClipData*>(ev.data);

                    draw.filledRect({ x, relativePosition.y }, { 36, laneHeight }, { clipColor });
                    Point textPos = { x + 2, relativePosition.y + (laneHeight - fontLaneSize) / 2 };
                    draw.text(textPos, "Clip: " + std::to_string(ev.value), fontLaneSize, { textColor, .font = fontLane });
                    textPos.x += 38;
                    draw.text(textPos, clipData->engineType + " " + clipData->engine, fontLaneSize, { textColor, .font = fontLane });

                    renderClipPreview(x, relativePosition.y + laneHeight, ev, clipData);
                }
            } else if (ev.type == Timeline::EventType::LOOP_BACK) {
                // draw.filledCircle({ x + 2, relativePosition.y + laneHeight / 2 }, 4, { loopColor });
                draw.filledRect({ x, relativePosition.y }, { 36, laneHeight }, { barColor });
                draw.filledRect({ x, relativePosition.y }, { 2, laneHeight }, { loopColor });
                draw.text({ x + 5, relativePosition.y + (laneHeight - fontLaneSize) / 2 }, "<- " + std::to_string(ev.value), fontLaneSize, { textColor, .font = fontLane });
            }
        }
    }

    void renderClipPreview(int xStart, int y, Timeline::Event& ev, ClipData* clipData)
    {
        int clipStepStart = ev.step;
        // ---- viewport cropping ----
        int clipEnd = clipStepStart + clipData->stepCount;
        int viewEnd = viewStepStart + viewStepCount;

        int visibleStart = std::max(clipStepStart, viewStepStart);
        int visibleEnd = std::min(clipEnd, viewEnd);

        if (visibleEnd <= visibleStart)
            return;

        // ---- draw clip bounding box ----
        int xA = relativePosition.x + (visibleStart - viewStepStart) * stepPixel;
        int xB = relativePosition.x + (visibleEnd - viewStepStart) * stepPixel;
        int boxWidth = xB - xA;
        if (visibleEnd < clipEnd) boxWidth += 10; // just to ensure that we dont cut the clip too early making the feeling that there is nothing coming...

        // bool isSelected = track == selectedTrack && selectedStep >= clipStepStart && selectedStep < clipEnd;
        bool isSelected = track == selectedTrack && selectedStep == clipStepStart;

        if (isSelected) {
            selectedClipEvent = &ev;
        }

        draw.filledRect({ xA, y }, { boxWidth, clipPreviewHeight }, { darken(clipColor, isSelected ? 0.3f : 0.5f) });
        draw.rect({ xA, y }, { boxWidth - (isSelected ? 1 : 0), clipPreviewHeight }, { isSelected ? selectedColor : clipColor });

        // ---- draw each note in piano-roll style ----
        if (clipData->steps.size() == 0)
            return;

        for (auto& step : clipData->steps) {
            int stStart = step.position + clipStepStart;
            int stEnd = stStart + step.len;

            // Skip notes outside visible range
            if (stEnd <= visibleStart || stStart >= visibleEnd)
                continue;

            // Clip start/end
            int clippedStart = std::max(stStart, visibleStart);
            int clippedEnd = std::min(stEnd, visibleEnd);

            int px = relativePosition.x + (clippedStart - viewStepStart) * stepPixel;
            int pw = std::max(1, (clippedEnd - clippedStart) * stepPixel);

            // ---- vertical placement using fixed MIDI map ----
            float noteNorm = (float)(step.note - MIDI_MIN) / MIDI_RANGE;
            noteNorm = std::clamp(noteNorm, 0.0f, 1.0f);

            int py = y + (clipPreviewHeight - 1) - (int)(noteNorm * (clipPreviewHeight - 1));
            int ph = 3;

            draw.filledRect({ px, py }, { pw, ph }, { clipColor });
            draw.line({ px, py }, { px, py + ph - 1 }, { darken(clipColor, 0.2f) });
        }
    }

    void onEncoder(int8_t id, int8_t direction) override
    {
        if (id == scrollEncoder) {
            direction = direction > 0 ? 1 : -1;
            viewStepStart = std::max(0, viewStepStart + direction * 8);
            renderNext();
        }
    }

    const std::vector<EventInterface::EncoderPosition> getEncoderPositions() override
    {
        if (scrollEncoder < 0)
            return {};

        return {
            { scrollEncoder, size, relativePosition, .allowMotionScroll = false },
        };
    }

protected:
    bool isDragging = false;
    bool isDraggingClip = false;
    int lastDragX = 0;
    const int DRAG_THRESHOLD = 4;
    bool dragStarted = false;

public:
    void onMotion(MotionInterface& motion) override
    {
        // logDebug("motion %d %d", motion.position.x, motion.position.y);

        int mx = motion.position.x;

        // First touch point
        if (!dragStarted) {
            dragStarted = true;
            isDragging = false;
            isDraggingClip = false;
            lastDragX = mx;
            return;
        }

        int dx = mx - lastDragX;

        // If movement is large enough → we're dragging
        if (!isDragging && std::abs(dx) > DRAG_THRESHOLD) {
            isDragging = true;
            Timeline::Event* event = getClipEventAtCoordinates(mx, motion.position.y);
            if (event && event == selectedClipEvent && track == selectedTrack) {
                isDraggingClip = true;
            }
        }

        if (isDraggingClip) {
            if (selectedClipEvent) {
                int stepDelta = dx / stepPixel; // drag right = move clip right
                if (stepDelta != 0) {
                    selectedClipEvent->step += stepDelta;

                    // Prevent negative steps
                    if (selectedClipEvent->step < 0) {
                        selectedClipEvent->step = 0;
                    }
                    selectedStep = selectedClipEvent->step;

                    lastDragX = mx; // update last position for next delta
                    renderNext(); // redraw with the updated clip position
                }
            }
        } else if (isDragging) {
            // Convert pixel delta → step delta
            int stepDelta = -dx / stepPixel; // drag left = move view right

            if (stepDelta != 0) {
                viewStepStart = std::max(0, viewStepStart + stepDelta);
                lastDragX = mx;
                setContext(viewStepStartContextId, viewStepStart);
                renderNext();
            }
        }
    }

    void onMotionRelease(MotionInterface& motion) override
    {
        dragStarted = false;
        if (isDragging) {
            isDragging = false;
            return; // do NOT select a clip after dragging
        }

        // logDebug("motion release %d %d", motion.encoderId, motion.position.x, motion.position.y);
        Timeline::Event* ev = getClipEventAtCoordinates(motion.position.x, motion.position.y);
        if (ev) {
            selectClip(ev);
        }
    }
};
