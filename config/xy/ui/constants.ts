/** Description:
This TypeScript file serves as the foundational settings and configuration map for a user interface, likely belonging to an embedded system, music sequencer, or specialized control device. It functions entirely by defining fixed values, known as constants, that standardize the application's appearance and behavior.

The basic idea is to create a single source of truth for all crucial properties—from screen measurements to color schemes—ensuring the application remains consistent.

**How the Code Works:**

1.  **Defining Dimensions:** The file establishes the precise working size of the screen (320 units wide by 240 units high). It then pre-calculates common layout measurements, such as dividing the width into halves, quarters, sixths, and eighths. This simplifies the process of positioning graphical elements correctly. It also defines specific coordinates for complex UI components, such as selectors and key information displays.

2.  **Color Palette:** A comprehensive color scheme is defined using numerical values for various elements. This includes colors for status bars (`PrimaryBar`), generic buttons, and specific elements associated with musical tracks (e.g., `ColorTrackMaster`, `ColorTrack1` through `Track10`).

3.  **Control and Mapping:** The file assigns unique IDs to physical controls (like four different rotary knobs or encoders) and maps internal actions to specific keyboard keys (using constants like A1, B2, etc.).

4.  **Application States (Contexts):** Crucially, the file defines numeric IDs that represent specific application modes or states (e.g., `shiftContext`, `shutdownContext`). These unique numbers act as flags that tell the application what it should be doing.

5.  **Visibility Rules:** These state IDs are then used to create complex visibility rules. For instance, rules are defined that determine whether a part of the interface should be shown *only* when the "Shift" context is active, or hidden when it is not. This controls the dynamic appearance and disappearance of menus and buttons.

Tags: Music Sequencing, Interface Layout, Visual Design, Control Mapping, Screen Sizing
sha: 75656c2ad7da07432f63c3ad97cf708a06e4f2a04587816dd1ebaa47d7d3ce2a 
*/
import { VisibilityContext } from '@/libs/nativeComponents/component';
import { rgb } from '@/libs/ui';

export * from '../constants';

// 1540 × 720
// export const ScreenWidth = 1540;
// export const ScreenHeight = 720;

//  1280 x 800
// export const ScreenWidth = 1280;
// export const ScreenHeight = 800;

// 1920 x 1200
export const ScreenWidth = 320;
export const ScreenHeight = 240;

export const W1_4 = ScreenWidth / 4;
export const W2_4 = W1_4 * 2;
export const W1_2 = W2_4;
export const W3_4 = W1_4 * 3;

export const W1_8 = ScreenWidth / 8;
export const W2_8 = W1_8 * 2;
export const W3_8 = W1_8 * 3;
export const W4_8 = W1_8 * 4;
export const W5_8 = W1_8 * 5;
export const W6_8 = W1_8 * 6;
export const W7_8 = W1_8 * 7;
export const W8_8 = W1_8 * 8;

export const W1_6 = ScreenWidth / 6;
export const W2_6 = W1_6 * 2;
export const W3_6 = W1_6 * 3;
export const W4_6 = W1_6 * 4;
export const W5_6 = W1_6 * 5;
export const W6_6 = W1_6 * 6;

export const KeyTopInfoPosition = [140, 284, 100, 12];

export const KeyInfoPosition = [0, 296 + 320, ScreenWidth];
export const SelectorPosition = [0, 296 + 320, ScreenWidth];
export const SelectorPosition2 = [0, 308 + 320, ScreenWidth];

export const PrimaryBar = '#377275';
export const SecondaryBar = '#b54f5e';
export const TertiaryBar = '#368a36';
export const QuaternaryBar = '#94854b';

export const ColorTrackMaster = rgb(255, 74, 74);
export const ColorTrack1 = rgb(163, 201, 18);
export const ColorTrack2 = rgb(253, 111, 14);
export const ColorTrack3 = rgb(21, 165, 202);
export const ColorTrack4 = rgb(255, 205, 4);
export const ColorTrack5 = rgb(181, 79, 94);
export const ColorTrack6 = rgb(55, 114, 117);
export const ColorTrack7 = rgb(98, 79, 181);
export const ColorTrack8 = rgb(181, 79, 173);
export const ColorTrack9 = rgb(4, 196, 180);
export const ColorTrack10 = rgb(245, 176, 239);

export const ColorButton = rgb(50, 50, 50);
export const ColorBtnOff = rgb(0, 0, 0);

export const menuTextColor = rgb(110, 110, 110);

export const encTopLeft = 0;
export const encTopRight = 1;
export const encBottomLeft = 2;
export const encBottomRight = 3;

export const A1 = 'q';
export const A2 = 'w';
export const A3 = 'e';
export const A4 = 'r';
export const A5 = 't';
export const A6 = 'y';
export const A7 = 'u';
export const A8 = 'i';
export const A9 = 'o';

export const B1 = 'a';
export const B2 = 's';
export const B3 = 'd';
export const B4 = 'f';
export const B5 = 'g';
export const B6 = 'h';
export const B7 = 'j';
export const B8 = 'k';
export const B9 = 'l';

export const C1 = 'z';
export const C2 = 'x';
export const C3 = 'c';
export const C4 = 'v';

export const shiftContext = 254;
export const clipRenderContext = 200;
export const seqContext = 201;
export const shutdownContext = 202;
export const updateContext = 203;
export const githubContext = 204;
export const githubContext2 = 205;
export const trackContextId = 206;
export const stepContextId = 207;
export const viewStepStartContextId = 208;

export const unshiftVisibilityContext: VisibilityContext = {
    condition: 'SHOW_WHEN',
    index: shiftContext,
    value: 0,
};

export const shiftVisibilityContext: VisibilityContext = {
    condition: 'SHOW_WHEN',
    index: shiftContext,
    value: 1,
};

export const notUnshiftVisibilityContext: VisibilityContext = {
    condition: 'SHOW_WHEN_NOT',
    index: shiftContext,
    value: 0,
};
