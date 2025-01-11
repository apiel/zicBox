import * as React from '@/libs/react';

import { GraphEncoder } from '@/libs/components/GraphEncoder';
import { View } from '@/libs/components/View';
import { ScreenWidth } from '../constants';
import { height } from '../constantsValue';
import { TextGridCommon } from '../TextGridCommon';
import { TextGridPage1 } from './TextGridPage1';

export type Props = {
    name: string;
};

export function WaveformView({ name }: Props) {
    return (
        <View name={name}>
            <GraphEncoder
                position={[0, 0, ScreenWidth, height]}
                plugin="Drum23"
                data_id="WAVEFORM"
                RENDER_TITLE_ON_TOP={false}
                encoders={['0 WAVEFORM_TYPE', '2 MACRO', '1 SHAPE']}
            />
            <TextGridPage1 />
            <TextGridCommon selected={0} />
        </View>
    );
}
