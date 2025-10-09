import * as React from '@/libs/react';
import {
    ScreenHeight,
    ScreenWidth,
    W1_4,
    W2_4,
    W3_4
} from './constants';
import { Layout } from './Layout';

export const ui = {
    pixelController: 'pixel_12btn',
    // pixelController: 'grid',
    // i2c: ['pixel+_v1'],
    screen: {
        windowPosition: { x: 200, y: 300 },
        screenSize: { width: ScreenWidth, height: ScreenHeight },
        zonesEncoders: [
            [0, 0, 0, 0], // lets skip encoder id 0
            [0, 0, W1_4, ScreenHeight],
            [W1_4, 0, W1_4, ScreenHeight],
            [W2_4, 0, W1_4, ScreenHeight],
            [W3_4, 0, W1_4, ScreenHeight],
        ],
        st7789: {
            resetPin: 17,
        },
    },
    // renderer: 'FB',
    views: (
        <>
           <Layout viewName="Drum1" content={<></>} color="primary" title="Grain" />
        </>
    ),
};
