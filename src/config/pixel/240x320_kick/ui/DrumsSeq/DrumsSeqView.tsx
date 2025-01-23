import * as React from '@/libs/react';

import { Keymaps } from '@/libs/components/Keymaps';
import { StepEditMono } from '@/libs/components/StepEditMono';
import { Text } from '@/libs/components/Text';
import { TextGrid } from '@/libs/components/TextGrid';
import { View } from '@/libs/components/View';
import { VisibilityContext } from '@/libs/components/VisibilityContext';
import { rgb } from '@/libs/ui';
import { Common } from '../components/Common';
import { ColorTrack5, KeyInfoPosition, SampleTrack, ScreenWidth } from '../constants';

// SHOW_PLAYING_STEP
function Seq({
    x,
    w,
    track,
    encoder,
    color,
    show_playing_step,
}: {
    x: number;
    w: number;
    track: number;
    encoder: number;
    color: string;
    show_playing_step?: boolean;
}) {
    let y = 0;
    return Array.from({ length: 32 }, (_, i) => {
        const yy = y + 12;
        y += 8 + (i % 4 == 3 ? 3 : 0);
        return (
            <StepEditMono
                position={[x, yy, w, 8]}
                data={`Sequencer ${i}`}
                group={i}
                playing_color={rgb(35, 161, 35)}
                background_color={
                    i % 8 == 0 || i % 8 == 1 || i % 8 == 2 || i % 8 == 3
                        ? rgb(42, 54, 56)
                        : 'background'
                }
                selected_color={rgb(76, 94, 97)}
                track={track}
                encoder={encoder}
                show_playing_step={show_playing_step}
                on_color={color}
            />
        );
    });
}

export type Props = {
    name: string;
};

export function DrumsSeqView({ name }: Props) {
    const w = ScreenWidth / 3;
    let y = 0;
    return (
        <View name={name}>
            <Text text=" Sample" position={[2 * w, 0, w, 9]} background_color={ColorTrack5} color="text" />
            <Seq x={w * 2} w={w} track={SampleTrack} encoder={2} color={ColorTrack5} show_playing_step />
            <TextGrid
                position={KeyInfoPosition}
                rows={[
                    '&icon::arrowUp::filled &empty ...',
                    '&icon::arrowDown::filled Sample &icon::musicNote::pixelated',
                ]}
            >
                <VisibilityContext index={254} condition="SHOW_WHEN" value={0} />
                <Keymaps
                    keys={[
                        { key: 'q', action: 'incGroup:-1' },
                        // { key: 'w', action: 'setView:Click' },
                        { key: 'e', action: 'contextToggle:254:1:0' },

                        { key: 'a', action: 'incGroup:+1' },
                        { key: 's', action: 'setView:Sample' },
                        { key: 'd', action: 'noteOn:Sample:60' },
                    ]}
                />
            </TextGrid>
            <Common selected={0} hideSequencer track={SampleTrack} />
        </View>
    );
}
