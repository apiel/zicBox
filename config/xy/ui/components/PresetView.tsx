import * as React from '@/libs/react';

import { Preset } from '@/libs/nativeComponents/Preset';
import { Text } from '@/libs/nativeComponents/Text';
import { A1, A2, A3, A4, ScreenHeight, ScreenWidth } from '../constants';
import { Layout } from './Layout';
import { TextArray } from './TextArray';

export type Props = {
    name: string;
    track: number;
    synthName: string;
    color: string;
    title: string;
};

export function PresetView({ name, track, synthName, color, title }: Props) {
    return (
        <Layout
            viewName={name}
            color={color}
            synthName={synthName}
            title={title}
            noPrevious
            content={
                <>
                    <Text text="Preset:" bounds={[20, 80, 100, 20]} font="PoppinsLight_12" color="#b6b6b6" />
                    <Preset
                        bounds={[20, 100, ScreenWidth - 40, 30]}
                        audioPlugin={synthName}
                        track={track}
                        folder="data/presets/SynthMultiEngine"
                        keys={[
                            { key: A1, action: `.loadTrig` }, // if already loaded, pressing it will trigger the sound
                            { key: A2, action: `.restore`, action2: `setView:${synthName}SavePreset` },
                            { key: A3, action: `.restore`, action2: `setView:&previous` },
                            { key: A4, action: `.exit`, action2: `setView:&previous` },
                        ]}
                    />
                   
                    <TextArray texts={['Load', 'Save', 'Cancel', 'Done', '---']} top={ScreenHeight - 40} />
                    <TextArray texts={['---', '&icon::trash', '---', '---', '---', '---', '---', '---']} top={ScreenHeight - 20} />
                </>
            }
        />
    );
}
