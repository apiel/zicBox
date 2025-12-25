import * as React from '@/libs/react';

import { Container } from '@/libs/nativeComponents/Container';
import { View } from '@/libs/nativeComponents/View';
import {
    ColorTrack1,
    ScreenHeight,
    ScreenWidth,
    trackContextId,
    W1_4,
    W2_4,
    W3_4
} from './constants';
import { TimelinePart } from './TimelinePart';
// import { ColorTrack1, ColorTrack2, ColorTrack3, ColorTrack4, ColorTrack5, ColorTrack6, Track1, Track2, Track3, Track4, Track5, Track6, trackContextId } from '../constants';
import { PatchEdit } from './PatchEdit';

const patchEditHeight = 200;


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
            yRamMargin: 0,
        },
    },
    renderer: 'ST7789',
    taggedViews: { track: 'Track1' },
    views: (
        <>
            <View name="Timeline">
                {/* <TrackContainter name="Track1" track={Track1} /> */}
                {/* <TrackContainter name="Track2" track={Track2} /> */}
                {/* <TrackContainter name="Track3" track={Track3} /> */}
                {/* <TrackContainter name="Track4" track={Track4} /> */}
                {/* <TrackContainter name="Track5" track={Track5} /> */}
                {/* <TrackContainter name="Track6" track={Track6} /> */}
                {/* <Container name="Timeline" position={[0, patchEditHeight]} height="100%"> */}
                    <TimelinePart />
                {/* </Container> */}
            </View>
        </>
    ),
};

function TrackContainter({ name, track }: { name: string; track: number }) {
    return (
        <Container
            name={name}
            position={[0, 0]}
            height={`${patchEditHeight}px`}
            visibilityContext={[{ index: trackContextId, value: track, condition: 'SHOW_WHEN' }]}
        >
            <PatchEdit name={name} track={track} synthName={name} color={ColorTrack1} title={`${track}.`} />
        </Container>
    );
}
