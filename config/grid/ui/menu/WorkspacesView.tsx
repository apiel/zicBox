import * as React from '@/libs/react';

import { HiddenValue } from '@/libs/nativeComponents/HiddenValue';
import { View } from '@/libs/nativeComponents/View';
import { Workspaces } from '@/libs/nativeComponents/Workspaces';
import { TextGrid } from '@/libs/tsComponents/TextGrid';
import { rgb } from '@/libs/ui';
import { workspaceFolder } from '../../audio';
import { MainKeys } from '../components/Common';
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
    ColorButton,
    D1,
    D10,
    D11,
    D12,
    D2,
    D3,
    D4,
    D5,
    D6,
    D7,
    D8,
    D9,
    ScreenWidth,
    SelectorPosition,
} from '../constants';

export type Props = {
    name: string;
};

export function WorkspacesView({ name }: Props) {
    return (
        <View name={name}>
            <Workspaces
                workspaceFolder={workspaceFolder}
                audioPlugin="SerializeTrack"
                bounds={[0, 0, ScreenWidth, 280]}
                keys={[
                    { key: D1, action: '.data:LOAD_WORKSPACE', context: { id: 253, value: 0 } },
                    { key: C2, action: '.up', context: { id: 253, value: 0 } },
                    { key: D2, action: '.down', context: { id: 253, value: 0 } },

                    { key: D1, action: '.delete', context: { id: 253, value: 1 } },
                ]}
            />

            <TextGrid
                bounds={SelectorPosition}
                rows={[
                    '&empty &icon::arrowUp::filled &empty &empty &empty',
                    'Use &icon::arrowDown::filled Exit New &icon::trash',
                ]}
                keys={[
                    { key: D3, action: 'setView:#track' },
                    { key: D4, action: 'setView:CreateWorkspace' },
                    { key: D5, action: 'contextToggle:253:1:0' },
                ]}
                context={[{ index: 253, value: 0, condition: 'SHOW_WHEN' }]}
            />

            <TextGrid
                bounds={SelectorPosition}
                selected="Delete?"
                selectedBackground={rgb(173, 99, 99)}
                rows={[
                    'Delete? &empty &empty &empty &empty',
                    'Yes &empty &empty &empty ^&icon::trash',
                ]}
                context={[{ index: 253, value: 1, condition: 'SHOW_WHEN' }]}
                keys={[{ key: D5, action: 'contextToggle:253:1:0' }]}
            />

            <HiddenValue
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
                            { key: C2, color: ColorButton },
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

                            { key: D1, color: ColorButton },
                            { key: D2, color: ColorButton },
                            { key: D3, color: ColorButton },
                            { key: D4, color: ColorButton },
                            { key: D5, color: ColorButton },
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
                visibilityContext={[{ index: 253, value: 0, condition: 'SHOW_WHEN' }]}
            />

            <HiddenValue
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

                            { key: D1, color: '#FF0000' },
                            { key: D2, color: ColorBtnOff },
                            { key: D3, color: ColorBtnOff },
                            { key: D4, color: ColorBtnOff },
                            { key: D5, color: ColorButton },
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
                visibilityContext={[{ index: 253, value: 1, condition: 'SHOW_WHEN' }]}
            />

            <MainKeys synthName="Drum1" forcePatchView viewName={name} />
        </View>
    );
}
