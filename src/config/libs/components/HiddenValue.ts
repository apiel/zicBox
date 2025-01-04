import { getComponent } from '@/libs/ui';
import { ComponentProps } from './component';

export type Props = Omit<ComponentProps, 'position'> & {};

export function HiddenValue({ track, ...props }: Props = {}) {
    return getComponent('HiddenValue', [0, 0], [{ track }, props]);
}
