import * as React from '@/libs/react';

import { Text } from '@/libs/nativeComponents/Text';
import { View } from '@/libs/nativeComponents/View';
import { TextGrid } from '@/libs/tsComponents/TextGrid';
import { rgb } from '@/libs/ui';
import { btn6, btn7, ScreenWidth, SelectorPosition } from '../constants';

export type Props = {
    name: string;
};

export function ShutdownView({ name }: Props) {
    return (
        <View name={name}>
            <Text
                bounds={[0, 0, ScreenWidth, 280]}
                text="Are you sure to shutdown?"
                color="#ffacac" //#ffacac
                centered
                keys={[
                    { key: btn6, action: 'sh:halt -f' },
                    { key: btn7, action: 'setView:Clips' },
                ]}
            />

            {/* <MainKeys synthName="Drum1" forcePatchView /> */}
            <TextGrid
                bounds={SelectorPosition}
                selected="Yes"
                selectedBackground={rgb(173, 99, 99)}
                rows={['&empty &empty &empty &empty &empty', 'Yes No &empty &empty &empty']}
            />
        </View>
    );
}
