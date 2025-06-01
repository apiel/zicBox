import * as React from '@/libs/react';

import { Clips as ClipsComponent } from '@/libs/nativeComponents/Clips';
import { Text } from '@/libs/nativeComponents/Text';
import { ScreenWidth } from '../constants';

export type Props = {
    color: string;
    track: number;
    title: string;
    key: string;
};

export function Clips({ color, track, title, key }: Props) {
    const w = ScreenWidth / 5;
    return (
        <>
            <Text text={title} bounds={[(track - 1) * w, 2, w, 8]} centered color={color} />
            {/* <Text text={title} bounds={[(track - 1) * w, 2, w, 8]} centered background_color={color} color="text" /> */}
            <ClipsComponent
                bounds={[(track - 1) * w, 15, w - 2, 100]}
                track={track}
                color={color}
                visibleCount={12}
                keys={[
                    { key, action: '.saveOrDelete', context: {id: 251, value: 1} },
                    { key, action: '.toggle', context: {id: 254, value: 0} },
                ]}
            />
        </>
    );
}
