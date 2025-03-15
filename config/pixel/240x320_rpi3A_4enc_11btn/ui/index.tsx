import * as React from '@/libs/react';
import { BassView } from './Bass/BassView';
import { BassEnvView } from './Bass/EnvView';
import { BassFxView } from './Bass/FxView';
import { BassSeqView } from './Bass/SeqView';
import { BassWaveformView } from './Bass/WaveformView';
import { ClipsView } from './clipsView/ClipsView';
import {
    ColorTrack1,
    ColorTrack2,
    ColorTrack3,
    ColorTrack4,
    Drum23Track,
    ScreenHeight,
    ScreenWidth,
    Synth1Track,
    Synth2Track,
    Synth3Track,
    W1_2,
} from './constants';
import { Drum23View } from './Drum23/Drum23View';
import { CreateWorkspaceView } from './menu/CreateWorkspaceView';
import { MenuView } from './menu/menuView';
import { ShutdownView } from './menu/ShutdownView';
import { PercFxView } from './Perc/PercFxView';
import { PercModView } from './Perc/PercModView';
import { PercNoise2View } from './Perc/PercNoise2View';
import { PercNoiseView } from './Perc/PercNoiseView';
import { PercView } from './Perc/PercView';
import { PercSeqView } from './Perc/SeqView';
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

            <BassView name="Bass" />
            <BassFxView name="BassFx" />
            <BassEnvView name="BassEnv" />
            <BassSeqView name="BassSeq" />
            <BassWaveformView name="BassWaveform" />

            {/* <Fm1View name="Fm1" /> */}
            {/* <Fm1FxView name="Fm1Fx" /> */}
            {/* <Fm1EnvView name="Fm1Env" /> */}
            {/* <Fm1SeqView name="Fm1Seq" /> */}

            <PercView name="Perc" />
            <PercFxView name="PercFx" />
            <PercSeqView name="PercSeq" />
            <PercNoiseView name="PercNoise" />
            <PercNoise2View name="PercNoise2" />
            <PercModView name="PercMod" />

            <MenuView name="Menu" />
            <CreateWorkspaceView name="CreateWorkspace" />
            <ShutdownView name="Shutdown" />

            <ClipsView name="Clips" />
        </>
    ),
};
