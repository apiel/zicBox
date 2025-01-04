import * as React from '@/libs/react';

import { Value } from '@/libs/components/Value';
import { KeyInfoPosition, W1_4, W3_4 } from '../constants';

export function ValueBpm() {
    return (
        <Value
            value="Tempo BPM"
            position={[W3_4, KeyInfoPosition[1], W1_4, 22]}
            SHOW_LABEL_OVER_VALUE={0}
            BAR_HEIGHT={0}
            VALUE_FONT_SIZE={16}
        />
    );
}
