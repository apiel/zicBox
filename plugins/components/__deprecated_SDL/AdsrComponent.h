/** Description:
This C/C++ header file defines the blueprint for an `AdsrComponent`, a specialized graphical element designed for a user interface, particularly relevant in audio applications like synthesizers or effects processors.

### Core Purpose

The component’s primary function is to visualize and control an **ADSR envelope**: the four critical phases that define how a sound's volume or timbre changes over time (Attack, Decay, Sustain, and Release). It acts as a bridge between abstract audio settings and a dynamic graphical representation.

### How It Works

1.  **Visualization:** The component calculates the precise shape of the ADSR curve based on the current values of the four parameters (Attack time, Decay time, Sustain level, and Release time). It dynamically draws this shape as a graph within its designated area, using custom colors for the background, lines, and filled area.
2.  **Parameter Linking:** It links each ADSR phase to an external audio processing parameter (typically residing in an audio "plugin"). This allows the graphical curve to always reflect the actual sound settings.
3.  **Interaction:** It is designed to react to physical controls, such as rotary encoders (knobs). When a user turns a designated knob, the component identifies which ADSR parameter that knob controls and adjusts the corresponding audio value, immediately updating the visual curve.
4.  **Configuration:** The component can be heavily customized at runtime. Users can configure the specific audio parameters it controls, assign different physical knobs to different phases, and set custom colors for the graph elements and the text label.

### Optional Live Feedback

The component includes an optional feature where, if activated, it can display the live progression of the sound. If the component is monitoring an active sound, it can draw a moving line across the ADSR graph, showing exactly which phase (Attack, Decay, or Sustain) the sound is currently passing through, offering dynamic visual feedback during playback.

sha: 46a56813dc8ddf3517d2f589d4a6e8716c397d9cbf9b147afcad60b8972336db 
*/
#ifndef _UI_COMPONENT_ADSR_H_
#define _UI_COMPONENT_ADSR_H_

#include "../component.h"
#include <string>
#include "../utils/color.h"

// #define ADSR_ANIMATION 1

class AdsrComponent : public Component {
protected:
    bool encoderActive = true;

    Rect graphArea;

    std::string label = "Adsr";

#ifdef ADSR_ANIMATION
    int dataId = -1;
    AudioPlugin* plugin = NULL;
#endif

    struct Phase {
        ValueInterface* value;
        int8_t encoderId;
    } adsr[4];

#ifdef ADSR_ANIMATION
    void renderStateProgression(std::vector<Point>& points)
    {
        // TODO to be implemented
        if (plugin) {
            struct AdsrState {
                int8_t phase;
                float position;
            }* adsrState = (AdsrState*)plugin->data(dataId);
            if (adsrState) {
                // printf("adsrState %d %f\n", adsrState->phase, adsrState->position);
                std::vector<Point> newPoints;
                int8_t phase = adsrState->phase + 1;
                for (int i = 0; i < points.size() && i < phase; i++) {
                    newPoints.push_back(points[i]);
                }
                int x = points[phase - 1].x + (points[phase].x - points[phase - 1].x) * adsrState->position;
                int y = points[phase - 1].y + (points[phase].y - points[phase - 1].y) * adsrState->position;
                newPoints.push_back({ x, y });
                newPoints.push_back({ x, graphArea.position.y + graphArea.size.h });

                draw.filledPolygon(newPoints, colors.progression);
            }
        }
    }
#endif

    void set(uint8_t index, int8_t encoderId, const char* pluginName, const char* key)
    {
        adsr[index].value = watch(getPlugin(pluginName, track).getValue(key));
        adsr[index].encoderId = encoderId;

        printf("set: %s %s %d = label %s\n", pluginName, key, encoderId, adsr[index].value->label().c_str());
    }

    void set(uint8_t index, char* value)
    {
        char* pluginName = strtok(value, " ");
        char* keyValue = strtok(NULL, " ");
        int8_t encoderId = atoi(strtok(NULL, " "));
        set(index, encoderId, pluginName, keyValue);
    }

    struct Colors {
        Color background;
        Color line;
        Color filling;
        Color progression;
        Color label;
    } colors;

    Colors getColorsFromColor(Color color)
    {
        return Colors({
            styles.colors.background,
            color,
            alpha(color, 0.2),
            alpha(color, 0.3),
            alpha(styles.colors.white, 0.4),
        });
    }

public:
    AdsrComponent(ComponentInterface::Props props)
        : Component(props)
        , colors(getColorsFromColor(styles.colors.primary))
    {
        graphArea = {
            { position.x + 4, position.y + 4 },
            { size.w - 20, size.h - 20 }
        };
#ifdef ADSR_ANIMATION
        jobRendering = [this](unsigned long now) {
            if (plugin && plugin->data(dataId) != NULL) {
                renderNext();
            }
        };
#endif
    }

    void render()
    {
        draw.filledRect(position, size, { colors.background });

        if (adsr[0].value == NULL || adsr[1].value == NULL || adsr[2].value == NULL || adsr[3].value == NULL) {
            return;
        }

        int w = graphArea.size.w;
        int h = graphArea.size.h;
        int a = w * adsr[0].value->pct() * 0.3;
        int d = w * adsr[1].value->pct() * 0.3;
        int r = w * adsr[3].value->pct() * 0.3;
        int s = w - a - d - r;

        int sustainY = graphArea.position.y + h * adsr[2].value->pct();

        std::vector<Point> points = {
            { graphArea.position.x, graphArea.position.y + h },
            { graphArea.position.x + a, graphArea.position.y },
            { graphArea.position.x + a + d, sustainY },
            { graphArea.position.x + a + d + s, sustainY },
            { graphArea.position.x + w, graphArea.position.y + h },
        };

        draw.filledPolygon(points, { colors.filling });
#ifdef ADSR_ANIMATION
        renderStateProgression(points);
#endif
        draw.lines(points, { colors.line });

        for (Point p : points) {
            draw.filledRect({ p.x - 2, p.y - 2 }, { 4, 4 }, { colors.line });
        }

        draw.textCentered({ (int)(graphArea.position.x + w * 0.5), graphArea.position.y + h + 2 }, label.c_str(), 10, { colors.label });
    }

    bool config(char* key, char* value)
    {
        if (strcmp(key, "COLOR") == 0) {
            colors = getColorsFromColor(draw.getColor(value));
            return true;
        }

        if (strcmp(key, "BACKGROUND_COLOR") == 0) {
            colors.background = draw.getColor(value);
            return true;
        }

        if (strcmp(key, "LABEL_COLOR") == 0) {
            colors.label = draw.getColor(value);
            return true;
        }

        if (strcmp(key, "LABEL") == 0) {
            label = value;
            return true;
        }

        if (strcmp(key, "ATTACK") == 0) {
            set(0, value);
            return true;
        }

        if (strcmp(key, "DECAY") == 0) {
            set(1, value);
            return true;
        }

        if (strcmp(key, "SUSTAIN") == 0) {
            set(2, value);
            return true;
        }

        if (strcmp(key, "RELEASE") == 0) {
            set(3, value);
            return true;
        }

#ifdef ADSR_ANIMATION
        if (strcmp(key, "DATA_STATE") == 0) {
            char* pluginName = strtok(value, " ");
            dataId = atoi(strtok(NULL, " "));
            plugin = &getPlugin(pluginName, track);
            return true;
        }
#endif

        return false;
    }

    void onEncoder(int8_t id, int8_t direction)
    {
        if (encoderActive) {
            for (int i = 0; i < 4; i++) {
                if (adsr[i].encoderId == id) {
                    adsr[i].value->increment(direction);
                    return;
                }
            }
        }
    }
};

#endif
