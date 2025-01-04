import { Position } from '../ui';

export type ComponentProps = {
    position: Position;
    track?: number;
    group?: number;
    [key: string]: any;
};
