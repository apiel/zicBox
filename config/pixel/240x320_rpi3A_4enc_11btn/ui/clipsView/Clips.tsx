import * as React from '@/libs/react';

import { Clips as ClipsComponent } from '@/libs/nativeComponents/Clips';
import { Text } from '@/libs/nativeComponents/Text';
import { btn1, btn2, btn5, btn6, ScreenWidth } from '../constants';

export type Props = {
    color: string;
    track: number;
    title: string;
};

export function Clips({ color, track, title }: Props) {
    const w = ScreenWidth / 8;
    return (
        <>
            <Text text={title} bounds={[(track - 1) * w, 2, w, 8]} centered color={color} />
            {/* <Text text={title} bounds={[(track - 1) * w, 2, w, 8]} centered background_color={color} color="text" /> */}
            <ClipsComponent
                bounds={[(track - 1) * w, 15, w - 2, 100]}
                track={track}
                color={color}
                visibleCount={12}
                groupAll={0}
                keys={[
                    { key: btn1, action: '.toggle', context: {id: 254, value: 0} },
                    { key: btn2, action: '.up', context: {id: 254, value: 0}},
                    { key: btn6, action: '.down', context: {id: 254, value: 0}},

                    // { key: btn1, action: '.next', context: '254:1' },
                    { key: btn5, action: '.delete', context: {id: 254, value: 1} },
                    { key: btn2, action: '.save', context: {id: 254, value: 1} },       
                ]}
            />
        </>
    );
}
