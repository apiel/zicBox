import * as React from '@/libs/react';

import { pixelController } from '@/libs/controllers/pixelController';
import { applyZic } from '@/libs/core';
import { Rect } from '@/libs/nativeComponents/Rect';
import { Text } from '@/libs/nativeComponents/Text';
import { Value } from '@/libs/nativeComponents/Value';
import { View } from '@/libs/nativeComponents/View';
import { setScreenSize, setWindowPosition } from '@/libs/ui';
import { Keymap } from './libs/nativeComponents/Keymap';
import { Yo } from './yo';

// pluginAlias('Tempo', 'libzic_Tempo.so');
// plugin('Tempo');

const ScreenWidth = 240;
const ScreenHeight = 320;

pixelController('rpi3A_4enc_11btn');

setWindowPosition(400, 500);
setScreenSize(ScreenWidth, ScreenHeight);

applyZic(
    <View name="Demo">
        <Text fontSize={16} text="title" bounds={[0, 0, ScreenWidth, 16]} centered />

        <Rect color="tertiary" filled={false} bounds={[10, 30, 100, 50]} />

        <Value
            audioPlugin="Tempo"
            param="BPM"
            bounds={[120, 60, 110, 20]}
            encoderId={0}
            barColor="quaternary"
        >
            <Keymap key="s" action="setView:Drum23" context="254:1" />
        </Value>

        <Yo name="Yo" />
    </View>
);
