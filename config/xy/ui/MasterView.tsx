import * as React from '@/libs/react';

import { Draw } from '@/libs/nativeComponents/Draw';
import { KnobValue } from '@/libs/nativeComponents/KnobValue';
import { Rect } from '@/libs/nativeComponents/Rect';
import { Text } from '@/libs/nativeComponents/Text';
import { Track } from '@/libs/nativeComponents/Track';
import { WorkspaceKnob } from '@/libs/nativeComponents/WorkspaceKnob';
import { rgb } from '@/libs/ui';
import { Layout } from './components/Layout';
import { TextArray } from './components/TextArray';
import { TrackKeys } from './components/TrackKeys';
import {
    A1,
    A2,
    A3,
    A4,
    B1,
    B2,
    B3,
    B4,
    B5,
    B6,
    B7,
    B8,
    enc1,
    enc3,
    enc4,
    MasterTrack,
    ScreenHeight,
    ScreenWidth,
    shiftContext,
    Track1,
    Track2,
    Track3,
    Track4,
    Track5,
    Track6,
    Track7,
    Track8,
    unshiftVisibilityContext,
    W1_4,
    W1_8,
    W2_8,
    W3_8,
    W4_8,
    W5_8,
    W6_8,
    W7_8,
} from './constants';

export type Props = {
    name: string;
};

export function MasterView({ name }: Props) {
    const color = rgb(170, 170, 170);
    const muteColor = '#d6d6d6';
    return (
        <Layout
            viewName={name}
            color={color}
            title="Master"
            noPrevious
            content={
                <>
                    <WorkspaceKnob
                        {...enc1}
                        audioPlugin="SerializeTrack"
                        keys={[
                            { key: A1, action: 'playPause' },
                            { key: A2, action: '.load' },
                            { key: A3, action: `contextToggle:${shiftContext}:2:0` },
                            { key: A4, action: 'setView:&previous' },
                        ]}
                    />

                    <KnobValue audioPlugin="Tempo" param="BPM" {...enc3} color="tertiary" track={MasterTrack} />

                    <KnobValue audioPlugin="TrackFx" param="VOLUME" label="Master Vol." {...enc4} color="tertiary" track={MasterTrack} />

                    <Draw
                        bounds={[W1_4 / 2 - 2, 142, 25, 50]}
                        lines={[
                            [0, 0],
                            [0, 25],
                            [20, 25],
                            [20, 50],
                        ]}
                        color={'#6b6b6b'}
                    />

                    <TextArray texts={['TOGGLE_PLAY_PAUSE', 'Load', 'Mute', 'Exit', '---']} top={ScreenHeight - 40} />

                    <Rect bounds={[0, ScreenHeight - 20, ScreenWidth, 20]} color={color} />
                    <TextArray texts={['1', '2', '3', '4', '5', '6', '7', '8']} top={ScreenHeight - 20} bgColor={color} />
                    <Text
                        text="Tracks"
                        bounds={[W6_8, ScreenHeight - 28, W2_8, 10]}
                        centered={true}
                        color="text"
                        bgColor={color}
                        font="PoppinsLight_8"
                        visibilityContext={[unshiftVisibilityContext]}
                    />

                    <Text
                        text="Mute"
                        bounds={[W6_8, ScreenHeight - 28, W2_8, 10]}
                        centered={true}
                        color="text"
                        bgColor={color}
                        font="PoppinsLight_8"
                        visibilityContext={[{ condition: 'SHOW_WHEN', index: shiftContext, value: 2 }]}
                    />

                    <Track
                        bounds={[W1_8 - 14, ScreenHeight - 15, 10, 10]}
                        track={Track1}
                        bgColor={color}
                        color={muteColor}
                        keys={[{ key: B1, action: `.mute`, context: { id: shiftContext, value: 2 } }]}
                    />
                    <Track
                        bounds={[W2_8 - 14, ScreenHeight - 15, 10, 10]}
                        track={Track2}
                        bgColor={color}
                        color={muteColor}
                        keys={[{ key: B2, action: `.mute`, context: { id: shiftContext, value: 2 } }]}
                    />
                    <Track
                        bounds={[W3_8 - 14, ScreenHeight - 15, 10, 10]}
                        track={Track3}
                        bgColor={color}
                        color={muteColor}
                        keys={[{ key: B3, action: `.mute`, context: { id: shiftContext, value: 2 } }]}
                    />
                    <Track
                        bounds={[W4_8 - 14, ScreenHeight - 15, 10, 10]}
                        track={Track4}
                        bgColor={color}
                        color={muteColor}
                        keys={[{ key: B4, action: `.mute`, context: { id: shiftContext, value: 2 } }]}
                    />
                    <Track
                        bounds={[W5_8 - 14, ScreenHeight - 15, 10, 10]}
                        track={Track5}
                        bgColor={color}
                        color={muteColor}
                        keys={[{ key: B5, action: `.mute`, context: { id: shiftContext, value: 2 } }]}
                    />
                    <Track
                        bounds={[W6_8 - 14, ScreenHeight - 15, 10, 10]}
                        track={Track6}
                        bgColor={color}
                        color={muteColor}
                        keys={[{ key: B6, action: `.mute`, context: { id: shiftContext, value: 2 } }]}
                    />
                    <Track
                        bounds={[W7_8 - 14, ScreenHeight - 15, 10, 10]}
                        track={Track7}
                        bgColor={color}
                        color={muteColor}
                        keys={[{ key: B7, action: `.mute`, context: { id: shiftContext, value: 2 } }]}
                    />
                    <Track
                        bounds={[ScreenWidth - 14, ScreenHeight - 15, 10, 10]}
                        track={Track8}
                        bgColor={color}
                        color={muteColor}
                        keys={[{ key: B8, action: `.mute`, context: { id: shiftContext, value: 2 } }]}
                    />

                    <TrackKeys viewName={name} />
                </>
            }
        />
    );
}
