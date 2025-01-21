import * as React from '@/libs/react';

import { Keymaps } from '@/libs/components/Keymaps';
import { StepEditSmall } from '@/libs/components/StepEditSmall';
import { Text } from '@/libs/components/Text';
import { TextGrid } from '@/libs/components/TextGrid';
import { View } from '@/libs/components/View';
import { VisibilityContext } from '@/libs/components/VisibilityContext';
import { rgb } from '@/libs/ui';
import { Common } from '../components/Common';
import { HiHatTrack, KeyInfoPosition, PercTrack, ScreenWidth, SnareTrack } from '../constants';

// SHOW_PLAYING_STEP
function Seq({
    x,
    w,
    track,
    encoder,
    show_playing_step,
}: {
    x: number;
    w: number;
    track: number;
    encoder: number;
    show_playing_step?: boolean;
}) {
    let y = 0;
    return Array.from({ length: 32 }, (_, i) => {
        const yy = y + 12;
        y += 8 + (i % 4 == 3 ? 3 : 0);
        return (
            <StepEditSmall
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
            <Text text="Snare" position={[0, 0, w, 8]} />
            <Seq x={0} w={w} track={SnareTrack} encoder={0} />
            <Text text="HiHat" position={[w, 0, w, 8]} />
            <Seq x={w} w={w} track={HiHatTrack} encoder={1} />
            <Text text="Perc" position={[2 * w, 0, w, 8]} />
            <Seq x={w * 2} w={w} track={PercTrack} encoder={2} show_playing_step />
            <TextGrid
                position={KeyInfoPosition}
                rows={[
                    '&icon::arrowUp::filled &icon::toggle::rect ...',
                    '&icon::arrowDown::filled Synth &icon::musicNote::pixelated',
                ]}
            >
                <VisibilityContext index={254} condition="SHOW_WHEN" value={0} />
                <Keymaps
                    keys={[
                        { key: 'q', action: 'incGroup:-1' },
                        // { key: 'w', action: 'setView:Click' },
                        { key: 'e', action: 'contextToggle:254:1:0' },

                        { key: 'a', action: 'incGroup:+1' },
                        { key: 's', action: 'setView:Snare' },
                        { key: 'd', action: 'noteOn:Snare:60' },
                    ]}
                />
            </TextGrid>
            <Common selected={0} hideSequencer track={SnareTrack} />
        </View>
    );
}
