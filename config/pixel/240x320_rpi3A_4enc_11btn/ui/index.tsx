import * as React from '@/libs/react';
import { ClipsView } from './clipsView/ClipsView';
import {
    ColorTrack1,
    ColorTrack2,
    ColorTrack3,
    ColorTrack4,
    ColorTrack6,
    ColorTrack7,
    ColorTrack8,
    Drum23Track,
    Sample1Track,
    Sample2Track,
    Sample3Track,
    ScreenHeight,
    ScreenWidth,
    Synth1Track,
    Synth2Track,
    Synth3Track,
    W1_2
} from './constants';
import { Drum23View } from './Drum23/Drum23View';
import { CreateWorkspaceView } from './menu/CreateWorkspaceView';
import { MenuView } from './menu/menuView';
import { ShutdownView } from './menu/ShutdownView';
import { WorkspacesView } from './menu/WorkspacesView';
import { PercFxView } from './Perc/PercFxView';
import { PercModView } from './Perc/PercModView';
import { PercNoise2View } from './Perc/PercNoise2View';
import { PercNoiseView } from './Perc/PercNoiseView';
import { PercView } from './Perc/PercView';
import { PercSeqView } from './Perc/SeqView';
import { SampleView } from './Sample/SampleView';
import { SynthView } from './Synth/SynthView';

const halfHeight = ScreenHeight / 2;
// addZoneEncoder([0, 0, W1_2, halfHeight]);
// addZoneEncoder([W1_2, 0, W1_2, halfHeight]);
// addZoneEncoder([0, halfHeight, W1_2, halfHeight]);
// addZoneEncoder([W1_2, halfHeight, W1_2, halfHeight]);

export const ui = {
    pixelController: 'rpi3A_4enc_11btn',
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
            <Drum23View track={Drum23Track} synthName="Kick" color={ColorTrack1} />

            <SynthView track={Synth1Track} synthName="Synth1" color={ColorTrack2} />
            <SynthView track={Synth2Track} synthName="Synth2" color={ColorTrack3} />
            <SynthView track={Synth3Track} synthName="Synth3" color={ColorTrack4} />

            <PercView name="Perc" />
            <PercFxView name="PercFx" />
            <PercSeqView name="PercSeq" />
            <PercNoiseView name="PercNoise" />
            <PercNoise2View name="PercNoise2" />
            <PercModView name="PercMod" />

            <SampleView track={Sample1Track} synthName="Sample1" color={ColorTrack6} />
            <SampleView track={Sample2Track} synthName="Sample2" color={ColorTrack7} />
            <SampleView track={Sample3Track} synthName="Sample3" color={ColorTrack8} />

            <MenuView name="Menu" />
            <CreateWorkspaceView name="CreateWorkspace" />
            <WorkspacesView name="Workspaces" />
            <ShutdownView name="Shutdown" />

            <ClipsView name="Clips" />
        </>
    ),
};
