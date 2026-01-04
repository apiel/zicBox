import * as React from '@/libs/react';

import { SavePreset } from '@/libs/nativeComponents/SavePreset';
import { Text } from '@/libs/nativeComponents/Text';
import { A1, A2, A3, A4, ScreenHeight, ScreenWidth, W1_4 } from '../constants';
import { Layout } from './Layout';
import { TextArray } from './TextArray';

const bgColor = '#3a3a3a';

const topBounds = 50;
const bounds1 = [0, topBounds, W1_4 - 2, 80];
const bounds2 = [W1_4, topBounds, W1_4 - 2, 80];
const bounds3 = [W1_4 * 2, topBounds, W1_4 - 2, 80];
const bounds4 = [W1_4 * 3, topBounds, W1_4 - 2, 80];
const enc1 = { encoderId: 1, bounds: bounds1 };
const enc2 = { encoderId: 2, bounds: bounds2 };
const enc3 = { encoderId: 3, bounds: bounds3 };
const enc4 = { encoderId: 4, bounds: bounds4 };

export type Props = {
    name: string;
    track: number;
    synthName: string;
    color: string;
    title: string;
};

export function SavePresetView({ name, track, synthName, color, title }: Props) {
    return (
        <Layout
            viewName={name}
            color={color}
            synthName={synthName}
            title={title}
            noPrevious
            content={
                <>
                    <Text text="Save preset" bounds={[230, 3, 100, 20]} font="PoppinsLight_12" color="#b6b6b6" />

                    <Text
                        fontSize={16}
                        text="abc"
                        bounds={enc1.bounds}
                        font="PoppinsLight_8"
                        bgColor={bgColor}
                        centered
                        extendEncoderIdArea={enc1.encoderId}
                    />
                    <Text
                        fontSize={16}
                        text="Cursor_"
                        bounds={enc2.bounds}
                        font="PoppinsLight_8"
                        bgColor={bgColor}
                        centered
                        extendEncoderIdArea={enc2.encoderId}
                    />

                    <SavePreset
                        bounds={[20, 140, ScreenWidth - 40, 30]}
                        audioPlugin={synthName}
                        track={track}
                        folder="data/presets/SynthMultiEngine"
                        keys={[
                            { key: A2, action: `.backspace` },
                            { key: A1, action: `.jump` },
                            { key: A3, action: `setView:&previous` },
                            { key: A4, action: `.save` },

                        ]}
                    />

                    <TextArray texts={['aA0!', '&icon::backspace::filled', 'Done', 'Save', '---']} top={ScreenHeight - 40} />
                    <TextArray texts={['---', '---', '---', '---', '---', '---', '---', '---']} top={ScreenHeight - 20} />
                </>
            }
        />
    );
}
