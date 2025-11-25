export interface Props {
    name: string;
    position: [Number, Number];
}

export function Container(this: any, { name, position }: Props) {
    const children: [] = this.children;
    return [
        {
            __type: 'Container',
            name,
            position,
            components: (children || []).filter((child) => child).flat(Infinity),
        },
    ];
}
