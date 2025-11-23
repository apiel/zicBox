/** Description:
This script defines and prepares a fundamental building block for a software application, specifically a component named the `NoteGrid`. This grid is likely used within a music creation, sequencing, or synthesizer interface.

The main purpose of this code is to register the `NoteGrid` component using a standardized system utility (`getJsonComponent`). This utility allows the application to quickly recognize and load the component's definition, often from a simple configuration or JSON file, making the system flexible and easy to update.

The `NoteGrid` itself acts like a customizable electronic music pad or matrix. The definition specifies all the ways a user or developer can customize how that grid looks and behaves when implemented:

*   **Structure and Layout:** You can define the exact size of the grid by setting the number of columns (`cols`) and rows (`rows`).
*   **Sound Control:** It allows linking the grid to a specific sound generator or effect using an `audioPlugin`.
*   **Musical Settings:** The code includes various controls (`scaleIndex`, `octave`, `encScale`, `encOctave`) necessary for defining the specific musical scale or range of notes the grid will play.
*   **Orientation:** It includes an option (`inverted`) to potentially reverse or mirror the grid's visual layout or functional mapping.

In short, this small piece of code formally introduces a highly configurable and essential musical interface element to the application's framework.

sha: bd0cf93c590f37c400b402c28a9e17dcfd011e4a33f3d07c02a812e26882488a 
*/
import { getJsonComponent } from '../ui';

export const NoteGrid = getJsonComponent<{
    audioPlugin?: string;
    cols?: number;
    rows?: number;
    scaleIndex?: number;
    octave?: number;
    encScale?: number;
    encOctave?: number;
    inverted?: boolean;
}>('NoteGrid');
