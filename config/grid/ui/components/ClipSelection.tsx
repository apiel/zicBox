import * as React from '@/libs/react';

import { Clips } from '@/libs/nativeComponents/Clips';
import {
    B1,
    B10,
    B11,
    B2,
    B3,
    B4,
    B5,
    B6,
    B7,
    B8,
    B9,
    ColorButton,
    ScreenWidth,
    shiftContext,
} from '../constants';

export type Props = {
    track: number;
    color: string;
};

export function ClipSelection({ track, color }: Props) {
    return (
        <Clips
            bounds={[40, 625, ScreenWidth - 80, 14]}
            track={track}
            color={color}
            visibleCount={10}
            keys={[
                { key: B1, action: `.set:1`, context: { id: shiftContext, value: 0 } },
                { key: B1, action: `.save:1`, context: { id: shiftContext, value: 1 } },

                { key: B2, action: `.set:2`, context: { id: shiftContext, value: 0 } },
                { key: B2, action: `.save:2`, context: { id: shiftContext, value: 1 } },

                { key: B3, action: `.set:3`, context: { id: shiftContext, value: 0 } },
                { key: B3, action: `.save:3`, context: { id: shiftContext, value: 1 } },

                { key: B4, action: `.set:4`, context: { id: shiftContext, value: 0 } },
                { key: B4, action: `.save:4`, context: { id: shiftContext, value: 1 } },

                { key: B5, action: `.set:5`, context: { id: shiftContext, value: 0 } },
                { key: B5, action: `.save:5`, context: { id: shiftContext, value: 1 } },

                { key: B6, action: `.set:6`, context: { id: shiftContext, value: 0 } },
                { key: B6, action: `.save:6`, context: { id: shiftContext, value: 1 } },

                { key: B7, action: `.set:7`, context: { id: shiftContext, value: 0 } },
                { key: B7, action: `.save:7`, context: { id: shiftContext, value: 1 } },

                { key: B8, action: `.set:8`, context: { id: shiftContext, value: 0 } },
                { key: B8, action: `.save:8`, context: { id: shiftContext, value: 1 } },

                { key: B9, action: `.set:9`, context: { id: shiftContext, value: 0 } },
                { key: B9, action: `.save:9`, context: { id: shiftContext, value: 1 } },

                { key: B10, action: `.set:10`, context: { id: shiftContext, value: 0 } },
                { key: B10, action: `.save:10`, context: { id: shiftContext, value: 1 } },

                { key: B11, action: `.bank`, context: { id: shiftContext, value: 0 } },
                { key: B11, action: `.reload`, context: { id: shiftContext, value: 1 } },
            ]}
            controllerColors={[
                {
                    controller: 'Default',
                    colors: [
                        { key: B11, color: ColorButton },
                    ],
                },
            ]}
        />
    );
}
