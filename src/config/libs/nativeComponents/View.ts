import { ZicObj } from '@/libs/core';

export interface Props extends ZicObj {
    name: string;
}

export function View(this: any, { name }: Props) {
    const children: [] = this.children;
    return [
        {
            view: {
                name,
                components: (children || [])
                    .filter((child) => child)
                    .flat(Infinity),
            },
        },
    ];
}
