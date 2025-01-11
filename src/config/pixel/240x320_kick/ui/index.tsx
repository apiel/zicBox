import { applyZic } from '@/libs/core';
import * as React from '@/libs/react';
import { addZoneEncoder, setScreenSize, setWindowPosition } from '@/libs/ui';
import { ScreenHeight, ScreenWidth, W1_2, W1_4, W3_4 } from './constants';
import { Page1View } from './Page1';
import { DistortionView } from './Page1/DistortionView';
import { FrequencyView } from './Page1/FrequencyView';
import { WaveformView } from './Page1/WaveformView';
import { Page2View } from './Page2';
import { ClickView } from './Page2/ClickView';

setWindowPosition(400, 500);
setScreenSize(ScreenWidth, ScreenHeight);

addZoneEncoder([0, 0, W1_4, ScreenHeight]);
addZoneEncoder([W1_4, 0, W1_4, ScreenHeight]);
addZoneEncoder([W1_2, 0, W1_4, ScreenHeight]);
addZoneEncoder([W3_4, 0, W1_4, ScreenHeight]);

applyZic(<Page1View name="Page1" />);
applyZic(<DistortionView name="Distortion" />);
applyZic(<WaveformView name="Waveform" />);
applyZic(<FrequencyView name="Frequency" />);

applyZic(<Page2View name="Page2" />);
applyZic(<ClickView name="Click" />);
