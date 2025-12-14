/** Description:
This TypeScript file acts as a detailed blueprint for laying out the user interface (UI) on a screen, likely for a device that uses physical controls, such as an instrument or industrial equipment.

### Basic Idea

The core purpose of this code is to define precise rectangular areas, or "boxes," on the screen. Every interactive element, display area, or background section needs a designated space. Each box is defined by four numbers: its starting position across the screen, its starting position down from the top, its width, and its height.

### How the Layout Works

1.  **Screen Division:** The code first references the total screen dimensions (width and height) and uses predefined constants to divide the screen into quarters (like W1\_4) for easy placement of elements side-by-side.
2.  **Defining Boundaries:** Hundreds of constant variables are defined (e.g., `fullValues`, `topValues`, `bounds1`, `bounds2mini`). These lists specify the exact coordinates and size for every visual component, ensuring a consistent layout.
3.  **Control Mapping:** Many of these boundaries are linked to specialized objects called "encoders" (e.g., `enc1`, `enc2mini`). An encoder is typically a reference to a physical rotary knob or control dial on the device. By linking a specific encoder ID to a screen box, the system knows exactly where to display feedback when that physical control is adjusted.
4.  **Layout Variations:** The code provides multiple layout schemes—labeled 'mini,' 'smpl,' and 'Seq' (likely sequencer)—allowing the application to switch between different grid arrangements (e.g., displaying four large controls or twelve smaller ones) instantly.

### Helper Tools

The file also includes small utility functions that act as calculation recipes. These functions allow the system to easily adjust the predefined boxes without manually recalculating every number. For example, the `moveUp` function mathematically shifts an entire control box higher on the screen, while the `boundsOnTop` function creates a perfectly sized label area positioned exactly on the upper edge of an existing control box.

In summary, this file is the spatial map that dictates where all visual and interactive elements—from full-screen graphs to individual control knobs—must be located relative to each other on the device's display.

sha: 9db091c44c1bc4c0411178aa511e4d6a86aaa2e8708a0be43b5cdf840853ed51 
*/
import { W1_6, W2_6, W3_6, W4_6, W5_6 } from './constants';

export const top = 5;
export const graphHeight = 50;

export const encMargin = 30; // 40; <--- not 40 because we usually only show half of the graph
const topMini = top + 30;

export const bounds1mini = [0, topMini, W1_6 - 2, 30];
export const bounds2mini = [W1_6, topMini, W1_6 - 2, 30];
export const bounds3mini = [W2_6, topMini, W1_6 - 2, 30];
export const bounds4mini = [W3_6, topMini, W1_6 - 2, 30];
export const boundsAmini = [W4_6, topMini, W1_6 - 2, 30];
export const boundsBmini = [W5_6, topMini, W1_6 - 2, 30];

export const bounds5mini = [0, topMini + 34 + encMargin, W1_6 - 2, 30];
export const bounds6mini = [W1_6, topMini + 34 + encMargin, W1_6 - 2, 30];
export const bounds7mini = [W2_6, topMini + 34 + encMargin, W1_6 - 2, 30];
export const bounds8mini = [W3_6, topMini + 34 + encMargin, W1_6 - 2, 30];
export const boundsCmini = [W4_6, topMini + 34 + encMargin, W1_6 - 2, 30];
export const boundsDmini = [W5_6, topMini + 34 + encMargin, W1_6 - 2, 30];

export const bounds9mini = [0, topMini + 68 + encMargin + encMargin, W1_6 - 2, 30];
export const bounds10mini = [W1_6, topMini + 68 + encMargin + encMargin, W1_6 - 2, 30];
export const bounds11mini = [W2_6, topMini + 68 + encMargin + encMargin, W1_6 - 2, 30];
export const bounds12mini = [W3_6, topMini + 68 + encMargin + encMargin, W1_6 - 2, 30];
export const boundsEmini = [W4_6, topMini + 68 + encMargin + encMargin, W1_6 - 2, 30];
export const boundsFmini = [W5_6, topMini + 68 + encMargin + encMargin, W1_6 - 2, 30];

export const enc1mini = { encoderId: 1, bounds: bounds1mini };
export const enc2mini = { encoderId: 2, bounds: bounds2mini };
export const enc3mini = { encoderId: 3, bounds: bounds3mini };
export const enc4mini = { encoderId: 4, bounds: bounds4mini };
export const encAmini = { encoderId: 13, bounds: boundsAmini };
export const encBmini = { encoderId: 14, bounds: boundsBmini };

export const enc5mini = { encoderId: 5, bounds: bounds5mini };
export const enc6mini = { encoderId: 6, bounds: bounds6mini };
export const enc7mini = { encoderId: 7, bounds: bounds7mini };
export const enc8mini = { encoderId: 8, bounds: bounds8mini };
export const encCmini = { encoderId: 15, bounds: boundsCmini };
export const encDmini = { encoderId: 16, bounds: boundsDmini };

export const enc9mini = { encoderId: 9, bounds: bounds9mini };
export const enc10mini = { encoderId: 10, bounds: bounds10mini };
export const enc11mini = { encoderId: 11, bounds: bounds11mini };
export const enc12mini = { encoderId: 12, bounds: bounds12mini };
export const encEmini = { encoderId: 17, bounds: boundsEmini };
export const encFmini = { encoderId: 18, bounds: boundsFmini };

export function knob(enc: { encoderId: number; bounds: number[] }) {
    const i = 30;
    return {
        encoderId: enc.encoderId,
        bounds: [enc.bounds[0], enc.bounds[1] - i, enc.bounds[2], enc.bounds[3] + i],
    };
}
