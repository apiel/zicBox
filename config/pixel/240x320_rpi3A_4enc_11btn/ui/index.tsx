import * as React from '@/libs/react';
import { BassView } from './Bass/BassView';
import { BassEnvView } from './Bass/EnvView';
import { BassFxView } from './Bass/FxView';
import { BassSeqView } from './Bass/SeqView';
import { BassWaveformView } from './Bass/WaveformView';
import { ClipsView } from './clipsView/ClipsView';
import { ScreenHeight, ScreenWidth, W1_2 } from './constants';
import { AmpView } from './Drum23/AmpView';
import { ClickView } from './Drum23/ClickView';
import { DistortionView } from './Drum23/DistortionView';
import { Drum23View } from './Drum23/Drum23View';
import { FrequencyView } from './Drum23/FrequencyView';
import { Drum23SeqView } from './Drum23/SeqView';
import { WaveformView } from './Drum23/WaveformView';
import { CreateWorkspaceView } from './menu/CreateWorkspaceView';
import { MenuView } from './menu/menuView';
import { ShutdownView } from './menu/ShutdownView';
import { PercFxView } from './Perc/PercFxView';
import { PercModView } from './Perc/PercModView';
import { PercNoise2View } from './Perc/PercNoise2View';
import { PercNoiseView } from './Perc/PercNoiseView';
import { PercView } from './Perc/PercView';
import { PercSeqView } from './Perc/SeqView';
import { SampleEditorView } from './Sample/SampleEditorView';
import { SynthEnv1View } from './Synth/Env1View';
import { SynthEnv2View } from './Synth/Env2View';
import { SynthFilterView } from './Synth/FilterView';
import { SynthFxView } from './Synth/FxView';
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
            <Drum23View name="Drum23" />
            <DistortionView name="Distortion" />
            <WaveformView name="Waveform" />
            <FrequencyView name="Frequency" />
            <AmpView name="Amplitude" />
            <ClickView name="Click" />
            <Drum23SeqView name="Sequencer" />

            <SampleEditorView name="Sample" />

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

            <SynthView name="Synth" />
            <SynthFxView name="SynthFx" />
            <SynthFilterView name="SynthFilter" />
            <SynthEnv1View name="SynthEnv1" />
            <SynthEnv2View name="SynthEnv2" />

            <MenuView name="Menu" />
            <CreateWorkspaceView name="CreateWorkspace" />
            <ShutdownView name="Shutdown" />

            <ClipsView name="Clips" />
        </>
    ),
};
