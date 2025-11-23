/** Description:
This TypeScript code defines a single, reusable software building block named `KnobValue`.

### Purpose and Functionality

The `KnobValue` component is designed to represent a digital control knob—similar to a dial on a physical piece of music equipment or software interface. Its primary function is to display and potentially control a specific parameter within an audio plugin or sound system.

### The Blueprint for Customization

The core of this code is a detailed "blueprint" that dictates exactly how this knob component can be customized when used in a project. This blueprint lists all the available settings, which fall into several categories:

1.  **Core Linkage:** Settings like `audioPlugin` and `param` determine which specific audio setting the knob is connected to and controls.
2.  **Appearance and Style:** Options such as `color`, `bgColor` (background color), and `textColor` manage the visual look.
3.  **Display Control:** Settings like `hideValue`, `hideUnit`, and `floatPrecision` control how the numerical value is displayed—whether the measurement units are visible, or how many decimal places are shown.
4.  **Sizing and Layout:** Parameters like `valueSize` and `titleSize` adjust the size of the text elements, ensuring the component fits well within a graphical interface.

### How It Works (The Basic Idea)

Instead of manually programming every line of the knob's appearance, the code uses a function called `getJsonComponent`. This suggests the `KnobValue` is built from a pre-defined, standardized template (often stored as configuration data). This template uses the blueprint described above, allowing developers to quickly create consistent, highly customized knobs simply by choosing which settings to apply, without needing to change the core programming logic.

sha: 3ada4130e67d8c20dc1d9ea8be781eff4923707ee9eb8b09fa1c1388d8755907 
*/
import { getJsonComponent } from '../ui';

export const KnobValue = getJsonComponent<{
    audioPlugin: string;
    param: string;
    encoderId?: number;
    type?: string;
    label?: string;
    color?: string;
    bgColor?: string;
    textColor?: string;
    floatPrecision?: number;
    hideValue?: boolean;
    hideUnit?: boolean;
    unitSize?: number;
    valueSize?: number;
    titleSize?: number;
    valueReplaceTitle?: boolean;
    insideRadius?: number;
}>('KnobValue');
