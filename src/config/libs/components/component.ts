import { Bounds } from '../ui';

export type ComponentProps<P = unknown> = {
    bounds: Bounds;
    group?: number;
} &
    (P extends {}
        ? P
        : {
              [key: string]: any; // Allow random parameters if P is not specified
          });
