import * as React from '@/libs/react';

import { writeFileSync } from 'fs';
import 'tsconfig-paths/register'; // To solve imported alias

import { audioPlugin } from '@/libs/audio';
import { Keyboard } from '@/libs/nativeComponents/Keyboard';
import { KnobValue } from '@/libs/nativeComponents/KnobValue';
import { List } from '@/libs/nativeComponents/List';
import { Rect } from '@/libs/nativeComponents/Rect';
import { Text } from '@/libs/nativeComponents/Text';
import { Value } from '@/libs/nativeComponents/Value';
import { View } from '@/libs/nativeComponents/View';

const ScreenWidth = 240;
const ScreenHeight = 320;

const demoView = (
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

const output = JSON.stringify(
    {
        audio: { tracks: [{ id: 0, plugins: [audioPlugin('Tempo')] }] },
        pixelController: 'rpi3A_4enc_11btn',
        screen: { screenSize: { width: ScreenWidth, height: ScreenHeight } },
        views: [demoView],
    },
    null,
    2
);

writeFileSync('data/config.json', output);
console.log('done');
