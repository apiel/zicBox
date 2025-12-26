import { ScreenHeight, ScreenWidth, W1_4, W2_4 } from './constants';

export const top = 20;
export const fullValues = [0, top, ScreenWidth - 1, 180];
export const halfFullValues = [0, top + fullValues[3] / 4, ScreenWidth - 1, fullValues[3] / 2];
export const topValues = [0, top, ScreenWidth - 1, 80];
export const bottomValues = [0, top + 100, ScreenWidth - 1, 80];
export const topLeftKnob = [0, top, W2_4 - 2, 100];
export const topRightKnob = [W2_4, top, W2_4 - 2, 100];
export const bottomLeftKnob = [0, top + 100, W2_4 - 2, 100];
export const bottomRightKnob = [W2_4, top + 100, W2_4 - 2, 100];

export const backgroundBounds = [0, top, ScreenWidth, ScreenHeight - top];

const topBounds = top + 20;
export const bounds1 = [0, topBounds, W1_4 - 2, 80];
export const bounds2 = [W1_4, topBounds, W1_4 - 2, 80];
export const bounds3 = [W1_4 * 2, topBounds, W1_4 - 2, 80];
export const bounds4 = [W1_4 * 3, topBounds, W1_4 - 2, 80];
export const enc1 = { encoderId: 1, bounds: bounds1 };
export const enc2 = { encoderId: 2, bounds: bounds2 };
export const enc3 = { encoderId: 3, bounds: bounds3 };
export const enc4 = { encoderId: 4, bounds: bounds4 };

const topMini = top + 30;
export const bounds1mini = [0, topMini, W1_4 - 2, 30];
export const bounds2mini = [W1_4, topMini, W1_4 - 2, 30];
export const bounds3mini = [W1_4 * 2, topMini, W1_4 - 2, 30];
export const bounds4mini = [W1_4 * 3, topMini, W1_4 - 2, 30];
export const bounds5mini = [0, topMini + 40, W1_4 - 2, 30];
export const bounds6mini = [W1_4, topMini + 40, W1_4 - 2, 30];
export const bounds7mini = [W1_4 * 2, topMini + 40, W1_4 - 2, 30];
export const bounds8mini = [W1_4 * 3, topMini + 40, W1_4 - 2, 30];
export const bounds9mini = [0, topMini + 80, W1_4 - 2, 30];
export const bounds10mini = [W1_4, topMini + 80, W1_4 - 2, 30];
export const bounds11mini = [W1_4 * 2, topMini + 80, W1_4 - 2, 30];
export const bounds12mini = [W1_4 * 3, topMini + 80, W1_4 - 2, 30];
export const enc1mini = { encoderId: 1, bounds: bounds1mini };
export const enc2mini = { encoderId: 2, bounds: bounds2mini };
export const enc3mini = { encoderId: 3, bounds: bounds3mini };
export const enc4mini = { encoderId: 4, bounds: bounds4mini };
export const enc5mini = { encoderId: 1, bounds: bounds5mini };
export const enc6mini = { encoderId: 2, bounds: bounds6mini };
export const enc7mini = { encoderId: 3, bounds: bounds7mini };
export const enc8mini = { encoderId: 4, bounds: bounds8mini };
export const enc9mini = { encoderId: 1, bounds: bounds9mini };
export const enc10mini = { encoderId: 2, bounds: bounds10mini };
export const enc11mini = { encoderId: 3, bounds: bounds11mini };
export const enc12mini = { encoderId: 4, bounds: bounds12mini };
export function moveUp(enc: { encoderId: number; bounds: number[] }, move: number) {
    return { ...enc, bounds: [enc.bounds[0], enc.bounds[1] - move, enc.bounds[2], enc.bounds[3]] };
}

const topSmpl = top + 5;
export const bounds1smpl = [0, topSmpl, W1_4 - 2, 24];
export const bounds2smpl = [W1_4, topSmpl, W1_4 - 2, 24];
export const bounds3smpl = [W1_4 * 2, topSmpl, W1_4 - 2, 24];
export const bounds4smpl = [W1_4 * 3, topSmpl, W1_4 - 2, 24];
export const bounds5smpl = [0, topSmpl + 80, W1_4 - 2, 24];
export const bounds6smpl = [W1_4, topSmpl + 80, W1_4 - 2, 24];
export const bounds7smpl = [W1_4 * 2, topSmpl + 80, W1_4 - 2, 24];
export const bounds8smpl = [W1_4 * 3, topSmpl + 80, W1_4 - 2, 24];
export const bounds9smpl = [0, topSmpl + 115, W1_4 - 2, 24];
export const bounds10smpl = [W1_4, topSmpl + 115, W1_4 - 2, 24];
export const bounds11smpl = [W1_4 * 2, topSmpl + 115, W1_4 - 2, 24];
export const bounds12smpl = [W1_4 * 3, topSmpl + 115, W1_4 - 2, 24];
export const enc1smpl = { encoderId: 1, bounds: bounds1smpl };
export const enc2smpl = { encoderId: 2, bounds: bounds2smpl };
export const enc3smpl = { encoderId: 3, bounds: bounds3smpl };
export const enc4smpl = { encoderId: 4, bounds: bounds4smpl };
export const enc5smpl = { encoderId: 1, bounds: bounds5smpl };
export const enc6smpl = { encoderId: 2, bounds: bounds6smpl };
export const enc7smpl = { encoderId: 3, bounds: bounds7smpl };
export const enc8smpl = { encoderId: 4, bounds: bounds8smpl };
export const enc9smpl = { encoderId: 1, bounds: bounds9smpl };
export const enc10smpl = { encoderId: 2, bounds: bounds10smpl };
export const enc11smpl = { encoderId: 3, bounds: bounds11smpl };
export const enc12smpl = { encoderId: 4, bounds: bounds12smpl };

const seqmarginLeft = 3;
export const seqTop = top + 20;
export const seqWidth = W1_4 - 12;
export const bounds1Seq = [seqmarginLeft + 0, seqTop, seqWidth, 50];
export const bounds2Seq = [seqmarginLeft + seqWidth, seqTop, seqWidth, 50];
export const bounds3Seq = [seqmarginLeft + seqWidth * 2, seqTop, seqWidth, 50];
export const bounds4Seq = [seqmarginLeft + seqWidth * 3, seqTop, seqWidth, 50];

export const bounds5Seq = [seqmarginLeft + 0, seqTop + 60, seqWidth, 50];
export const bounds6Seq = [seqmarginLeft + seqWidth, seqTop + 60, seqWidth, 50];
export const bounds7Seq = [seqmarginLeft + seqWidth * 2, seqTop + 60, seqWidth, 50];
export const bounds8Seq = [seqmarginLeft + seqWidth * 3, seqTop + 60, seqWidth, 50];

export const enc1Seq = { encoderId: 1, bounds: bounds1Seq };
export const enc2Seq = { encoderId: 2, bounds: bounds2Seq };
export const enc3Seq = { encoderId: 3, bounds: bounds3Seq };
export const enc4Seq = { encoderId: 4, bounds: bounds4Seq };
export const enc5Seq = { encoderId: 5, bounds: bounds5Seq };
export const enc6Seq = { encoderId: 6, bounds: bounds6Seq };
export const enc7Seq = { encoderId: 7, bounds: bounds7Seq };
export const enc8Seq = { encoderId: 8, bounds: bounds8Seq };

export const graphTopValues = [0, top + 20, ScreenWidth - 1, 60];
export const graphCenterValues = [0, top + 60, ScreenWidth - 1, 80];
export const graphBottomValues = [0, top + 120, ScreenWidth - 1, 60];

export const graphBounds = [0, top + 50, ScreenWidth - 1, 70];

export function boundsOnTop(bounds: number[], height: number = 22, margin: number = 10) {
    return [bounds[0], bounds[1] + margin  - 20, bounds[2], height];
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

export const seqContextTrack1 = 50;
export const seqContextTrack2 = 51;
export const seqContextTrack3 = 52;
export const seqContextTrack4 = 53;

export const seqContextTrack5 = 57;
export const seqContextTrack6 = 58;

export const seqContextTrack7 = 59;
export const seqContextTrack8 = 60;
