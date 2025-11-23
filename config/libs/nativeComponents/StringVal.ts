/** Description:
This file serves as a blueprint definition for a specific user interface element intended for software applications.

### Core Functionality

The main purpose of this code is to define and export a standardized, reusable software component named `StringVal`. This component is designed to manage, display, and configure textual data or string values, typically in complex systems like audio processing software or configuration panels.

### How it Works (The Blueprint)

Instead of writing the entire component from scratch, the code utilizes a specialized builder function called `getJsonComponent`. This function acts like a "factory," taking a name ('StringVal') and a detailed structure definition to instantly create the component.

The structure definition dictates what information the `StringVal` component must handle and what its capabilities are:

1.  **Mandatory Settings:** It requires information about the underlying system it controls, specifically which `audioPlugin` and which specific `param` (parameter) within that plugin it is linked to.
2.  **Optional Settings:** It allows developers to customize its appearance and display characteristics, such as `bgColor` (background color), `labelColor`, the size of its labels (`labelSize`), and whether it displays a `unit` indicator.

By exporting `StringVal`, this standardized text-handling interface component can be easily plugged into different parts of the overall application without needing to redefine its fundamental structure every time.

sha: 01fc5b440ca520bae119497f461e34f336b41655f45acf2d104c88acc4626c69 
*/
import { getJsonComponent } from '../ui';

export const StringVal = getJsonComponent<{
    audioPlugin: string;
    param: string;
    labelSize?: number;
    fontLabel?: string;
    bgColor?: string;
    labelColor?: string;
    unit?: boolean;
}>('StringVal');
