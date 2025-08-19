import * as React from '@/libs/react';
import { Drum1, Drum2, Drum3, Drum4, Sample1, Sample2, Sample3, Sample4 } from './components/Common';
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
    Drum4Track,
    Sample1Track,
    Sample2Track,
    Sample3Track,
    Sample4Track,
    ScreenHeight,
    ScreenWidth
} from './constants';
import {
    bounds1,
    bounds2,
    bounds3,
    bounds4,
    seqContextDrum1,
    seqContextDrum2,
    seqContextDrum3,
    seqContextDrum4,
    seqContextSample1,
    seqContextSample2,
    seqContextSample3,
    seqContextSample4
} from './constantsValue';
import { DrumViews } from './Drum/DrumViews';
import { MenuView } from './menu/MenuView';
import { SampleViews } from './Sample/SampleViews';

// const halfHeight = ScreenHeight / 2;

export const ui = {
    pixelController: 'pixel_12btn',
    // pixelController: 'grid',
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
        ],
    },
    // renderer: 'FB',
    taggedViews: { track: 'Drum1' },
    views: (
        <>
            <DrumViews track={Drum1Track} synthName="Drum1" color={ColorTrack1} title={Drum1} contextId={seqContextDrum1} />
            <DrumViews track={Drum2Track} synthName="Drum2" color={ColorTrack2} title={Drum2} contextId={seqContextDrum2} />
            <DrumViews track={Drum3Track} synthName="Drum3" color={ColorTrack3} title={Drum3} contextId={seqContextDrum3} />
            <DrumViews track={Drum4Track} synthName="Drum4" color={ColorTrack4} title={Drum4} contextId={seqContextDrum4} />

            <SampleViews track={Sample1Track} synthName="Sample1" color={ColorTrack5} title={Sample1} contextId={seqContextSample1} />
            <SampleViews track={Sample2Track} synthName="Sample2" color={ColorTrack6} title={Sample2} contextId={seqContextSample2} />
            <SampleViews track={Sample3Track} synthName="Sample3" color={ColorTrack7} title={Sample3} contextId={seqContextSample3} />
            <SampleViews track={Sample4Track} synthName="Sample4" color={ColorTrack8} title={Sample4} contextId={seqContextSample4} />

            <MenuView name="Menu" />
        </>
    ),
};
