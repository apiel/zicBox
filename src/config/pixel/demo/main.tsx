import * as React from '@/libs/react';

import { plugin, pluginAlias } from '@/libs/audio';
import { applyZic } from '@/libs/core';
import { Text } from '@/libs/nativeComponents/Text';
import { View } from '@/libs/nativeComponents/View';
import { TextGrid } from '@/libs/tsComponents/TextGrid';
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
        <Text fontSize={16} text="title" bounds={[0, 20, ScreenWidth, 16]} centered />
        <Text text="hello world" bounds={[5, 5, ScreenWidth, 16]} color="secondary" />

        <TextGrid
            bounds={[0, 50, ScreenWidth, 20]}
            rows={[
                'Hello !world &empty',
                '^123 abc &icon::arrowRight::filled',
            ]}
        ></TextGrid>
    </View>
);
