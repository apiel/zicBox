import { pixelController } from '@/libs/controllers/pixelController';
import { applyZic } from '@/libs/core';
import * as React from '@/libs/react';
import { addZoneEncoder, setScreenSize, setWindowPosition } from '@/libs/ui';
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
import { Fm1EnvView } from './Fm1/EnvView';
import { Fm1View } from './Fm1/Fm1View';
import { Fm1FxView } from './Fm1/FxView';
import { Fm1SeqView } from './Fm1/SeqView';
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
import { SampleEditorView } from './Sample/SampleEditorView';
import { SynthEnv1View } from './Synth/Env1View';
import { SynthEnv2View } from './Synth/Env2View';
import { SynthFxView } from './Synth/FxhView';
import { SynthView } from './Synth/SynthView';
import { SynthWaveform1View } from './Synth/Waveform1View';
import { SynthWaveform2View } from './Synth/Waveform2View';

pixelController('rpi3A_4enc_11btn');

setWindowPosition(400, 500);
setScreenSize(ScreenWidth, ScreenHeight);

const halfHeight = ScreenHeight / 2;
addZoneEncoder([0, 0, W1_2, halfHeight]);
addZoneEncoder([W1_2, 0, W1_2, halfHeight]);
addZoneEncoder([0, halfHeight, W1_2, halfHeight]);
addZoneEncoder([W1_2, halfHeight, W1_2, halfHeight]);

applyZic(<Drum23View name="Drum23" />);
applyZic(<DistortionView name="Distortion" />);
applyZic(<WaveformView name="Waveform" />);
applyZic(<FrequencyView name="Frequency" />);
applyZic(<AmpView name="Amplitude" />);
applyZic(<ClickView name="Click" />);
applyZic(<Drum23SeqView name="Sequencer" />);

applyZic(<SampleEditorView name="Sample" />);

applyZic(<BassView name="Bass" />);
applyZic(<BassFxView name="BassFx" />);
applyZic(<BassEnvView name="BassEnv" />);
applyZic(<BassSeqView name="BassSeq" />);
applyZic(<BassWaveformView name="BassWaveform" />);

applyZic(<Fm1View name="Fm1" />);
applyZic(<Fm1FxView name="Fm1Fx" />);
applyZic(<Fm1EnvView name="Fm1Env" />);
applyZic(<Fm1SeqView name="Fm1Seq" />);

applyZic(<PercView name="Perc" />);
applyZic(<PercFxView name="PercFx" />);
// applyZic(<PercEnvView name="PercEnv" />);
applyZic(<PercSeqView name="PercSeq" />);
applyZic(<PercNoiseView name="PercNoise" />);
applyZic(<PercNoise2View name="PercNoise2" />);
applyZic(<PercModView name="PercMod" />);

applyZic(<SynthView name="Synth" />);
applyZic(<SynthFxView name="SynthFx" />);
applyZic(<SynthEnv1View name="SynthEnv1" />);
applyZic(<SynthEnv2View name="SynthEnv2" />);
applyZic(<SynthWaveform1View name="SynthWaveform1" />);
applyZic(<SynthWaveform2View name="SynthWaveform2" />);

applyZic(<MenuView name="Menu" />);
applyZic(<WorkspacesView name="Workspaces" />);
applyZic(<CreateWorkspaceView name="CreateWorkspace" />);
applyZic(<ShutdownView name="Shutdown" />);

// Tape is messing up the memory, need to fix it!
// applyZic(<TapeView name="Tape" />);
// applyZic(<SaveTapeView name="SaveTape" />);

applyZic(<ClipsView name="Clips" />);
