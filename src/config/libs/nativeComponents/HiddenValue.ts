import { getJsonComponent } from '../ui';
import { VisibilityContext } from './component';

export const HiddenValue = (props: {
    audioPlugin?: string;
    param?: string;
    encoderId?: number;
    inverted?: boolean;
    visibilityContext?: VisibilityContext[];
} = {}) => // Seems like we need to give a default value, else Lua cannot handle it...
    getJsonComponent(
        'HiddenValue',
        'libzic_HiddenValueComponent.so'
    )({
        ...props,
        bounds: [0, 0, 0, 0],
    });
