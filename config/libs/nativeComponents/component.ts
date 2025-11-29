/** Description:
This collection of definitions acts as the fundamental blueprint or data structure for creating customizable, interactive elements (components) within a larger software system. It dictates all the standard settings and behaviors these screen elements can possess.

**Core Function:**
The code defines the complete set of properties (`ComponentProps`) required for any visual element, covering its size, its interaction controls, and its display rules.

**Key Settings Defined:**

1.  **Visibility Rules (`VisibilityContext`):** These settings determine *when* a component should be displayed. For example, a button might only appear if a linked system value is "over," "under," or "equal to" a specific number.

2.  **Control Mapping (`KeypadLayout`):** This defines how physical or virtual keys (like on a keyboard or a controller) are linked to actions within the application. It specifies which key performs which primary or secondary action.

3.  **Color Customization (`ControllerColors`):** This structure allows developers to define unique color schemes for keys or controls associated with a specific hardware or software controller.

4.  **Resizing Options (`ResizeType`):** This defines exactly how a component can be adjusted by the user. It allows individual options for resizing horizontally (X), vertically (Y), adjusting width (W), or adjusting height (H). These options can be combined (like `RESIZE_ALL`) to allow full manipulation of the element's size and position.

In essence, this code provides a standard, organized way to configure everything from a simple button's color and visibility to a complex panel's size and connected key functions.

sha: 422442fe99544ea82ef6b71495a7bfc7e30336f4fd51b0ce0230a810998b4480 
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
