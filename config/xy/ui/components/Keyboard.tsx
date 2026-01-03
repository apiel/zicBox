import * as React from '@/libs/react';

import { KnobValue } from '@/libs/nativeComponents/KnobValue';
import { NoteGrid } from '@/libs/nativeComponents/NoteGrid';
import { Text } from '@/libs/nativeComponents/Text';
import { A1, A3, A4, B1, B2, B3, B4, B5, B6, B7, B8, ScreenHeight, ScreenWidth, W1_4, W2_4 } from '../constants';
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

export function Keyboard({ name, track, synthName, color, title }: Props) {
    const gridBounds = [...enc1.bounds];
    gridBounds[2] = W2_4 - 2;
    const textTop = 84;
    let keyIndex = 0;
    return (
        <Layout
            viewName={name}
            color={color}
            synthName={synthName}
            title={title}
            content={
                <>
                    <Text
                        fontSize={24}
                        text="Scale"
                        bounds={enc1.bounds}
                        font="PoppinsLight_12"
                        bgColor={bgColor}
                        centered
                        extendEncoderIdArea={enc1.encoderId}
                    />
                    <Text
                        fontSize={24}
                        text="Oct."
                        bounds={enc2.bounds}
                        font="PoppinsLight_12"
                        bgColor={bgColor}
                        centered
                        extendEncoderIdArea={enc2.encoderId}
                    />
                    <KnobValue audioPlugin="Sequencer" param="PLAYING_LOOPS" {...enc3} color="primary" track={track} bgColor={bgColor} />

                    <TextArray
                        texts={['&icon::play::filled', '---', 'Exit', 'Save', '---']}
                        top={ScreenHeight - textTop}
                    />

                    <NoteGrid
                        audioPlugin="Sequencer"
                        track={track}
                        bounds={[0, 180, ScreenWidth, 60]}
                        encScale={enc1.encoderId}
                        encOctave={enc2.encoderId}
                        cols={8}
                        rows={1}
                        keys={[
                            { key: A1, action: `playPause` },
                            { key: A3, action: `setView:&previous` },
                            { key: A4, action: `data:Sequencer:${track}:SAVE_RECORD` },

                            { key: B1, action: `.key:${keyIndex++}` },
                            { key: B2, action: `.key:${keyIndex++}` },
                            { key: B3, action: `.key:${keyIndex++}` },
                            { key: B4, action: `.key:${keyIndex++}` },

                            { key: B5, action: `.key:${keyIndex++}` },
                            { key: B6, action: `.key:${keyIndex++}` },
                            { key: B7, action: `.key:${keyIndex++}` },
                            { key: B8, action: `.key:${keyIndex++}` },
                        ]}
                    />

                    {/* <KnobAction
                        action={`data:Sequencer:${track}:SAVE_RECORD`}
                        action2={`setView:&previous`}
                        encoderId={enc4.encoderId}
                    /> */}
                </>
            }
        />
    );
}
