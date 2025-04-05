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
import { CreateWorkspaceView } from './menu/CreateWorkspaceView';
import { MenuView } from './menu/menuView';
import { ShutdownView } from './menu/ShutdownView';
import { WorkspacesView } from './menu/WorkspacesView';
import { SampleView } from './Sample/SampleView';
import { SynthView } from './Synth/SynthView';
import { TempoView } from './Tempo/TempoView';

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
            <KickView track={KickTrack} synthName="Kick" color={ColorTrack1} />

            <DrumView track={Drum1Track} synthName="Drum1" color={ColorTrack2} />
            <DrumView track={Drum2Track} synthName="Drum2" color={ColorTrack3} />
            <DrumView track={Drum3Track} synthName="Drum3" color={ColorTrack4} />

            <SynthView track={Synth1Track} synthName="Synth1" color={ColorTrack5} />
            <SynthView track={Synth2Track} synthName="Synth2" color={ColorTrack6} />

            <SampleView track={Sample1Track} synthName="Sample1" color={ColorTrack7} />
            <SampleView track={Sample2Track} synthName="Sample2" color={ColorTrack8} />

            <MenuView name="Menu" />
            <CreateWorkspaceView name="CreateWorkspace" />
            <WorkspacesView name="Workspaces" />
            <ShutdownView name="Shutdown" />
            <TempoView name="Tempo" />

            <ClipsView name="Clips" />
        </>
    ),
};
