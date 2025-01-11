import * as React from '@/libs/react';

import { DrumEnvelop } from '@/libs/components/DrumEnvelop';
import { View } from '@/libs/components/View';
import { W3_4 } from '../constants';
import { height } from '../constantsValue';
import { TextGridCommon } from '../TextGridCommon';
import { TextGridPage2 } from './TextGridPage2';

export type Props = {
    name: string;
};

export function Page2View({ name }: Props) {
    return (
        <View name={name}>
             <DrumEnvelop
                 position={[0, 0, W3_4 - 2, height]}
                 plugin="Drum23"
                 envelop_data_id="0"
                 RENDER_TITLE_ON_TOP={false}
                 encoder_time={0}
                 encoder_phase={1}
                 encoder_modulation={2}
             />

            <TextGridPage2 />
            <TextGridCommon selected={1} />
        </View>
    );
}
