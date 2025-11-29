/** Description:
This TypeScript code establishes the fundamental blueprints for defining and controlling customizable interactive elements—like buttons, gauges, or widgets—within a software application. It acts as a standardized checklist of configurations that every graphical component must adhere to.

The code defines several key structures to manage behavior:

1.  **Visibility Rules:** This configuration determines exactly when an element should be seen. It uses specific conditions (like "show when value is over X" or "show when value is under Y") to make elements appear or disappear dynamically based on the application's state.

2.  **Key Mapping:** This structure serves as the layout for controlling the component, translating key presses (from a physical device or virtual keyboard) into specific software actions. It maps a key identifier to an instruction (e.g., "Key 5 means 'Start Playback'"). It also allows defining custom color themes for these interactive keys.

3.  **Resizing Control:** A dedicated list of flags defines *how* an element can be resized by a user. For instance, it specifies if the element can only be stretched horizontally, only vertically, or not resized at all.

Finally, the central definition, `ComponentProps`, combines all these individual rules—including the component's size, location, visibility logic, key mappings, and resizing permissions—into one cohesive package. This standardized package ensures that the system can properly render, control, and integrate every interactive piece of the user interface.

sha: a7f36d3645c6dec168b15f67a116a7300a4b3601b228109678652156e789fb97 
*/
import { Bounds } from '../ui';

export interface VisibilityContext {
    index: number;
    condition: 'SHOW_WHEN_NOT' | 'SHOW_WHEN_OVER' | 'SHOW_WHEN_UNDER' | 'SHOW_WHEN';
    value: number;
}

export interface KeypadLayout {
    controller?: string;
    key: string | number;
    action: string;
    action2?: string;
    context?: {
        id: number;
        value: number;
    };
    multipleKeyHandler?: boolean;
}

export interface ControllerColors {
    controller: string;
    colors: {
        key: number | string;
        color: string;
    }[];
}

export enum ResizeType {
    RESIZE_NONE = 1 << 0, // 1
    RESIZE_X = 1 << 1, // 2
    RESIZE_Y = 1 << 2, // 4
    RESIZE_W = 1 << 3, // 8
    RESIZE_H = 1 << 4, // 16
}

export const RESIZE_ALL = ResizeType.RESIZE_X | ResizeType.RESIZE_Y | ResizeType.RESIZE_W | ResizeType.RESIZE_H;

export type ComponentProps<P = unknown> = {
    bounds: Bounds;
    track?: number;
    resizeType?: number;
    visibilityContext?: VisibilityContext[];
    visibilityGroup?: number; // TODO to be implemented
    keys?: KeypadLayout[];
    pluginPath?: string;
    controllerColors?: ControllerColors[];
} & (P extends {}
    ? P
    : {
          [key: string]: any; // Allow random parameters if P is not specified
      });
