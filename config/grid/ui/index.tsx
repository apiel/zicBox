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
} from './constants';
import {
    bounds1,
    bounds10,
    bounds11,
    bounds12,
    bounds2,
    bounds3,
    bounds4,
    bounds5,
    bounds6,
    bounds7,
    bounds8,
    bounds9,
} from './constantsValue';
import { DrumSeqView } from './DrumSeqView';
import { KickViews } from './Kick/KickViews';
import { Menu } from './menu';
import { SampleView } from './Sample/SampleView';

// const halfHeight = ScreenHeight / 2;

export const ui = {
    // pixelController: 'rpiCM4_4enc_11btn',
    pixelController: 'rpi3A_4enc_11btn',
    // i2c: ['pixel+_v1'],
    screen: {
        windowPosition: { x: 200, y: 300 },
        screenSize: { width: ScreenWidth, height: ScreenHeight },
        zonesEncoders: [
            [0, 0, 0, 0], // lets skip encoder id 0
            bounds1,
            bounds2,
            bounds3,
            bounds4,
            bounds5,
            bounds6,
            bounds7,
            bounds8,
            bounds9,
            bounds10,
            bounds11,
            bounds12,
        ],
    },
    renderer: 'FB',
    views: (
        <>
            <KickViews track={Drum1Track} synthName="Drum1" color={ColorTrack1} />

            <DrumSeqView name="DrumSeq" />

            <SampleView track={Sample1Track} synthName="Sample1" color={ColorTrack2} />
            <SampleView track={Sample2Track} synthName="Sample2" color={ColorTrack3} />
            <SampleView track={Sample3Track} synthName="Sample3" color={ColorTrack4} />
            <SampleView track={Sample4Track} synthName="Sample4" color={ColorTrack5} />

            <Menu />

            <ClipsView name="Clips" />
        </>
    ),
};
