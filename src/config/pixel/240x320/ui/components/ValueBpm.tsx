import * as React from '@/libs/react';

import { Value } from '@/libs/components/Value';
import { KeyInfoPosition, W1_4, W3_4 } from '../constants';

export function ValueBpm() {
    return (
        <Value
            value="Tempo BPM"
            bounds={[W3_4, KeyInfoPosition[1] - 2, W1_4, 24]}
            SHOW_LABEL_OVER_VALUE={0}
            LABEL_OVER_VALUE_X={16}
            LABEL_FONT_SIZE={8}
            BAR_HEIGHT={0}
            VALUE_FONT_SIZE={16}
        />
    );
}
