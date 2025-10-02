import { ZicObj } from '@/libs/core';

export interface Props extends ZicObj {
    name: string;
    noPrevious?: boolean;
}

export function View(this: any, { name, noPrevious }: Props) {
    const children: [] = this.children;
    return [
        {
            name,
            ...noPrevious && { noPrevious: true },
            components: (children || []).filter((child) => child).flat(Infinity),
        },
    ];
}
