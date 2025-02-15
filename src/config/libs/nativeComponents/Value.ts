import { getJsonComponent } from '../ui';

export const Value = getJsonComponent<{
    audioPlugin: string;
    param: string;
    encoderId?: number;
    label?: string;
    floatPrecision?: number;
    useStringValue?: boolean;
    barHeight?: number;
    barBgHeight?: number;
    verticalAlignCenter?: boolean;
    hideLabel?: boolean;
    hideUnit?: boolean;
    hideValue?: boolean;
    showLabelOverValue?: number;
    labelOverValueX?: number;
    valueSize?: number;
    labelSize?: number;
    unitSize?: number;
    font?: string;
    valueHeight?: number;
    bgColor?: string;
    labelColor?: string;
    valueColor?: string;
    barColor?: string;
    unitColor?: string;

}>('Value', 'libzic_ValueComponent.so');
