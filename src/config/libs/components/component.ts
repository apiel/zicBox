import { Position } from '../ui';

export type ComponentProps = {
    position: Position;
    track?: number;
    GROUP?: number;
    [key: string]: any;
};
