import { Bounds } from '../ui';

export interface VisibilityContext {
    index: number;
    condition: 'SHOW_WHEN_NOT' | 'SHOW_WHEN_OVER' | 'SHOW_WHEN_UNDER' | 'SHOW_WHEN';
    value: number;
}

export type ComponentProps<P = unknown> = {
    bounds: Bounds;
    group?: number;
    track?: number;
    visibilityContext?: VisibilityContext[];
} &
    (P extends {}
        ? P
        : {
              [key: string]: any; // Allow random parameters if P is not specified
          });
