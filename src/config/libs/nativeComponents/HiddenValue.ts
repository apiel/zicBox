import { getComponent, initializePlugin } from '@/libs/ui';
import { ComponentProps } from './component';

export type Props = Omit<ComponentProps, 'bounds'> & {};

export function HiddenValue({ track, ...props }: Props = {}) {
    initializePlugin('HiddenValue', 'libzic_HiddenValueComponent.so');
    return getComponent('HiddenValue', [0, 0], [{ track }, props]);
}
