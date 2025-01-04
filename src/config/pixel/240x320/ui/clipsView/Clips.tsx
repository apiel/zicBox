import * as React from '@/libs/react';

import { Clips as ClipsComponent } from '@/libs/components/Clips';
import { Keymaps } from '@/libs/components/Keymaps';
import { W1_8 } from '../constants';

export type Props = {
    color: string;
    track: number;
    group: number;
};

export function Clips({ color, track, group }: Props) {
    return (
        <ClipsComponent
            position={[(track - 1) * W1_8, 15, W1_8 - 2, 200]}
            plugin="SerializeTrack"
            track={track}
            group={group}
            color={color}
            seq_plugin="Sequencer"
        >
            <Keymaps
                keys={[
                    { key: 'q', action: '.toggle', context: '254:0' },
                    { key: 'w', action: '.up', context: '254:0' },
                    { key: 's', action: '.down', context: '254:0' },

                    { key: 'q', action: '.next', context: '254:1' },
                    { key: 's', action: '.delete', context: '254:1' },
                    { key: 'd', action: '.save', context: '254:1' },
                ]}
            />
        </ClipsComponent>
    );
}
