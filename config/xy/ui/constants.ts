import { VisibilityContext } from '@/libs/nativeComponents/component';
import { rgb } from '@/libs/ui';

export * from '../constants';

export const ScreenWidth = 320;
export const ScreenHeight = 240;

export const W1_4 = ScreenWidth / 4;
export const W2_4 = W1_4 * 2;
export const W1_2 = W2_4;
export const W3_4 = W1_4 * 3;

export const W1_8 = ScreenWidth / 8;
export const W2_8 = W1_8 * 2;
export const W3_8 = W1_8 * 3;
export const W4_8 = W1_8 * 4;
export const W5_8 = W1_8 * 5;
export const W6_8 = W1_8 * 6;
export const W7_8 = W1_8 * 7;
export const W8_8 = W1_8 * 8;

export const ColorTrackMaster = rgb(255, 74, 74);
export const ColorTrack1 = rgb(143, 175, 16);
export const ColorTrack2 = rgb(253, 111, 14);
export const ColorTrack3 = rgb(21, 165, 202);
export const ColorTrack4 = rgb(209, 169, 7);
export const ColorTrack5 = rgb(181, 79, 94);
export const ColorTrack6 = rgb(55, 114, 117);
export const ColorTrack7 = rgb(98, 79, 181);
export const ColorTrack8 = rgb(181, 79, 173);
export const ColorTrack9 = rgb(4, 196, 180);
export const ColorTrack10 = rgb(245, 176, 239);

export const menuTextColor = rgb(120, 120, 120);

export const bgColor = '#3a3a3a'; // Use as forground color for value encoders...

export const encTopLeft = 0;
export const encTopRight = 1;
export const encBottomLeft = 2;
export const encBottomRight = 3;

export const A1 = 'a';
export const A2 = 's';
export const A3 = 'd';
export const A4 = 'f';
export const A5 = 'g';

export const B1 = 'z';
export const B2 = 'x';
export const B3 = 'c';
export const B4 = 'v';
export const B5 = 'b';
export const B6 = 'n';
export const B7 = 'm';
export const B8 = 54; // ',';

export const C1 = '1';
export const C2 = '2';
export const C3 = '3';
export const C4 = '4';

export const seqContextTrack1 = 50;
export const seqContextTrack2 = 51;
export const seqContextTrack3 = 52;
export const seqContextTrack4 = 53;
export const seqContextTrack5 = 57;
export const seqContextTrack6 = 58;
export const seqContextTrack7 = 59;
export const seqContextTrack8 = 60;

export const shiftContext = 254;
export const clipRenderContext = 200;
export const seqContext = 201;
export const shutdownContext = 202;
export const updateContext = 203;
export const githubContext = 204;
export const githubContext2 = 205;
export const engineTypeIdContext = 210;

export const unshiftVisibilityContext: VisibilityContext = {
    condition: 'SHOW_WHEN',
    index: shiftContext,
    value: 0,
};

export const shiftVisibilityContext: VisibilityContext = {
    condition: 'SHOW_WHEN',
    index: shiftContext,
    value: 1,
};

export const notUnshiftVisibilityContext: VisibilityContext = {
    condition: 'SHOW_WHEN_NOT',
    index: shiftContext,
    value: 0,
};

const topBounds = 50;
const bounds1 = [0, topBounds, W1_4 - 2, 80];
const bounds2 = [W1_4, topBounds, W1_4 - 2, 80];
const bounds3 = [W1_4 * 2, topBounds, W1_4 - 2, 80];
const bounds4 = [W1_4 * 3, topBounds, W1_4 - 2, 80];
export const enc1 = { encoderId: 1, bounds: bounds1 };
export const enc2 = { encoderId: 2, bounds: bounds2 };
export const enc3 = { encoderId: 3, bounds: bounds3 };
export const enc4 = { encoderId: 4, bounds: bounds4 };