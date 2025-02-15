
export type Props = {
    condition: 'SHOW_WHEN_NOT' | 'SHOW_WHEN_OVER' | 'SHOW_WHEN_UNDER' | 'SHOW_WHEN';
    index: number;
    value: number;
};

export function VisibilityContext({ condition, index, value }: Props) {
    return [{ VISIBILITY_CONTEXT: `${index} ${condition} ${value}` }];
}
