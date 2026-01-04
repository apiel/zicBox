import * as React from '@/libs/react';

import { KnobValue } from '@/libs/nativeComponents/KnobValue';
import { Rect } from '@/libs/nativeComponents/Rect';
import { Text } from '@/libs/nativeComponents/Text';
import { WorkspaceKnob } from '@/libs/nativeComponents/WorkspaceKnob';
import { rgb } from '@/libs/ui';
import { Layout } from './components/Layout';
import { TextArray } from './components/TextArray';
import { Track } from './components/Track';
import { A1, A2, A4, enc1, enc3, enc4, MasterTrack, ScreenHeight, ScreenWidth, W2_8, W6_8 } from './constants';

export type Props = {
    name: string;
};

export function MasterView({ name }: Props) {
    const color = rgb(170, 170, 170);
    return (
        <Layout
            viewName={name}
            color={color}
            title="Master"
            noPrevious
            content={
                <>
                    <WorkspaceKnob {...enc1} audioPlugin="SerializeTrack" keys={[
                        { key: A1, action: 'playPause' },
                        { key: A2, action: '.load' },
                        { key: A4, action: 'setView:&previous' }
                    ]} />

                    <KnobValue audioPlugin="Tempo" param="BPM" {...enc3} color="tertiary" track={MasterTrack} />

                    <KnobValue audioPlugin="TrackFx" param="VOLUME" label="Master Vol." {...enc4} color="tertiary" track={MasterTrack} />

                    <TextArray texts={['&icon::play::filled', 'Load', '---', 'Exit', '---']} top={ScreenHeight - 40} />

                    <Rect bounds={[0, ScreenHeight - 20, ScreenWidth, 20]} color={color} />
                    <TextArray texts={['1', '2', '3', '4', '5', '6', '7', '8']} top={ScreenHeight - 20} bgColor={color} />
                    <Text
                        text="Tracks"
                        bounds={[W6_8, ScreenHeight - 28, W2_8, 10]}
                        centered={true}
                        color="text"
                        bgColor={color}
                        font="PoppinsLight_8"
                    />

                    <Track viewName={name} />

                    {/*
                    <Text
                        text="Shutdown"
                        bounds={[W2_4, ScreenHeight - 20, W1_4, 16]}
                        centered={true}
                        color={menuTextColor}
                        keys={[{ key: C3, action: `contextToggle:${shutdownContext}:1:0` }]}
                        visibilityContext={[
                            {
                                condition: 'SHOW_WHEN',
                                index: updateContext,
                                value: 0,
                            },
                        ]}
                    />
                    <Text
                        text="YES"
                        bounds={[W3_4, ScreenHeight - 20, W1_4, 16]}
                        centered={true}
                        // color={menuTextColor}
                        bgColor="#ffacac"
                        visibilityContext={[
                            {
                                condition: 'SHOW_WHEN',
                                index: shutdownContext,
                                value: 1,
                            },
                        ]}
                        keys={[{ key: C4, action: 'setView:ShuttingDown', action2: `sh:halt` }]}
                    /> */}
                </>
            }
        />
    );
}
