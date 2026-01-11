/** Description:
This code defines a blueprint for a highly specialized user interface element used within a larger application, likely related to music or audio production—specifically, a "sequencer." A sequencer is a tool used to structure and trigger musical events, often step-by-step.

### Purpose of the Component

The code defines a structure named `SequencerValue`. This structure acts as a mandatory configuration file for creating a complex sequencing control panel on the screen. By using a helper function (`getJsonComponent`), the application ensures that whenever a developer creates a `SequencerValue` panel, they must provide all the necessary settings for it to function correctly.

### Key Configuration Requirements

This blueprint enforces several groups of settings:

1.  **Mandatory Control Settings:**
    *   **`audioPlugin`:** Specifies which sound generator or effect this sequencer control is managing.
    *   **`type`:** This is the most critical setting, defining the precise mode of operation. It dictates how the user interacts with the steps. Examples include modes for selecting specific notes (`STEP_NOTE`), turning steps on or off (`STEP_TOGGLE`), or adjusting volume or duration (`STEP_VELOCITY`, `STEP_LENGTH`).

2.  **Data and Structure Settings:**
    *   Settings like `maxSteps` and `stepsDataId` control the underlying data structure, determining how many steps are available in the sequence and where the data is stored.

3.  **Visual Customization:**
    *   A wide array of optional settings allows complete control over the component’s appearance, including various colors (`bgColor`, `barColor`, `valueColor`), font sizes, and font styles for labels and numerical values.

In essence, this TypeScript definition ensures consistency across the entire application by making sure every sequence control panel is created with the required operational parameters and styling options.

sha: 3cc668befd36890f6c8cb7e761ce6e80ce8a22b62d32f8c2870b4b306d92cd01 
*/
import { getJsonComponent } from '../ui';

export type SequencerValueType =
    | 'STEP_SELECTION'
    | 'STEP_TOGGLE'
    | 'STEP_NOTE'
    | 'STEP_CONDITION'
    | 'STEP_VELOCITY'
    | 'STEP_LENGTH'
    | 'STEP_MOTION'
    | 'STEP_LENGTH_AND_TOGGLE'
    | 'DENSITY'
    | 'GENERATOR';

export const SequencerValue = getJsonComponent<{
    audioPlugin: string;
    bgColor?: string;
    stepsDataId?: string;
    maxSteps?: number;
    labelColor?: string;
    valueColor?: string;
    barColor?: string;
    contextId?: number;
    valueSize?: number;
    labelSize?: number;
    fontValue?: string;
    fontLabel?: string;
    encoderId?: number;
    type: SequencerValueType;
    selectStepUsingLength?: boolean;
}>('SequencerValue');
