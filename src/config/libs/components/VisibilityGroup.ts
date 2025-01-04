
export type Props = {
    condition: 'SHOW_WHEN_NOT' | 'SHOW_WHEN_OVER' | 'SHOW_WHEN_UNDER' | 'SHOW_WHEN';
    group: number;
};

export function VisibilityGroup({ condition, group }: Props) {
    return [{ VISIBILITY_GROUP: `${condition} ${group}` }];
}
