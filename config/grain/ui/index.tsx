import * as React from '@/libs/react';
import { ScreenHeight, ScreenWidth, W1_4, W2_4, W3_4 } from './constants';
import { enc1mini, enc2mini, enc3mini, enc4mini } from './constantsValue';
import { Layout } from './Layout';
import { Val } from './Val';

export const ui = {
    pixelController: 'pixel_12btn',
    // pixelController: 'grid',
    // i2c: ['pixel+_v1'],
    screen: {
        windowPosition: { x: 200, y: 300 },
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
    // renderer: 'FB',
    views: (
        <>
            <Layout
                viewName="Loop"
                content={
                    <>
                        <Val
                            {...enc1mini}
                            audioPlugin="SynthLoop"
                            param="FREQ"
                            color="secondary"
                            track={0}
                        />
                        <Val
                            {...enc2mini}
                            audioPlugin="SynthLoop"
                            param="RANGE"
                            track={0}
                            color="quaternary"
                        />
                        <Val
                            {...enc3mini}
                            audioPlugin="SynthLoop"
                            param="FX_TYPE"
                            color="tertiary"
                            track={0}
                        />
                        <Val
                            {...enc4mini}
                            audioPlugin="SynthLoop"
                            param="FX_AMOUNT"
                            track={0}
                            color="primary"
                        />
                    </>
                }
                color="primary"
                title="Grain"
            />
        </>
    ),
};
