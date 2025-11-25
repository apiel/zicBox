
import { Bounds, getBounds } from '../ui';

export interface Props {
    name: string;
    bounds: Bounds;
}

export function Container(this: any, { name, bounds }: Props) {
    const children: [] = this.children;
    return [
        {
            __type: 'Container',
            name,
            bounds: getBounds(bounds),
            components: (children || []).filter((child) => child).flat(Infinity),
        },
    ];
}
