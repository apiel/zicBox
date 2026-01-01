/** Description:
This TypeScript code defines the core blueprints and configuration options used to build and manage interactive components within an application's user interface, especially focusing on integration with external hardware controllers or specialized input methods.

### How the Code Works

The code establishes several structured data types (interfaces and an enumeration) that act as contracts for different parts of a UI element's definition:

1.  **Visibility Rules:** The `VisibilityContext` defines criteria for when a component should be displayed. It sets rules based on comparing a current input value (e.g., a fader position) against a target, using conditions like "show when over" or "show when under" a certain threshold.

2.  **Hardware Mapping:**
    *   The `KeypadLayout` defines how physical input (like a button on a controller) translates into an internal application command or "action." It links a specific key identity to the software function it should trigger.
    *   `ControllerColors` allows the application to control the color of lights on a connected hardware device, providing visual feedback to the user.

3.  **Resizing and Layout:** The `ResizeType` is a list of distinct options (like horizontal resize, vertical resize, width change, etc.). These options are stored using an efficient flag system, allowing a single number to specify multiple allowed resizing behaviors simultaneously.

4.  **The Master Blueprint:** The main definition, `ComponentProps`, combines all these settings into a complete configuration for any individual UI element. This blueprint includes the component's exact screen location (`bounds`), its allowed resizing options, its visibility requirements, and all associated hardware key mappings and color themes, ensuring the element functions correctly within the application environment.

Tags: Control Mapping, Input Devices, User Interface, Component Layout, System Configuration
sha: 0cfa71ad1a55b0d36d8b85512d720db0e33a97d1c7f987a72e5cc3e87f1703a2 
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
    extendEncoderIdArea?: number;
} & (P extends {}
    ? P
    : {
          [key: string]: any; // Allow random parameters if P is not specified
      });
