import * as React from '@/libs/react';

import { Container } from '@/libs/nativeComponents/Container';
import { View } from '@/libs/nativeComponents/View';
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
    trackContextId,
    W1_4,
    W2_4,
    W3_4,
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
        },
    },
    renderer: 'ST7789',
    taggedViews: { track: 'Track1' },
    views: (
        <>
            <View name="Timeline">
                <Container name="Track1" position={[0, 0]} height={`${patchEditHeight}px`} visibilityContext={[{ index: trackContextId, value: Track1, condition: 'SHOW_WHEN' }]}>
                    <PatchEdit name="Track1" track={Track1} synthName="Track1" color={ColorTrack1} title="1." />
                </Container>
                <Container name="Track2" position={[0, 0]} height={`${patchEditHeight}px`} visibilityContext={[{ index: trackContextId, value: Track2, condition: 'SHOW_WHEN' }]}>
                    <PatchEdit name="Track2" track={Track2} synthName="Track2" color={ColorTrack2} title="2." />
                </Container>
                <Container name="Track3" position={[0, 0]} height={`${patchEditHeight}px`} visibilityContext={[{ index: trackContextId, value: Track3, condition: 'SHOW_WHEN' }]}>
                    <PatchEdit name="Track3" track={Track3} synthName="Track3" color={ColorTrack3} title="3." />
                </Container>
                <Container name="Track4" position={[0, 0]} height={`${patchEditHeight}px`} visibilityContext={[{ index: trackContextId, value: Track4, condition: 'SHOW_WHEN' }]}>
                    <PatchEdit name="Track4" track={Track4} synthName="Track4" color={ColorTrack4} title="4." />
                </Container>
                <Container name="Track5" position={[0, 0]} height={`${patchEditHeight}px`} visibilityContext={[{ index: trackContextId, value: Track5, condition: 'SHOW_WHEN' }]}>
                    <PatchEdit name="Track5" track={Track5} synthName="Track5" color={ColorTrack5} title="5." />
                </Container>
                <Container name="Track6" position={[0, 0]} height={`${patchEditHeight}px`} visibilityContext={[{ index: trackContextId, value: Track6, condition: 'SHOW_WHEN' }]}>
                    <PatchEdit name="Track6" track={Track6} synthName="Track6" color={ColorTrack6} title="6." />
                </Container>
                <Container name="Timeline" position={[0, patchEditHeight]} height="100%">
                    <TimelinePart />
                </Container>
            </View>
        </>
    ),
};
