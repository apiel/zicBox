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

export function Page1() {
    return (
        <Layout
            viewName="Page1"
            content={
                <>
                    <Val
                        {...enc1mini}
                        audioPlugin="SynthLoop"
                        param="START"
                        color="secondary"
                        track={0}
                    />
                    <Val
                        {...enc2mini}
                        audioPlugin="SynthLoop"
                        param="END"
                        track={0}
                        color="quaternary"
                    />
                    {/* <Val
                        {...enc3mini}
                        bounds={[
                            enc3mini.bounds[0],
                            enc3mini.bounds[1],
                            W2_4 - 2,
                            enc3mini.bounds[3],
                        ]}
                        audioPlugin="SynthLoop"
                        param="BROWSER"
                        color="tertiary"
                        track={0}
                    /> */}
                    <Val
                        {...enc3mini}
                        audioPlugin="SynthLoop"
                        param="CHUNKS"
                        color="tertiary"
                        track={0}
                    />
                    <Val
                        {...enc4mini}
                        audioPlugin="SynthLoop"
                        param="BROWSER"
                        track={0}
                        color="primary"
                    />

                    <Val
                        {...enc5mini}
                        audioPlugin="SynthLoop"
                        param="GRAIN_FREQ"
                        color="secondary"
                        track={0}
                    />
                    <Val
                        {...enc6mini}
                        audioPlugin="SynthLoop"
                        param="GRAIN_RANGE"
                        track={0}
                        color="quaternary"
                    />
                    <Val
                        {...enc7mini}
                        audioPlugin="SynthLoop"
                        param="FX_TYPE"
                        color="tertiary"
                        track={0}
                    />
                    <Val
                        {...enc8mini}
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
    );
}
