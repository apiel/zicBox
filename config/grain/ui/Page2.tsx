import * as React from '@/libs/react';

import {
    enc1mini,
    enc2mini,
    enc3mini,
    enc4mini,
    enc5mini,
    enc6mini,
    enc7mini,
    enc8mini,
} from './constantsValue';
import { Layout } from './Layout';
import { Val } from './Val';

export function Page2() {
    return (
        <Layout
            viewName="Page2"
            content={
                <>
                    <Val
                        {...enc1mini}
                        audioPlugin="SynthLoop"
                        param="GRAIN_LENGTH"
                        color="secondary"
                        track={0}
                    />
                    <Val
                        {...enc2mini}
                        audioPlugin="SynthLoop"
                        param="DENSITY"
                        track={0}
                        color="quaternary"
                    />
                    <Val
                        {...enc3mini}
                        audioPlugin="SynthLoop"
                        param="DENSITY_DELAY"
                        color="tertiary"
                        track={0}
                    />
                    <Val
                        {...enc4mini}
                        audioPlugin="SynthLoop"
                        param="DELAY_RANDOMIZE"
                        track={0}
                        color="primary"
                    />

                    <Val
                        {...enc5mini}
                        audioPlugin="SynthLoop"
                        param="DETUNE"
                        color="secondary"
                        track={0}
                    />
                    <Val
                        {...enc6mini}
                        audioPlugin="SynthLoop"
                        param="PITCH_RANDOMIZE"
                        track={0}
                        color="quaternary"
                    />
                    <Val
                        {...enc7mini}
                        audioPlugin="SynthLoop"
                        param="DIRECTION"
                        color="tertiary"
                        track={0}
                    />
                    {/* <Val
                        {...enc8mini}
                        audioPlugin="SynthLoop"
                        param="FX_AMOUNT"
                        track={0}
                        color="primary"
                    /> */}
                    <Val
                        {...enc8mini}
                        audioPlugin="SynthLoop"
                        param="CHUNKS"
                        color="tertiary"
                        track={0}
                    />
                </>
            }
            color="primary"
            title="Grain"
        />
    );
}
