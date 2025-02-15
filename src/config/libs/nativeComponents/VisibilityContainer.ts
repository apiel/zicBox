import { getPosition } from '@/libs/ui';
import { ComponentProps } from './component';

export type Props = ComponentProps & {};

export function VisibilityContainer({ bounds, group, ...props }: Props) {
    return [
        { CONTAINER: 'VisibilityContainer ' + getPosition(bounds) },
        { VISIBILITY_GROUP: group },
        props,
    ];
}
