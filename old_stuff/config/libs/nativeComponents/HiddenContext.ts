/** Description:
This code defines a specialized software function designed to create configurations for an invisible user interface component called a "Hidden Context."

### Core Purpose

This function acts as a sophisticated blueprint builder. Its primary role is not to draw anything on the screen, but to establish the rules and logic that govern other visible parts of an interface, such as keypads, encoders, or data displays.

### How It Works

1.  **Configuration Collection:** The function accepts a variety of settings (called "properties"). These properties dictate operational behavior, including:
    *   Unique identifiers (like `encoderId` or `contextId`).
    *   Value constraints, such as minimum (`min`) and maximum (`max`) limits for adjustable inputs.
    *   Rules defining which parts of the keypad or interface should be visible under specific circumstances (`visibilityContext`).
2.  **Data Structuring:** After collecting these settings, the function calls a dedicated utility (a `getJsonComponent` helper). This helper takes all the input parameters and packages them into a standardized, machine-readable format—often a structure similar to JSON.
3.  **Invisible Logic:** Crucially, the code explicitly sets the display boundaries (size and position) to zero. This confirms that the resulting "Hidden Context" component is purely for managing background state, logic, and configuration rules, ensuring it never appears visually to the end user.

In summary, this component is the infrastructure manager, setting up the necessary logical definitions that the main application engine uses to control input devices and visibility rules behind the scenes.

sha: bead69ec386ce655d1b5faaa5c52d3d2ee992161706df815c2bd5ac311bb52a6 
*/
import { getJsonComponent } from '../ui';
import { KeypadLayout, VisibilityContext } from './component';

export const HiddenContext = (
    props: {
        encoderId?: number;
        inverted?: boolean;
        min?: number;
        max?: number;
        stepIncrementation?: number;
        contextId?: number;
        visibilityContext?: VisibilityContext[];
        visibilityGroup?: number;
        keys?: KeypadLayout[];
    } = {} // Seems like we need to give a default value, else Lua cannot handle it...
) => getJsonComponent('HiddenContext')({ ...props, bounds: [0, 0, 0, 0] });
