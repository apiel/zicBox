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
    context? : {
        id: number;
        value: number
    },
    multipleKeyHandler?: boolean
}

export interface ControllerColors {
    controller: string;
    colors: {
        key: number | string;
        color: string;
    }[];
}

export type ComponentProps<P = unknown> = {
    bounds: Bounds;
    group?: number;
    track?: number;
    visibilityContext?: VisibilityContext[];
    visibilityGroup?: number; // TODO to be implemented
    keys?: KeypadLayout[];
    pluginPath?: string;
    controllerColors?: ControllerColors[];
} &
    (P extends {}
        ? P
        : {
              [key: string]: any; // Allow random parameters if P is not specified
          });
