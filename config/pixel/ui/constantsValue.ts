import { ScreenWidth, W1_4, W2_4 } from './constants';

const top = 20;
export const fullValues = [0, top, ScreenWidth - 1, 180];
export const halfFullValues = [0, top + fullValues[3] / 4, ScreenWidth - 1, fullValues[3] / 2];
export const topValues = [0, top, ScreenWidth - 1, 80];
export const bottomValues = [0, top + 100, ScreenWidth - 1, 80];
export const topLeftKnob = [0, top, W2_4 - 2, 100];
export const topRightKnob = [W2_4, top, W2_4 - 2, 100];
export const bottomLeftKnob = [0, top + 100, W2_4 - 2, 100];
export const bottomRightKnob = [W2_4, top + 100, W2_4 - 2, 100];

export const bounds1 = [0, top, W1_4 - 2, 80];
export const bounds2 = [W1_4, top, W1_4 - 2, 80];
export const bounds3 = [W1_4 * 2, top, W1_4 - 2, 80];
export const bounds4 = [W1_4 * 3, top, W1_4 - 2, 80];

export const backgroundBounds = [0, top, ScreenWidth, top + 100];

export const enc1 = { encoderId: 1, bounds: bounds1 };
export const enc2 = { encoderId: 2, bounds: bounds2 };
export const enc3 = { encoderId: 3, bounds: bounds3 };
export const enc4 = { encoderId: 4, bounds: bounds4 };

export const graphTopValues = [0, top + 20, ScreenWidth - 1, 60];
export const graphCenterValues = [0, top + 60, ScreenWidth - 1, 80];
export const graphBottomValues = [0, top + 120, ScreenWidth - 1, 60];

export const graphBounds = [0, top + 50, ScreenWidth - 1, 70];

export function boundsOnTop(bounds: number[], height: number = 22, margin: number = 10) {
    return [bounds[0], bounds[1] + margin, bounds[2], height];
}

export function boundsOnBottom(bounds: number[], height: number = 22) {
    return [bounds[0], bounds[1] + bounds[3] - height, bounds[2], height];
}

export function encTopValue(enc: { encoderId: number; bounds: number[] }, height: number = 30, margin: number = 10) {
    return { ...enc, bounds: boundsOnTop(enc.bounds, height, margin) };
}

export function encBottomValue(enc: { encoderId: number; bounds: number[] }, height: number = 30) {
    return { ...enc, bounds: boundsOnBottom(enc.bounds, height) };
}

export function boundsMarginTop(bounds: number[], margin: number = 10) {
    return [bounds[0], bounds[1] + margin, bounds[2], bounds[3]];
}

// export const seqCardBounds_small = [100, 440, 280, 160];
export const seqCardBounds_small = [ScreenWidth - 130, 105, 120, 70];

export const seqContextDrum1 = 50;
export const seqContextDrum2 = 51;
export const seqContextDrum3 = 52;
export const seqContextDrum4 = 53;

export const seqContextSample1 = 57;
export const seqContextSample2 = 58;
export const seqContextSample3 = 59;
export const seqContextSample4 = 60;
