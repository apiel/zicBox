import { ScreenWidth, W2_4 } from '@/pixel/240x320/ui/constants';

const top = 60;
export const fullValues = [0, top, ScreenWidth - 1, 180];
export const topValues = [0, top, ScreenWidth - 1, 80];
export const bottomValues = [0, top + 100, ScreenWidth - 1, 80];
export const topLeftKnob = [0, top, W2_4 - 2, 80];
export const topRightKnob = [W2_4, top, W2_4 - 2, 80];
export const bottomLeftKnob = [0, top + 100, W2_4 - 2, 80];
export const bottomRightKnob = [W2_4, top + 100, W2_4 - 2, 80];
