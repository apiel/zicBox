import { Bounds } from '../ui';

export type ComponentProps = {
    bounds: Bounds;
    track?: number;
    group?: number;
    [key: string]: any;
};
