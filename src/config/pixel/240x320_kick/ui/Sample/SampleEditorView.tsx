import * as React from '@/libs/react';

import { SampleEditor } from '@/libs/components/SampleEditor';
import { View } from '@/libs/components/View';
import { Common } from '../components/Common';
import { TextGridDrums } from '../components/TextGridDrums';
import { SampleTrack, ScreenWidth } from '../constants';

export type Props = {
    name: string;
};

export function SampleEditorView({ name }: Props) {
    return (
        <View name={name}>
            <SampleEditor position={[0, 80, ScreenWidth, 100]} track={SampleTrack} />

            <TextGridDrums selected={1} viewName={name} target="Sample" />
            <Common selected={1} track={SampleTrack} />
        </View>
    );
}
