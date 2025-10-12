import * as React from '@/libs/react';
import {
    ColorTrack1,
    ColorTrack2,
    ColorTrack3,
    ColorTrack4,
    ColorTrack5,
    ColorTrack6,
    ColorTrack7,
    ColorTrack8,
    Sample1Track,
    Sample2Track,
    ScreenHeight,
    ScreenWidth,
    Track1,
    Track2,
    Track3,
    Track4,
    Track5,
    Track6,
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
import { Github } from './menu/Github';
import { GithubHelp } from './menu/GithubHelp';
import { MenuView } from './menu/MenuView';
import { ShuttingDown } from './menu/ShuttingDown';
import { UpdateAvailable } from './menu/UpdateAvailable';
import { Updating } from './menu/Updating';
import { WifiView } from './menu/Wifi';
import { SampleViews } from './Sample/SampleViews';

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
                track={Track1}
                synthName="Drum1"
                color={ColorTrack1}
                title="1."
                contextId={seqContextTrack1}
            />
            <DrumViews
                track={Track2}
                synthName="Drum2"
                color={ColorTrack2}
                title="2."
                contextId={seqContextTrack2}
            />
            <DrumViews
                track={Track3}
                synthName="Drum3"
                color={ColorTrack3}
                title="3."
                contextId={seqContextTrack3}
            />
            <DrumViews
                track={Track4}
                synthName="Drum4"
                color={ColorTrack4}
                title="4."
                contextId={seqContextTrack4}
            />

            <DrumViews
                track={Track5}
                synthName="Synth1"
                color={ColorTrack5}
                title="5."
                contextId={seqContextTrack5}
            />
            <DrumViews
                track={Track6}
                synthName="Synth2"
                color={ColorTrack6}
                title="6."
                contextId={seqContextTrack6}
            />

            <SampleViews
                track={Sample1Track}
                synthName="Sample1"
                color={ColorTrack7}
                title="Sample1"
                contextId={seqContextTrack7}
            />
            <SampleViews
                track={Sample2Track}
                synthName="Sample2"
                color={ColorTrack8}
                title="Sample2"
                contextId={seqContextTrack8}
            />

            <MenuView name="Menu" />
            <ShuttingDown name="ShuttingDown" />
            <Updating name="Updating" />
            <UpdateAvailable name="UpdateAvailable" />
            <Github name="Github" />
            <GithubHelp name="GithubHelp" />
            <WifiView name="Wifi" />
        </>
    ),
};
