import { getComponent } from '@/libs/ui';
import { ComponentProps } from './component';

export type Props = ComponentProps & {
};

export function SampleEditor({ position, track, ...props }: Props) {
    return getComponent('SampleEditor', position, [{ track }, props]);
}
