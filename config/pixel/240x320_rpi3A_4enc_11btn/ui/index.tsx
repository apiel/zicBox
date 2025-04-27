import * as React from '@/libs/react';
import { ClipsView } from './ClipsView/ClipsView';
import {
    ColorTrack1,
    ColorTrack2,
    ColorTrack3,
    ColorTrack4,
    ColorTrack5,
    ColorTrack6,
    ColorTrack7,
    ColorTrack8,
    Drum1Track,
    Drum2Track,
    Drum3Track,
    KickTrack,
    Sample1Track,
    Sample2Track,
    ScreenHeight,
    ScreenWidth,
    Synth1Track,
    Synth2Track,
    W1_2
} from './constants';
import { DrumView } from './Drum/DrumView';
import { KickView } from './Kick/KickView';
import { Menu } from './menu';
import { SampleView } from './Sample/SampleView';
import { SynthView } from './Synth/SynthView';

const halfHeight = ScreenHeight / 2;
// addZoneEncoder([0, 0, W1_2, halfHeight]);
// addZoneEncoder([W1_2, 0, W1_2, halfHeight]);
// addZoneEncoder([0, halfHeight, W1_2, halfHeight]);
// addZoneEncoder([W1_2, halfHeight, W1_2, halfHeight]);

export const ui = {
    pixelController: 'rpiCM4_4enc_11btn', //  'rpi3A_4enc_11btn',
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
    views: (
        <>
            <KickView track={KickTrack} synthName="Kick" color={ColorTrack1} />

            <DrumView track={Drum1Track} synthName="Drum1" color={ColorTrack2} />
            <DrumView track={Drum2Track} synthName="Drum2" color={ColorTrack3} />
            <DrumView track={Drum3Track} synthName="Drum3" color={ColorTrack4} />

            <SynthView track={Synth1Track} synthName="Synth1" color={ColorTrack5} />
            <SynthView track={Synth2Track} synthName="Synth2" color={ColorTrack6} />

            <SampleView track={Sample1Track} synthName="Sample1" color={ColorTrack7} />
            <SampleView track={Sample2Track} synthName="Sample2" color={ColorTrack8} />

            <Menu />

            <ClipsView name="Clips" />
        </>
    ),
};
