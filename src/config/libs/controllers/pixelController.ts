import { applyZic } from '@/libs/core';

export function pixelController(type: 'rpi0_4enc_6btn') {
    applyZic([{ PIXEL_CONTROLLER: type }]);
}
