/** Description:
This code defines a specialized software function designed to create configuration blueprints for a digital control feature known as a "Knob Action."

**What This Code Does**

Imagine you are setting up a virtual control panel with various knobs and dials. This function acts as a factory, ensuring every detail required to make one of those knobs work is properly collected and formatted.

The primary role of the `KnobAction` function is to gather essential ingredients (known as "properties"):

1.  **The Task:** What the knob actually does when turned (the primary and optional secondary `action`).
2.  **Hardware Link:** Identification details for the physical input device controlling it (`encoderId`).
3.  **Visibility Rules:** When and where the knob configuration should be visible within the application or interface.
4.  **Appearance:** Structural details like keypad layouts and the necessary color scheme.

**How It Works**

Instead of building the control feature from scratch every time, this function uses a standardized helper tool (`getJsonComponent`).

1.  It collects all the configuration details specific to the Knob.
2.  It sends these details to the helper tool, telling it to create a standard software component specifically labeled 'KnobAction'.
3.  The helper tool wraps these settings into a complete, standardized structure, ready to be consumed by the larger application. This standardized output ensures that the resulting configuration (the "Knob Action") is recognized and rendered correctly within the control system or user interface.

sha: 2f1a5f7787454d83432366b292a1442b41f81f02dcd177f7fcad8bc057078d22 
*/
import { getJsonComponent } from '../ui';
import { ControllerColors, KeypadLayout, VisibilityContext } from './component';

export const KnobAction = (
    props: {
        action: string;
        action2?: string;
        encoderId?: number;
        visibilityContext?: VisibilityContext[];
        visibilityGroup?: number;
        keys?: KeypadLayout[];
        controllerColors?: ControllerColors[];
    }
) => getJsonComponent('KnobAction')({ ...props, bounds: [0, 0, 0, 0] });
