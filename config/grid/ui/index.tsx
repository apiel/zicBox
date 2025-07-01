import * as React from '@/libs/react';
import { ClipsView } from './ClipsView/ClipsView';
import {
    ColorTrack1,
    ColorTrack2,
    ColorTrack3,
    ColorTrack4,
    ColorTrack5,
    Drum1Track,
    Sample1Track,
    Sample2Track,
    Sample3Track,
    Sample4Track,
    ScreenHeight,
    ScreenWidth,
    W1_2,
} from './constants';
import { DrumView } from './Drum/DrumView';
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
        windowPosition: { x: 200, y: 300 },
        screenSize: { width: ScreenWidth, height: ScreenHeight },
        zonesEncoders: [
            [0, 0, W1_2, halfHeight],
            [W1_2, 0, W1_2, halfHeight],
            [0, halfHeight, W1_2, halfHeight],
            [W1_2, halfHeight, W1_2, halfHeight],
        ],
    },
    renderer: 'FB',
    views: (
        <>
            <DrumView track={Drum1Track} synthName="Drum1" color={ColorTrack1} />

            <SampleView track={Sample1Track} synthName="Sample1" color={ColorTrack2} />
            <SampleView track={Sample2Track} synthName="Sample2" color={ColorTrack3} />
            <SampleView track={Sample3Track} synthName="Sample3" color={ColorTrack4} />
            <SampleView track={Sample4Track} synthName="Sample4" color={ColorTrack5} />

            <Menu />

            <ClipsView name="Clips" />
        </>
    ),
};
