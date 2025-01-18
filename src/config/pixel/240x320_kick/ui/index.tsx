import { applyZic } from '@/libs/core';
import * as React from '@/libs/react';
import { addZoneEncoder, setScreenSize, setWindowPosition } from '@/libs/ui';
import { ScreenHeight, ScreenWidth, W1_2 } from './constants';
import { CreateWorkspaceView } from './menu/CreateWorkspaceView';
import { MenuView } from './menu/menuView';
import { ShutdownView } from './menu/ShutdownView';
import { WorkspacesView } from './menu/WorkspacesView';
import { SeqView } from './Seq';
import { AmpView } from './Synth/AmpView';
import { ClickView } from './Synth/ClickView';
import { DistortionView } from './Synth/DistortionView';
import { FrequencyView } from './Synth/FrequencyView';
import { MasterView } from './Synth/MasterView';
import { WaveformView } from './Synth/WaveformView';

setWindowPosition(400, 500);
setScreenSize(ScreenWidth, ScreenHeight);

const halfHeight = ScreenHeight / 2;
addZoneEncoder([0, 0, W1_2, halfHeight]);
addZoneEncoder([0, halfHeight, W1_2, halfHeight]);
addZoneEncoder([W1_2, 0, W1_2, halfHeight]);
addZoneEncoder([W1_2, halfHeight, W1_2, halfHeight]);

applyZic(<MasterView name="Master" />);
applyZic(<DistortionView name="Distortion" />);
applyZic(<WaveformView name="Waveform" />);
applyZic(<FrequencyView name="Frequency" />);
applyZic(<AmpView name="Amplitude" />);
applyZic(<ClickView name="Click" />);

applyZic(<SeqView name="Sequencer" />);

applyZic(<MenuView name="Menu" />);
applyZic(<WorkspacesView name="Workspaces" />);
applyZic(<CreateWorkspaceView name="CreateWorkspace" />);
applyZic(<ShutdownView name="Shutdown" />);