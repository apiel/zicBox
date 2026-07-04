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

export function Page3() {
    return (
        <Layout
            viewName="Page3"
            content={
                <>
                    <Val
                        {...enc1mini}
                        audioPlugin="SynthLoop"
                        param="MIX"
                        color="secondary"
                        track={0}
                    />
                    <Val
                        {...enc2mini}
                        audioPlugin="SynthLoop"
                        param="FREQ"
                        track={0}
                        color="quaternary"
                    />
                    <Val
                        {...enc3mini}
                        audioPlugin="SynthLoop"
                        param="RANGE"
                        color="tertiary"
                        track={0}
                    />
                    <Val
                        {...enc4mini}
                        audioPlugin="SynthLoop"
                        param="YO"
                        track={0}
                        color="primary"
                    />

                    <Val
                        {...enc5mini}
                        audioPlugin="SynthLoop"
                        param="YO"
                        color="secondary"
                        track={0}
                    />
                    <Val
                        {...enc6mini}
                        audioPlugin="SynthLoop"
                        param="YO"
                        track={0}
                        color="quaternary"
                    />
                    <Val
                        {...enc7mini}
                        audioPlugin="SynthLoop"
                        param="YO"
                        color="tertiary"
                        track={0}
                    />
                    <Val
                        {...enc8mini}
                        audioPlugin="SynthLoop"
                        param="YO"
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
