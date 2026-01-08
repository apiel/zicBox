import * as React from '@/libs/react';

import { Draw } from '@/libs/nativeComponents/Draw';
import { KnobValue } from '@/libs/nativeComponents/KnobValue';
import { NoteGrid } from '@/libs/nativeComponents/NoteGrid';
import { Rect } from '@/libs/nativeComponents/Rect';
import { Text } from '@/libs/nativeComponents/Text';
import { Layout } from './components/Layout';
import { TextArray } from './components/TextArray';
import {
    A1,
    A2,
    A3,
    A4,
    A5,
    B1,
    B2,
    B3,
    B4,
    B5,
    B6,
    B7,
    B8,
    bgColor,
    enc1,
    enc2,
    enc3,
    helpTextColor,
    ScreenHeight,
    ScreenWidth,
    shiftContext,
    shiftVisibilityContext,
    unshiftVisibilityContext,
    W2_4,
} from './constants';

export type Props = {
    name: string;
    track: number;
    synthName: string;
    color: string;
    title: string;
};

export function RecView({ name, track, synthName, color, title }: Props) {
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
                    <Rect bounds={[0, 25, ScreenWidth, 130]} visibilityContext={[unshiftVisibilityContext]} />
                    <Rect bounds={[0, 25, ScreenWidth, 130]} color={bgColor} visibilityContext={[shiftVisibilityContext]} />

                    <Text
                        fontSize={12}
                        text="- Press Record to capture each 64-step loop you"
                        bounds={[5, 30, ScreenWidth, 20]}
                        font="PoppinsLight_12"
                        bgColor={bgColor}
                        visibilityContext={[shiftVisibilityContext]}
                        color={helpTextColor}
                    />
                    <Text
                        fontSize={12}
                        text="   play."
                        bounds={[5, 48, ScreenWidth, 20]}
                        font="PoppinsLight_12"
                        bgColor={bgColor}
                        visibilityContext={[shiftVisibilityContext]}
                        color={helpTextColor}
                    />

                    <Text
                        fontSize={12}
                        text="- Turn the Record knob to browse saved loops and"
                        bounds={[5, 72, ScreenWidth, 20]}
                        font="PoppinsLight_12"
                        bgColor={bgColor}
                        visibilityContext={[shiftVisibilityContext]}
                        color={helpTextColor}
                    />
                    <Text
                        fontSize={12}
                        text="   press Play to listen."
                        bounds={[5, 90, ScreenWidth, 20]}
                        font="PoppinsLight_12"
                        bgColor={bgColor}
                        visibilityContext={[shiftVisibilityContext]}
                        color={helpTextColor}
                    />

                    <Text
                        fontSize={12}
                        text="- Press Record again to record on top of the"
                        bounds={[5, 116, ScreenWidth, 20]}
                        font="PoppinsLight_12"
                        bgColor={bgColor}
                        visibilityContext={[shiftVisibilityContext]}
                        color={helpTextColor}
                    />
                    <Text
                        fontSize={12}
                        text="   selected loop."
                        bounds={[5, 134, ScreenWidth, 20]}
                        font="PoppinsLight_12"
                        bgColor={bgColor}
                        visibilityContext={[shiftVisibilityContext]}
                        color={helpTextColor}
                    />


                    <Text
                        fontSize={24}
                        text="Scale"
                        bounds={enc1.bounds}
                        font="PoppinsLight_12"
                        bgColor={bgColor}
                        centered
                        extendEncoderIdArea={enc1.encoderId}
                        visibilityContext={[unshiftVisibilityContext]}
                    />
                    <Text
                        fontSize={24}
                        text="Oct."
                        bounds={enc2.bounds}
                        font="PoppinsLight_12"
                        bgColor={bgColor}
                        centered
                        extendEncoderIdArea={enc2.encoderId}
                        visibilityContext={[unshiftVisibilityContext]}
                    />
                    <KnobValue
                        audioPlugin="Sequencer"
                        param="PLAYING_LOOPS"
                        {...enc3}
                        color="primary"
                        track={track}
                        bgColor={bgColor}
                        visibilityContext={[unshiftVisibilityContext]}
                    />

                    <Draw
                        bounds={[W2_4 - 18, 134, 60, 20]}
                        lines={[
                            [50, 0],
                            [50, 10],
                            [0, 10],
                            [0, 20],
                        ]}
                        color={'#AAAAAA'}
                        visibilityContext={[unshiftVisibilityContext]}
                    />

                    <TextArray
                        texts={['&icon::play::filled', '&icon::record::filled', 'Exit', 'Save', 'Shift']}
                        top={ScreenHeight - textTop}
                        visibilityContext={[unshiftVisibilityContext]}
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
                            { key: A2, action: `audioEvent:TOGGLE_RECORD_STOP` },
                            { key: A3, action: `setView:&previous` },
                            { key: A4, action: `data:Sequencer:${track}:SAVE_RECORD` },
                            { key: A5, action: `contextToggle:${shiftContext}:1:0` },

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
