import { applyZic } from '@/libs/core';
import * as React from '@/libs/react';
import { addZoneEncoder, setScreenSize, setWindowPosition } from '@/libs/ui';
import { ScreenHeight, ScreenWidth, W1_2 } from './constants';
import { AmpView } from './Drum23/AmpView';
import { ClickView } from './Drum23/ClickView';
import { DistortionView } from './Drum23/DistortionView';
import { Drum23View } from './Drum23/Drum23View';
import { FrequencyView } from './Drum23/FrequencyView';
import { Drum23SeqView } from './Drum23/SeqView';
import { WaveformView } from './Drum23/WaveformView';
import { DrumsSeqView } from './DrumsSeq/DrumsSeqView';
import { Fm2View } from './Fm/Fm2View';
import { FmView } from './Fm/FmView';
import { HiHat2View } from './HiHat/HiHat2View';
import { HiHatView } from './HiHat/HiHatView';
import { CreateWorkspaceView } from './menu/CreateWorkspaceView';
import { MenuView } from './menu/menuView';
import { ShutdownView } from './menu/ShutdownView';
import { WorkspacesView } from './menu/WorkspacesView';
import { Sample2View } from './Sample/Sample2View';
import { SampleView } from './Sample/SampleView';
import { Snare2View } from './Snare/Snare2View';
import { SnareView } from './Snare/SnareView';

setWindowPosition(400, 500);
setScreenSize(ScreenWidth, ScreenHeight);

const halfHeight = ScreenHeight / 2;
addZoneEncoder([0, 0, W1_2, halfHeight]);
addZoneEncoder([0, halfHeight, W1_2, halfHeight]);
addZoneEncoder([W1_2, 0, W1_2, halfHeight]);
addZoneEncoder([W1_2, halfHeight, W1_2, halfHeight]);

applyZic(<Drum23View name="Drum23" />);
applyZic(<DistortionView name="Distortion" />);
applyZic(<WaveformView name="Waveform" />);
applyZic(<FrequencyView name="Frequency" />);
applyZic(<AmpView name="Amplitude" />);
applyZic(<ClickView name="Click" />);
applyZic(<Drum23SeqView name="Sequencer" />);

applyZic(<SnareView name="Snare" />);
applyZic(<Snare2View name="Snare2" />);

applyZic(<HiHatView name="HiHat" />);
applyZic(<HiHat2View name="HiHat2" />);

applyZic(<FmView name="Fm" />);
applyZic(<Fm2View name="Fm2" />);

// applyZic(<PercView name="Perc" />);
// applyZic(<Perc2View name="Perc2" />);

applyZic(<SampleView name="Sample" />);
applyZic(<Sample2View name="Sample2" />);

applyZic(<DrumsSeqView name="DrumsSeq" />);

applyZic(<MenuView name="Menu" />);
applyZic(<WorkspacesView name="Workspaces" />);
applyZic(<CreateWorkspaceView name="CreateWorkspace" />);
applyZic(<ShutdownView name="Shutdown" />);