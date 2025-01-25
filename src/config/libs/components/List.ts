import { getComponent, initializePlugin } from '@/libs/ui';
import { ComponentProps } from './component';

export type Props = ComponentProps & {
    items: string[];
};

export function List({ position, items, ...props }: Props) {
    initializePlugin('List', 'libzic_ListComponent.so');
    return getComponent('List', position, [items.map((item) => ({ ADD_ITEM: item })), props]);
}
