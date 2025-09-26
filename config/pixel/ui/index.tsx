import * as React from '@/libs/react';
import { Drum1, Drum2, Drum3, Drum4, Sample1, Sample2, Synth1, Synth2 } from './components/Common';
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
    ScreenHeight,
    ScreenWidth,
    Synth1Track,
    Synth2Track,
    W1_4,
    W2_4,
    W3_4,
} from './constants';
import {
    seqContextTrack1,
    seqContextTrack2,
    seqContextTrack3,
    seqContextTrack4,
    seqContextTrack5,
    seqContextTrack6,
    seqContextTrack7,
    seqContextTrack8,
} from './constantsValue';
import { DrumViews } from './Drum/DrumViews';
import { MenuView } from './menu/MenuView';
import { ShuttingDown } from './menu/ShuttingDown';
import { Updating } from './menu/Updating';
import { SampleViews } from './Sample/SampleViews';
import { SynthViews } from './Synth/SynthViews';

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
    taggedViews: { track: 'Drum1' },
    views: (
        <>
            <DrumViews
                track={Drum1Track}
                synthName="Drum1"
                color={ColorTrack1}
                title={Drum1}
                contextId={seqContextTrack1}
            />
            <DrumViews
                track={Drum2Track}
                synthName="Drum2"
                color={ColorTrack2}
                title={Drum2}
                contextId={seqContextTrack2}
            />
            <DrumViews
                track={Drum3Track}
                synthName="Drum3"
                color={ColorTrack3}
                title={Drum3}
                contextId={seqContextTrack3}
            />
            <DrumViews
                track={Drum4Track}
                synthName="Drum4"
                color={ColorTrack4}
                title={Drum4}
                contextId={seqContextTrack4}
            />

            <SampleViews
                track={Sample1Track}
                synthName="Sample1"
                color={ColorTrack5}
                title={Sample1}
                contextId={seqContextTrack5}
            />
            <SampleViews
                track={Sample2Track}
                synthName="Sample2"
                color={ColorTrack6}
                title={Sample2}
                contextId={seqContextTrack6}
            />

            <SynthViews
                track={Synth1Track}
                synthName="Synth1"
                color={ColorTrack7}
                title={Synth1}
                contextId={seqContextTrack7}
            />
            <SynthViews
                track={Synth2Track}
                synthName="Synth2"
                color={ColorTrack8}
                title={Synth2}
                contextId={seqContextTrack8}
            />

            <MenuView name="Menu" />
            <ShuttingDown name="ShuttingDown" />
            <Updating name="Updating" />
        </>
    ),
};
