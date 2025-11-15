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
