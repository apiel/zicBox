import { getJsonComponent } from '../ui';
import { ControllerColors, KeypadLayout, VisibilityContext } from './component';

export const KnobAction = (
    props: {
        action: string;
        action2?: string;
        encoderId?: number;
        visibilityContext?: VisibilityContext[];
        visibilityGroup?: number;
        keys?: KeypadLayout[];
        controllerColors?: ControllerColors[];
    }
) => getJsonComponent('KnobAction')({ ...props, bounds: [0, 0, 0, 0] });
