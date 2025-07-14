import * as React from '@/libs/react';

import { StepEditSample } from '@/libs/nativeComponents/StepEditSample';
import { View } from '@/libs/nativeComponents/View';

export type Props = {
    name: string;
};

export function DrumSeqView({
    name,
}: Props) {
    return (
        <View name={name}>
            <StepEditSample audioPlugin="Sample1" stepIndex={0} bounds={[0, 0, 120, 10]} />
        </View>
    );
}
