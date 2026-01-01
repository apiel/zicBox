/** Description:
This TypeScript code defines a specialized "factory function" named `HiddenValue`. Its primary role is to create a standardized blueprint for a user interface component that handles control logic or data storage, but which is deliberately kept invisible to the user.

**Purpose and Function:**

This function acts as a helper to configure settings that need to exist in the system architecture but should never appear on screen. By forcing its visual boundaries to zero, the component is effectively hidden. It is often used to map physical controls (like knobs or buttons) to software parameters.

**How it Works (The Blueprint):**

When you call `HiddenValue`, you can provide it with specific instructions (properties) that define its behavior:

1.  **Linking Controls:** You can specify which `audioPlugin` and software `param` (parameter) it should manipulate.
2.  **Hardware Connection:** It can be linked to a specific physical control device identifier (`encoderId`).
3.  **Visibility Logic:** It accepts complex rules that define *when* this hidden control should be active or referenced by the system (its `visibilityContext`).

The function gathers all these settings, wraps them into a standardized data format using the underlying UI system (`getJsonComponent`), and outputs a structured data packet ready for the program (likely a Lua-based environment) to process. This ensures control actions are handled reliably, even without a visible on-screen element.

sha: 5aaf011e578e57a922b41d0c9f2d52a1cf6a1f4034dd473070034ce48216762a 
*/
import { getJsonComponent } from '../ui';
import { ControllerColors, KeypadLayout, VisibilityContext, VisibilityData } from './component';

export const HiddenValue = (
    props: {
        audioPlugin?: string;
        param?: string;
        encoderId?: number;
        inverted?: boolean;
        visibilityContext?: VisibilityContext[];
        visibilityData?: VisibilityData[];
        visibilityGroup?: number;
        keys?: KeypadLayout[];
        controllerColors?: ControllerColors[];
    } = {} // Seems like we need to give a default value, else Lua cannot handle it...
) => getJsonComponent('HiddenValue')({ ...props, bounds: [0, 0, 0, 0] });
