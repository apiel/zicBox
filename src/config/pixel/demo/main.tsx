import * as React from '@/libs/react';

import { plugin, pluginAlias } from '@/libs/audio';
import { Text } from '@/libs/components/Text';
import { View } from '@/libs/components/View';
import { applyZic } from '@/libs/core';
import { setScreenSize, setWindowPosition } from '@/libs/ui';

pluginAlias('AudioOutput', 'libzic_AudioOutputPulse.so');
pluginAlias('Bass', 'libzic_SynthBass.so');
pluginAlias('Tempo', 'libzic_Tempo.so');
plugin('Bass');
plugin('AudioOutput');
plugin('Tempo');

const ScreenWidth = 240;
const ScreenHeight = 320;

setWindowPosition(400, 500);
setScreenSize(ScreenWidth, ScreenHeight);

applyZic(
    <View name="Demo">
        <Text fontSize={16} text="title" position={[0, 20, ScreenWidth, 16]} centered />
        <Text text="hello world" position={[5, 5, ScreenWidth, 16]} color="secondary" />
    </View>
);
