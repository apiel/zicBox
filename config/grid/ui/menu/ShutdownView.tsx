import * as React from '@/libs/react';

import { Text } from '@/libs/nativeComponents/Text';
import { View } from '@/libs/nativeComponents/View';
import { TextGrid } from '@/libs/tsComponents/TextGrid';
import { rgb } from '@/libs/ui';
import {
    A1,
    A10,
    A11,
    A12,
    A2,
    A3,
    A4,
    A5,
    A6,
    A7,
    A8,
    A9,
    B1,
    B10,
    B11,
    B12,
    B2,
    B3,
    B4,
    B5,
    B6,
    B7,
    B8,
    B9,
    C1,
    C10,
    C11,
    C12,
    C2,
    C3,
    C4,
    C5,
    C6,
    C7,
    C8,
    C9,
    ColorBtnOff,
    ColorButton, D1, D10,
    D11,
    D12, D2, D3,
    D4,
    D5,
    D6,
    D7,
    D8,
    D9, ScreenWidth, SelectorPosition
} from '../constants';

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
                    { key: D1, action: 'sh:halt -f' },
                    { key: D2, action: 'setView:&previous' },
                ]}
                 controllerColors={[
                    {
                        controller: 'Default',
                        colors: [
                            { key: A1, color: ColorBtnOff },
                            { key: A2, color: ColorBtnOff },
                            { key: A3, color: ColorBtnOff },
                            { key: A4, color: ColorBtnOff },
                            { key: A5, color: ColorBtnOff },
                            { key: A6, color: ColorBtnOff },
                            { key: A7, color: ColorBtnOff },
                            { key: A8, color: ColorBtnOff },
                            { key: A9, color: ColorBtnOff },
                            { key: A10, color: ColorBtnOff },
                            { key: A11, color: ColorBtnOff },
                            { key: A12, color: ColorBtnOff },

                            { key: B1, color: ColorBtnOff },
                            { key: B2, color: ColorBtnOff },
                            { key: B3, color: ColorBtnOff },
                            { key: B4, color: ColorBtnOff },
                            { key: B5, color: ColorBtnOff },
                            { key: B6, color: ColorBtnOff },
                            { key: B7, color: ColorBtnOff },
                            { key: B8, color: ColorBtnOff },
                            { key: B9, color: ColorBtnOff },
                            { key: B10, color: ColorBtnOff },
                            { key: B11, color: ColorBtnOff },
                            { key: B12, color: ColorBtnOff },

                            { key: C1, color: ColorBtnOff },
                            { key: C2, color: ColorBtnOff },
                            { key: C3, color: ColorBtnOff },
                            { key: C4, color: ColorBtnOff },
                            { key: C5, color: ColorBtnOff },
                            { key: C6, color: ColorBtnOff },
                            { key: C7, color: ColorBtnOff },
                            { key: C8, color: ColorBtnOff },
                            { key: C9, color: ColorBtnOff },
                            { key: C10, color: ColorBtnOff },
                            { key: C11, color: ColorBtnOff },
                            { key: C12, color: ColorBtnOff },

                            { key: D1, color: "#FF0000" },
                            { key: D2, color: ColorButton },
                            { key: D3, color: ColorBtnOff },
                            { key: D4, color: ColorBtnOff },
                            { key: D5, color: ColorBtnOff },
                            { key: D6, color: ColorBtnOff },
                            { key: D7, color: ColorBtnOff },
                            { key: D8, color: ColorBtnOff },
                            { key: D9, color: ColorBtnOff },
                            { key: D10, color: ColorBtnOff },
                            { key: D11, color: ColorBtnOff },
                            { key: D12, color: ColorBtnOff },
                        ],
                    },
                ]}
            />

            <TextGrid
                bounds={SelectorPosition}
                selected="Yes"
                selectedBackground={rgb(173, 99, 99)}
                rows={['&empty &empty &empty &empty &empty', 'Yes No &empty &empty &empty']}
            />
        </View>
    );
}
