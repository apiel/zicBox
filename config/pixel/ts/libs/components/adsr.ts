import { getComponent, Position } from '../ui';

export interface Props {
    position: Position;
    plugin: string;
    values: string;
    [key: string]: any;
}

export function Adsr({ position, plugin, values, ...props }: Props) {
    return getComponent('Adsr', position, [
        { PLUGIN: plugin }, 
        { VALUES: values },
        props
    ]);
}
