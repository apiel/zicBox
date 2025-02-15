import { getJsonComponent } from '../ui';

export const HiddenValue = (props: {
    audioPlugin?: string;
    param?: string;
    encoderId?: number;
    inverted?: boolean;
}) =>
    getJsonComponent(
        'HiddenValue',
        'libzic_HiddenValueComponent.so'
    )({
        ...props,
        bounds: [0, 0, 0, 0],
    });
