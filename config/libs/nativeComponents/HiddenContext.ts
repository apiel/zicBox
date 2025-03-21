import { getJsonComponent } from '../ui';
import { KeypadLayout, VisibilityContext } from './component';

export const HiddenContext = (
    props: {
        encoderId?: number;
        inverted?: boolean;
        min?: number;
        max?: number;
        stepIncrementation?: number;
        contextId?: number;
        visibilityContext?: VisibilityContext[];
        visibilityGroup?: number;
        keys?: KeypadLayout[];
    } = {} // Seems like we need to give a default value, else Lua cannot handle it...
) => getJsonComponent('HiddenContext')({ ...props, bounds: [0, 0, 0, 0] });
