import { ScreenWidth, W2_4 } from './constants';

const top = 60;
export const fullValues = [0, top, ScreenWidth - 1, 180];
export const halfFullValues = [0, top + fullValues[3] / 4, ScreenWidth - 1, fullValues[3] / 2];
export const topValues = [0, top, ScreenWidth - 1, 80];
export const bottomValues = [0, top + 100, ScreenWidth - 1, 80];
export const topLeftKnob = [0, top, W2_4 - 2, 100];
export const topRightKnob = [W2_4, top, W2_4 - 2, 100];
export const bottomLeftKnob = [0, top + 100, W2_4 - 2, 100];
export const bottomRightKnob = [W2_4, top + 100, W2_4 - 2, 100];

export const graphTopValues = [0, top + 20, ScreenWidth - 1, 60];
export const graphCenterValues = [0, top + 60, ScreenWidth - 1, 80];
export const graphBottomValues = [0, top + 120, ScreenWidth - 1, 60];

