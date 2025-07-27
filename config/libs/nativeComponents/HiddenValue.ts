import { getJsonComponent } from '../ui';
import { ControllerColors, KeypadLayout, VisibilityContext } from './component';

export const HiddenValue = (
    props: {
        audioPlugin?: string;
        param?: string;
        encoderId?: number;
        inverted?: boolean;
        visibilityContext?: VisibilityContext[];
        visibilityGroup?: number;
        keys?: KeypadLayout[];
        controllerColors?: ControllerColors[];
    } = {} // Seems like we need to give a default value, else Lua cannot handle it...
) => getJsonComponent('HiddenValue')({ ...props, bounds: [0, 0, 0, 0] });
