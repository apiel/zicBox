import { Bounds, getOldComponentToBeDeprecated, initializePlugin } from '@/libs/ui';

export interface Props {
    bounds: Bounds;
    plugin: string;
    values: string;
    [key: string]: any;
}

export function Adsr({ bounds, plugin, values, ...props }: Props) {
    initializePlugin('Adsr', 'libzic_AdsrComponent.so');
    return getOldComponentToBeDeprecated('Adsr', bounds, [
        { PLUGIN: plugin }, 
        { VALUES: values },
        props
    ]);
}
