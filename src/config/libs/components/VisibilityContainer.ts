import { getPosition } from '@/libs/ui';
import { ComponentProps } from './component';

export type Props = ComponentProps & {};

export function VisibilityContainer({ position, group, ...props }: Props) {
    return [
        { CONTAINER: 'VisibilityContainer ' + getPosition(position) },
        { VISIBILITY_GROUP: group },
        props,
    ];
}
