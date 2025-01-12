import { PrimaryBar, QuaternaryBar, ScreenWidth, SecondaryBar, TertiaryBar, W2_4 } from '@/pixel/240x320/ui/constants';

// TODO clean me up!!

export const valueH = 29;
export const rowSpacing = 8;
export const height = 65;
export const posY = 84;
export const encoderH = 50;

export const topLeft = [0, 0, W2_4 - 2, valueH];
export const bottomLeft = [0, valueH + rowSpacing, W2_4 - 2, valueH];
export const topRight = [W2_4, 0, W2_4 - 2, valueH];
export const bottomRight = [W2_4, valueH + rowSpacing, W2_4 - 2, valueH];

export const posContainerValues = [0, posY, ScreenWidth, valueH * 2 + 2];
export const posTopContainer = [0, 4, ScreenWidth, 70];

const base = {
    SHOW_LABEL_OVER_VALUE: 6,
    LABEL_FONT_SIZE: 8,
};

export const primary = {
    LABEL_COLOR: 'primary',
    BAR_COLOR: PrimaryBar,
    ...base,
};

export const secondary = {
    LABEL_COLOR: 'secondary',
    BAR_COLOR: SecondaryBar,
    ...base,
};

export const tertiary = {
    LABEL_COLOR: 'tertiary',
    BAR_COLOR: TertiaryBar,
    ...base,
};

export const quaternary = {
    LABEL_COLOR: 'quaternary',
    BAR_COLOR: QuaternaryBar,
    ...base,
};
