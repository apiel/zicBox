import * as React from '@/libs/react';

import { KnobAction } from '@/libs/nativeComponents/KnobAction';
import { KnobValue } from '@/libs/nativeComponents/KnobValue';
import { NoteGrid } from '@/libs/nativeComponents/NoteGrid';
import { Text } from '@/libs/nativeComponents/Text';
import { A1, A2, A3, A4, B1, B2, B3, B4, C1, C2, C3, C4, ScreenWidth, W2_4 } from '../constants';
import { enc1, enc2, enc3, enc4 } from '../constantsValue';
import { Layout } from './Layout';

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
    let keyIndex = 0;
    return (
        <Layout
            viewName={name}
            color={color}
            synthName={synthName}
            title={title}
            content={
                <>
                    <NoteGrid
                        audioPlugin="Sequencer"
                        track={track}
                        bounds={gridBounds}
                        encScale={enc1.encoderId}
                        encOctave={enc2.encoderId}
                        keys={[
                            { key: A1, action: `.key:${keyIndex++}` },
                            { key: A2, action: `.key:${keyIndex++}` },
                            { key: A3, action: `.key:${keyIndex++}` },
                            { key: A4, action: `.key:${keyIndex++}` },

                            { key: B1, action: `.key:${keyIndex++}` },
                            { key: B2, action: `.key:${keyIndex++}` },
                            { key: B3, action: `.key:${keyIndex++}` },
                            { key: B4, action: `.key:${keyIndex++}` },

                            { key: C1, action: `.key:${keyIndex++}` },
                            { key: C2, action: `.key:${keyIndex++}` },
                            { key: C3, action: `.key:${keyIndex++}` },
                            { key: C4, action: `.key:${keyIndex++}` },
                        ]}
                    />

                    <KnobValue
                        audioPlugin="Sequencer"
                        param="PLAYING_LOOPS"
                        {...enc3}
                        color="primary"
                        track={track}
                    />

                    <Text
                        fontSize={12}
                        text="Turn right knob to exit and save."
                        bounds={[120, 4, ScreenWidth, 16]}
                        font="PoppinsLight_12"
                        color={'#8f8f8fff'}
                    />
                    <Text
                        fontSize={12}
                        text="&icon::arrowDown::filled"
                        bounds={[270, 25, 16, 16]}
                        font="PoppinsLight_12"
                        color={'#8f8f8fff'}
                    />

                    <Text
                        fontSize={32}
                        text="OK"
                        bounds={enc4.bounds}
                        font="PoppinsLight_16"
                        centered
                    />
                    <KnobAction
                        action={`data:Sequencer:${track}:SAVE_RECORD`}
                        action2={`setView:&previous`}
                        encoderId={enc4.encoderId}
                    />
                </>
            }
        />
    );
}
