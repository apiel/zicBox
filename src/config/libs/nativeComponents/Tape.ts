import { getOldComponentToBeDeprecated, initializePlugin } from '@/libs/ui';
import { ComponentProps } from './component';

export type Props = ComponentProps & {
};

export function Tape({ bounds, track, ...props }: Props) {
    initializePlugin('Tape', 'libzic_TapeComponent.so');
    return getOldComponentToBeDeprecated('Tape', bounds, [{ track }, props]);
}
