import * as React from '@/libs/react';
import { ClipsView } from './ClipsView/ClipsView';
import {
    ColorTrack1,
    ColorTrack10,
    ColorTrack2,
    ColorTrack9,
    Drum1Track,
    Drum2Track,
    Sample1Track,
    Sample2Track,
    ScreenHeight,
    ScreenWidth
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
import { DrumViews } from './Drum/DrumViews';
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
            <DrumViews track={Drum2Track} synthName="Drum2" color={ColorTrack2} />

            <DrumSeqView name="DrumSeq" />

            <SampleView track={Sample1Track} synthName="Sample1" color={ColorTrack9} />
            <SampleView track={Sample2Track} synthName="Sample2" color={ColorTrack10} />

            <Menu />

            <ClipsView name="Clips" />
        </>
    ),
};
