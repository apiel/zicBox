import { getComponent, initializePlugin, Position } from '@/libs/ui';

export interface Props {
    position: Position;
    plugin: string;
    values: string;
    [key: string]: any;
}

export function Adsr({ position, plugin, values, ...props }: Props) {
    initializePlugin('Adsr', 'libzic_AdsrComponent.so');
    return getComponent('Adsr', position, [
        { PLUGIN: plugin }, 
        { VALUES: values },
        props
    ]);
}
