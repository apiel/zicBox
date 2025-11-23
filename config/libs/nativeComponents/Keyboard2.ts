/** Description:
This code defines a highly configurable, reusable component designed for use within a user interface: a virtual input mechanism labeled "Keyboard2."

**Core Functionality**

The main purpose of this snippet is to establish a standardized blueprint for this virtual keyboard. It utilizes a specialized helper function that loads the component's visual layout and behavior from an external configuration file (likely a JSON file). This separation ensures that developers can easily update the keyboard's appearance without modifying the underlying programming logic.

**Blueprint and Customization**

The extensive list of properties specified in the code acts as a contract, ensuring that the component is used correctly elsewhere in the application. These properties represent all the customizable settings available for the keyboard.

Key customization options include:

1.  **Appearance:** Settings like `bgColor` (background color), `textColor`, and `font` control the visual style.
2.  **Behavior:** Options such as `redirectView` dictate what screen or area should be updated when the component is interacted with.
3.  **Content:** The most critical settings are `keyLabels` and `keyLabelsShifted`. These define the specific characters displayed on the keys in their default state and when a shift function is active.

In essence, this code creates a robust, standardized template for a virtual keyboard, allowing developers to easily embed it into different parts of an application while maintaining strict control over its visual design and operational capabilities.

sha: 7523c05c2d0eaf88cd34ca6ccd31cfc36a86299b38eca7de6d39586ead5809ba 
*/
import { getJsonComponent } from '../ui';

export const Keyboard2 = getJsonComponent<{
    redirectView?: string;
    audioPlugin?: string;
    dataId?: string;
    bgColor?: string;
    textColor?: string;
    selectionColor?: string;
    itemBackground?: string;
    font?: string;
    shiftContextId?: number;
    keyLabels: string[];
    keyLabelsShifted: string[];
    keyColors?: string[];
}>('Keyboard2');
