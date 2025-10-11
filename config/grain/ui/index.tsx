import * as React from '@/libs/react';
import { ScreenHeight, ScreenWidth, W1_4, W2_4, W3_4 } from './constants';
import { Page1 } from './Page1';
import { Page2 } from './Page2';
import { Page3 } from './Page3';


const halfHeight = ScreenHeight / 2;

export const ui = {
    pixelController: 'pixel_12btn',
    // pixelController: 'grid',
    // i2c: ['pixel+_v1'],
    screen: {
        windowPosition: { x: 200, y: 300 },
        screenSize: { width: ScreenWidth, height: ScreenHeight },
        zonesEncoders: [
            [0, 0, 0, 0], // lets skip encoder id 0

            [0, 0, W1_4, halfHeight], // 1
            [W1_4, 0, W1_4, halfHeight], // 2
            [W2_4, 0, W1_4, halfHeight], // 3
            [W3_4, 0, W1_4, halfHeight], // 4

            [0, halfHeight, W1_4, halfHeight], // 5
            [W1_4, halfHeight, W1_4, halfHeight], // 6
            [W2_4, halfHeight, W1_4, halfHeight], // 7
            [W3_4, halfHeight, W1_4, halfHeight], // 8
        ],
        st7789: {
            resetPin: 17,
        },
    },
    // renderer: 'FB',
    views: (
        <>
           <Page1 />
           <Page2 />
           <Page3 />
        </>
    ),
};
