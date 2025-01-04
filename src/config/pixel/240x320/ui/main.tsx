import { View } from '@/libs/components/View';
import { applyZic } from '@/libs/core';
import * as React from '@/libs/react';
import { addZoneEncoder, setScreenSize, setWindowPosition } from '@/libs/ui';
import { ScreenHeight, ScreenWidth, W1_2, W1_4, W3_4 } from './constants';

setWindowPosition(400, 500);
setScreenSize(ScreenWidth, ScreenHeight);

addZoneEncoder([0, 0, W1_4, ScreenHeight]);
addZoneEncoder([W1_4, 0, W1_4, ScreenHeight]);
addZoneEncoder([W1_2, 0, W1_4, ScreenHeight]);
addZoneEncoder([W3_4, 0, W1_4, ScreenHeight]);

applyZic(<View name={'test'}></View>);

// applyZic(getView('test', [{ABC: 123}]));
