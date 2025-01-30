import { applyZic } from '@/libs/core';

export function pixelController(type: 'rpi0_4enc_6btn' | 'rpi3A_4enc_11btn') {
    applyZic([{ PIXEL_CONTROLLER: type }]);
}
