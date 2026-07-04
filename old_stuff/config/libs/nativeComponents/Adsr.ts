/** Description:
This TypeScript code defines the blueprint for a specialized user interface component named `Adsr`.

### Overview

The primary function of this code is to load a pre-configured definition for a reusable UI element. The name `Adsr` stands for Attack, Decay, Sustain, and Release—a term commonly used in audio synthesis to control how a sound evolves over time.

Instead of writing the entire component logic here, the code uses a helper function (`getJsonComponent`) to retrieve its full configuration and structure from an external settings file (likely JSON). This makes the component easy to manage and reuse across different parts of the application.

### Key Capabilities and Configuration

The definition specifies exactly what kind of settings this audio control component can accept, organized into two main categories:

**1. Visual Customization:**
*   **Colors:** Properties like `fillColor`, `outlineColor`, `bgColor` (background color), and two specific text colors (`textColor1`, `textColor2`) allow detailed styling of the component’s appearance.
*   **Appearance:** Flags like `outline` and `filled` control whether the graphical representation should be drawn as an empty shape or a solid block.

**2. Functional Control:**
*   **Core Link:** A required property, `audioPlugin`, specifies the name of the underlying audio engine or software module that this visual component is controlling.
*   **Inputs:** Optional settings for four `encoders` (likely referencing input knobs or controls) and four `values` (text labels displaying the current ADSR timing settings).
*   **Layout:** The `renderValuesOnTop` flag adjusts the position of the displayed timing labels.

In essence, this code establishes the contract for a configurable, visually distinct component used to manage the timing envelope within an audio application.

sha: b31aa657790cc87a497759f5491856a19242330498cffd9164252ca9e5ae0e31 
*/
import { getJsonComponent } from '../ui';

export const Adsr = getJsonComponent<{
    outline?: boolean;
    fillColor?: string;
    filled?: boolean;
    outlineColor?: string;
    textColor1?: string;
    textColor2?: string;
    bgColor?: string;
    audioPlugin: string;
    encoders?: [number, number, number, number];
    values?: [string, string, string, string];
    renderValuesOnTop?: boolean;
}>('Adsr');