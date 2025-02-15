import { Keymap, Props as KeymapProps } from './Keymap';

export type Props = {
    keys: KeymapProps[];
};

export function Keymaps({ keys }: Props) {
    return keys.map(key => Keymap(key));
}
