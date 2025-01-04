import * as React from '@/libs/react';

import { SeqProgressBar } from '@/libs/components/SeqProgressBar';
import { Value } from '@/libs/components/Value';
import { View } from '@/libs/components/View';
import { KeyInfoPosition, ScreenWidth, W1_4, W3_4 } from '@/pixel/240x320/ui/constants';
import { TextGridProgressBar, TextGridProgressBarShifted } from './TextGrid';

export type Props = {
    name: string;
};

export function SeqView({ name }: Props) {
    return (
        <View name={name}>
            <TextGridProgressBar />
            <TextGridProgressBarShifted />

            <Value
                value="Tempo BPM"
                position={[W3_4, KeyInfoPosition[1], W1_4, 22]}
                SHOW_LABEL_OVER_VALUE={0}
                BAR_HEIGHT={0}
                VALUE_FONT_SIZE={16}
            />

            <SeqProgressBar
                position={[0, 160, ScreenWidth, 5]}
                seq_plugin="Sequencer 1"
                track={0}
                volume_plugin="MasterVolume VOLUME"
                active_color="#23a123"
                group={0}
            />
        </View>
    );
}
