import * as React from '@/libs/react';
import { ClipsView } from './ClipsView/ClipsView';
import {
    ColorTrack1,
    ColorTrack7,
    ColorTrack8,
    KickTrack,
    Sample1Track,
    Sample2Track,
    ScreenHeight,
    ScreenWidth,
    W1_2
} from './constants';
import { KickView } from './Kick/KickView';
import { Menu } from './menu';
import { SampleView } from './Sample/SampleView';

const halfHeight = ScreenHeight / 2;
// addZoneEncoder([0, 0, W1_2, halfHeight]);
// addZoneEncoder([W1_2, 0, W1_2, halfHeight]);
// addZoneEncoder([0, halfHeight, W1_2, halfHeight]);
// addZoneEncoder([W1_2, halfHeight, W1_2, halfHeight]);

export const ui = {
    // pixelController: 'rpiCM4_4enc_11btn',  
    pixelController: 'rpi3A_4enc_11btn',
    // i2c: ['pixel+_v1'],
    screen: {
        windowPosition: { x: 400, y: 500 },
        screenSize: { width: ScreenWidth, height: ScreenHeight },
        zonesEncoders: [
            [0, 0, W1_2, halfHeight],
            [W1_2, 0, W1_2, halfHeight],
            [0, halfHeight, W1_2, halfHeight],
            [W1_2, halfHeight, W1_2, halfHeight],
        ],
    },
    // renderer: 'FB',
    views: (
        <>
            <KickView track={KickTrack} synthName="Kick" color={ColorTrack1} />

            <SampleView track={Sample1Track} synthName="Sample1" color={ColorTrack7} />
            <SampleView track={Sample2Track} synthName="Sample2" color={ColorTrack8} />

            <Menu />

            <ClipsView name="Clips" />
        </>
    ),
};
