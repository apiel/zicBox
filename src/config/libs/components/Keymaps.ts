import { initializePlugin } from '../ui';
import { Keymap, Props as KeymapProps } from './Keymap';

export type Props = {
    keys: KeymapProps[];
};

export function Keymaps({ keys }: Props) {
    initializePlugin('Keymaps', 'libzic_KeymapsComponent.so');
    return keys.map(key => Keymap(key));
}
