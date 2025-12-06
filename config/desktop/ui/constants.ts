/** Description:
This file acts as the central configuration blueprint for the entire application's user interface and internal control mappings. It does not contain any functional code that performs tasks, but rather defines a robust set of fixed, named values—called constants—that guarantee consistency across the software.

### How the Code Works

The core function of this file is to establish a shared set of rules for geometry, color, and behavior identifiers:

1.  **Layout and Geometry:** It sets the application's base dimensions, defining `ScreenWidth` and `ScreenHeight`. Crucially, it then uses these base dimensions to calculate proportional measurements (like quarters and eighths of the screen width). These proportions ensure that UI elements are positioned correctly and responsively, regardless of minor screen adjustments. Specific coordinates for key display areas, such as selector bars and information panels, are also defined here.

2.  **Color Palette:** A comprehensive color scheme is established. This includes easy-to-read names for primary, secondary, and tertiary interface bars, as well as distinct colors for specific data streams or tracks (e.g., `ColorTrackMaster`, `ColorTrack1`, etc.). This guarantees a consistent and predictable visual appearance.

3.  **Internal Mapping:** The file defines abstract identifiers and internal codes. These include numeric codes for managing different user interaction states (known as "contexts," such as `shiftContext` or `shutdownContext`) and alphanumeric constants that likely map to specific keyboard shortcuts or control buttons within the application's interface.

By exporting all these constants, the application ensures that every component drawing a shape or checking a state uses the exact same foundational values.

sha: c8b8cb258596c78bff10e1e0bccdf1eb18d20859bba0ec5dc9982b2ece84b495 
*/
import { rgb } from '@/libs/ui';

export * from '../constants';

// 1540 × 720 
// export const ScreenWidth = 1540;
// export const ScreenHeight = 720;

//  1280 x 800
// export const ScreenWidth = 1280;
// export const ScreenHeight = 800;

// 1920 x 1200 
export const ScreenWidth = 1920 / 2;
export const ScreenHeight = 1200 / 2;

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
