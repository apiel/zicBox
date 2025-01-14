import { applyZic } from '@/libs/core';
import * as React from '@/libs/react';
import { addZoneEncoder, setScreenSize, setWindowPosition } from '@/libs/ui';
import { ScreenHeight, ScreenWidth, W1_2, W1_4, W3_4 } from './constants';
import { MasterView } from './Synth/MasterView';

setWindowPosition(400, 500);
setScreenSize(ScreenWidth, ScreenHeight);

addZoneEncoder([0, 0, W1_4, ScreenHeight]);
addZoneEncoder([W1_4, 0, W1_4, ScreenHeight]);
addZoneEncoder([W1_2, 0, W1_4, ScreenHeight]);
addZoneEncoder([W3_4, 0, W1_4, ScreenHeight]);

applyZic(<MasterView name="Master" />);
// applyZic(<DistortionView name="Distortion" />);
// applyZic(<WaveformView name="Waveform" />);
// applyZic(<FrequencyView name="Frequency" />);
// applyZic(<AmpView name="Amplitude" />);
// applyZic(<ClickView name="Click" />);

// applyZic(<SeqView name="Sequencer" />);