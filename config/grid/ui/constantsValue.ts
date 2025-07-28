import { ScreenHeight, ScreenWidth, W1_4, W2_4 } from './constants';

const top = 70;
export const fullValues = [0, top, ScreenWidth - 1, 180];
export const halfFullValues = [0, top + fullValues[3] / 4, ScreenWidth - 1, fullValues[3] / 2];
export const topValues = [0, top, ScreenWidth - 1, 80];
export const bottomValues = [0, top + 100, ScreenWidth - 1, 80];
export const topLeftKnob = [0, top, W2_4 - 2, 100];
export const topRightKnob = [W2_4, top, W2_4 - 2, 100];
export const bottomLeftKnob = [0, top + 100, W2_4 - 2, 100];
export const bottomRightKnob = [W2_4, top + 100, W2_4 - 2, 100];

export const backgroundBounds = [0, top, ScreenWidth, ScreenHeight - top];

export const bounds1 = [0, top, W1_4 - 2, 100];
export const bounds2 = [W1_4, top, W1_4 - 2, 100];
export const bounds3 = [W1_4 * 2, top, W1_4 - 2, 100];
export const bounds4 = [W1_4 * 3, top, W1_4 - 2, 100];

export const bounds5 = [0, top + 120, W1_4 - 2, 100];
export const bounds6 = [W1_4, top + 120, W1_4 - 2, 100];
export const bounds7 = [W1_4 * 2, top + 120, W1_4 - 2, 100];
export const bounds8 = [W1_4 * 3, top + 120, W1_4 - 2, 100];

export const bounds9 = [0, top + 240, W1_4 - 2, 100];
export const bounds10 = [W1_4, top + 240, W1_4 - 2, 100];
export const bounds11 = [W1_4 * 2, top + 240, W1_4 - 2, 100];
export const bounds12 = [W1_4 * 3, top + 240, W1_4 - 2, 100];

export const enc1 = { encoderId: 1, bounds: bounds1 };
export const enc2 = { encoderId: 2, bounds: bounds2 };
export const enc3 = { encoderId: 3, bounds: bounds3 };
export const enc4 = { encoderId: 4, bounds: bounds4 };

export const enc5 = { encoderId: 5, bounds: bounds5 };
export const enc6 = { encoderId: 6, bounds: bounds6 };
export const enc7 = { encoderId: 7, bounds: bounds7 };
export const enc8 = { encoderId: 8, bounds: bounds8 };

export const enc9 = { encoderId: 9, bounds: bounds9 };
export const enc10 = { encoderId: 10, bounds: bounds10 };
export const enc11 = { encoderId: 11, bounds: bounds11 };
export const enc12 = { encoderId: 12, bounds: bounds12 };

export const graphTopValues = [0, top + 20, ScreenWidth - 1, 60];
export const graphCenterValues = [0, top + 60, ScreenWidth - 1, 80];
export const graphBottomValues = [0, top + 120, ScreenWidth - 1, 60];

export function graphBounds(topLeft: number[], margin: number | [number, number] = 60, cols: number = 2, rows: number = 2) {
    const marginTop = typeof margin === 'number' ? margin : margin[0];
    const marginBottom = typeof margin === 'number' ? margin : margin[1];
    return [topLeft[0], topLeft[1] + marginTop - 10, ScreenWidth / cols,  120 * rows - marginBottom * 2];
}

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
export const seqCardBounds_small = [120, 450, 240, 140];

export const seqContextDrum1 = 50;
export const seqContextDrum2 = 51;
export const seqContextDrum3 = 52;
export const seqContextDrum4 = 53;

export const seqContextSynth1 = 54;
export const seqContextSynth2 = 55;
export const seqContextSynth3 = 56;
