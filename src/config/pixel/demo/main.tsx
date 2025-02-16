import * as React from '@/libs/react';

import { plugin, pluginAlias } from '@/libs/audio';
import { pixelController } from '@/libs/controllers/pixelController';
import { applyZic } from '@/libs/core';
import { Keyboard } from '@/libs/nativeComponents/Keyboard';
import { KnobValue } from '@/libs/nativeComponents/KnobValue';
import { List } from '@/libs/nativeComponents/List';
import { Rect } from '@/libs/nativeComponents/Rect';
import { Text } from '@/libs/nativeComponents/Text';
import { Value } from '@/libs/nativeComponents/Value';
import { View } from '@/libs/nativeComponents/View';
import { setScreenSize, setWindowPosition } from '@/libs/ui';

pluginAlias('Tempo', 'libzic_Tempo.so');
plugin('Tempo');

const ScreenWidth = 240;
const ScreenHeight = 320;

pixelController('rpi3A_4enc_11btn');

setWindowPosition(400, 500);
setScreenSize(ScreenWidth, ScreenHeight);

applyZic(
    <View name="Demo">
        <Text fontSize={16} text="title" bounds={[0, 0, ScreenWidth, 16]} centered />

        <Rect color="tertiary" filled={false} bounds={[10, 30, 100, 50]} />
        <Rect color="primary" bounds={[120, 30, 110, 20]} />

        <Value
            audioPlugin="Tempo"
            param="BPM"
            bounds={[120, 60, 110, 20]}
            encoderId={0}
            barColor="quaternary"
        />

        <KnobValue
            audioPlugin="Tempo"
            param="BPM"
            bounds={[10, 90, 70, 70]}
            encoderId={1}
            color="secondary"
        />

        <List items={['item1', 'item2', 'item3']} bounds={[120, 100, 110, 60]} />

        <Keyboard bounds={[0, 175, ScreenWidth, 120]} />
    </View>
);
