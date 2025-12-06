import * as React from '@/libs/react';
import {
    ColorTrack1,
    ColorTrack2,
    ColorTrack3,
    ColorTrack4,
    ColorTrack5,
    ColorTrack6,
    ScreenHeight,
    ScreenWidth,
    Track1,
    Track2,
    Track3,
    Track4,
    Track5,
    Track6,
    W1_4,
    W2_4,
    W3_4
} from './constants';
import {
    seqContextTrack1,
    seqContextTrack2,
    seqContextTrack3,
    seqContextTrack4,
    seqContextTrack5,
    seqContextTrack6
} from './constantsValue';
import { MultiSynthViews } from './MultiSynthViews';

export const ui = {
    pixelController: 'pixel_12btn',
    // pixelController: 'grid',
    // i2c: ['pixel+_v1'],
    screen: {
        windowPosition: { x: 0, y: 300 },
        screenSize: { width: ScreenWidth, height: ScreenHeight },
        zonesEncoders: [
            [0, 0, 0, 0], // lets skip encoder id 0
            [0, 0, W1_4, ScreenHeight],
            [W1_4, 0, W1_4, ScreenHeight],
            [W2_4, 0, W1_4, ScreenHeight],
            [W3_4, 0, W1_4, ScreenHeight],
        ],
        st7789: {
            resetPin: 17,
        },
    },
    renderer: 'ST7789',
    taggedViews: { track: 'Track1' },
    views: (
        <>
            <MultiSynthViews
                track={Track1}
                synthName="Track1"
                color={ColorTrack1}
                title="1."
                contextId={seqContextTrack1}
            />
            <MultiSynthViews
                track={Track2}
                synthName="Track2"
                color={ColorTrack2}
                title="2."
                contextId={seqContextTrack2}
            />
            <MultiSynthViews
                track={Track3}
                synthName="Track3"
                color={ColorTrack3}
                title="3."
                contextId={seqContextTrack3}
            />
            <MultiSynthViews
                track={Track4}
                synthName="Track4"
                color={ColorTrack4}
                title="4."
                contextId={seqContextTrack4}
            />

            <MultiSynthViews
                track={Track5}
                synthName="Track5"
                color={ColorTrack5}
                title="5."
                contextId={seqContextTrack5}
            />
            <MultiSynthViews
                track={Track6}
                synthName="Track6"
                color={ColorTrack6}
                title="6."
                contextId={seqContextTrack6}
            />
        </>
    ),
};
