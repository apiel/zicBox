import * as React from '@/libs/react';

import { Clips as ClipsComponent } from '@/libs/components/Clips';
import { Keymaps } from '@/libs/components/Keymaps';
import { Text } from '@/libs/components/Text';
import { ScreenWidth } from '../constants';

export type Props = {
    color: string;
    track: number;
    group: number;
    title: string;
    group_all: number;
};

export function Clips({ color, track, group, title, group_all }: Props) {
    const w = ScreenWidth / 5;
    return (
        <>
            <Text text={title} position={[(track - 1) * w, 2, w, 8]} centered color={color} />
            {/* <Text text={title} position={[(track - 1) * w, 2, w, 8]} centered background_color={color} color="text" /> */}
            <ClipsComponent
                position={[(track - 1) * w, 15, w - 2, 100]}
                plugin="SerializeTrack"
                track={track}
                group={group}
                color={color}
                seq_plugin="Sequencer"
                visible_count={12}
                group_all={group_all}
            >
                <Keymaps
                    keys={[
                        { key: 'q', action: '.toggle', context: '254:0' },
                        { key: 'w', action: '.up', context: '254:0' },
                        { key: 's', action: '.down', context: '254:0' },

                        // { key: 'q', action: '.next', context: '254:1' },
                        { key: 's', action: '.delete', context: '254:1' },
                        { key: 'w', action: '.save', context: '254:1' },
                    ]}
                />
            </ClipsComponent>
        </>
    );
}
