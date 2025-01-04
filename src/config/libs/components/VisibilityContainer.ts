import { getPosition } from '@/libs/ui';
import { ComponentProps } from './component';

export type Props = ComponentProps & {};

export function VisibilityContainer({ position, ...props }: Props) {
    return [{ CONTAINER: 'VisibilityContainer ' + getPosition(position) }, props];
}
