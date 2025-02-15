import * as React from '@/libs/react';

import { Clips as ClipsComponent } from '@/libs/nativeComponents/Clips';
import { Keymaps } from '@/libs/nativeComponents/Keymaps';
import { Text } from '@/libs/nativeComponents/Text';
import { btn1, btn2, btn5, btn6, ScreenWidth } from '../constants';

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
            <Text text={title} bounds={[(track - 1) * w, 2, w, 8]} centered color={color} />
            {/* <Text text={title} bounds={[(track - 1) * w, 2, w, 8]} centered background_color={color} color="text" /> */}
            <ClipsComponent
                bounds={[(track - 1) * w, 15, w - 2, 100]}
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
                        { key: btn1, action: '.toggle', context: '254:0' },
                        { key: btn2, action: '.up', context: '254:0' },
                        { key: btn6, action: '.down', context: '254:0' },

                        // { key: btn1, action: '.next', context: '254:1' },
                        { key: btn5, action: '.delete', context: '254:1' },
                        { key: btn2, action: '.save', context: '254:1' },
                    ]}
                />
            </ClipsComponent>
        </>
    );
}
