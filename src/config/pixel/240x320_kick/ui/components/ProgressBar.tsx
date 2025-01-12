import * as React from '@/libs/react';

import { SeqProgressBar } from '@/libs/components/SeqProgressBar';
import { ProgressPosition } from '../constants';

export function ProgressBar() {
    return (
        <SeqProgressBar
            position={ProgressPosition}
            seq_plugin="Sequencer 0"
            active_color="#23a123"
            selection_color="#23a123"
            volume_plugin="Volume VOLUME"
        />
    );
}
