import { getBounds, initializePlugin } from '@/libs/ui';
import { jsonStringify } from '../core';
import { ComponentProps } from './component';

export type Props = ComponentProps & {
};

export function Text({ bounds, track, ...props }: Props) {
    initializePlugin('Text', 'libzic_TextComponent.so');
    // return getComponent('Text', bounds, [{ track }, props]);

    const jsonConfig = jsonStringify({ 
        componentName: 'Text',
        bounds: getBounds(bounds), // TODO rename to getBounds
        ...props,
     });
     console.log('My config:', jsonConfig);

    return [{ COMPONENT: jsonConfig }];
}


// export function getComponent(name: string, position: Position, values: ZicValue[] = []) {
//     return [{ COMPONENT: `${name} ${getPosition(bounds)}` }, values];
// }