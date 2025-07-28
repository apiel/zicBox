import * as React from '@/libs/react';
import { ClipsView } from './ClipsView/ClipsView';
import { Drum2, Drum3, Drum4, Synth1, Synth2, Synth3 } from './components/Common';
import {
    ColorTrack1,
    ColorTrack2,
    ColorTrack3,
    ColorTrack4,
    ColorTrack5,
    ColorTrack6,
    ColorTrack7,
    Drum1Track,
    Drum2Track,
    Drum3Track,
    Drum4Track,
    ScreenHeight,
    ScreenWidth,
    Synth1Track,
    Synth2Track,
    Synth3Track
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
    seqContextDrum2,
    seqContextDrum3,
    seqContextDrum4,
    seqContextSynth1,
    seqContextSynth2,
    seqContextSynth3,
} from './constantsValue';
import { DrumViews } from './Drum/DrumViews';
import { DrumSeqView } from './DrumSeqView';
import { KickViews } from './Kick/KickViews';
import { Menu } from './menu';
import { SynthViews } from './Synth/SynthViews';

// const halfHeight = ScreenHeight / 2;

export const ui = {
    // pixelController: 'rpiCM4_4enc_11btn',
    // pixelController: 'rpi3A_4enc_11btn',
    pixelController: 'grid',
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
            <DrumViews track={Drum2Track} synthName="Drum2" color={ColorTrack2} title={Drum2} contextId={seqContextDrum2} />
            <DrumViews track={Drum3Track} synthName="Drum3" color={ColorTrack3} title={Drum3} contextId={seqContextDrum3} />
            <DrumViews track={Drum4Track} synthName="Drum4" color={ColorTrack4} title={Drum4} contextId={seqContextDrum4} />

            <SynthViews track={Synth1Track} synthName="Synth1" color={ColorTrack5} title={Synth1} contextId={seqContextSynth1} />
            <SynthViews track={Synth2Track} synthName="Synth2" color={ColorTrack6} title={Synth2} contextId={seqContextSynth2} />
            <SynthViews track={Synth3Track} synthName="Synth3" color={ColorTrack7} title={Synth3} contextId={seqContextSynth3} />

            <DrumSeqView name="DrumSeq" />

            {/* <SampleView track={Sample1Track} synthName="Sample1" color={ColorTrack9} /> */}
            {/* <SampleView track={Sample2Track} synthName="Sample2" color={ColorTrack10} /> */}

            <Menu />

            <ClipsView name="Clips" />
        </>
    ),
};
