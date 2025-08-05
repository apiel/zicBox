import * as React from '@/libs/react';

import { Keyboard2 } from '@/libs/nativeComponents/Keyboard2';
import { View } from '@/libs/nativeComponents/View';
import { MainKeys } from '../components/Common';
import {
    A1,
    A10,
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
    B2,
    B3,
    B4,
    B5,
    B6,
    B7,
    B8,
    B9,
    btn10,
    btn2,
    btn4,
    btn5,
    btn6,
    btn7,
    btn8,
    btn9,
    C1,
    C10,
    C2,
    C3,
    C4,
    C5,
    C6,
    C7,
    C8,
    C9,
    D1,
    D10,
    D2,
    D3,
    D4,
    D5,
    D6,
    D7,
    D8,
    D9,
    ScreenWidth,
    shiftContext,
} from '../constants';

export type Props = {
    name: string;
};

export function CreateWorkspaceView({ name }: Props) {
    return (
        <View name={name}>
            <Keyboard2
                bounds={[0, 40, ScreenWidth, 300]}
                font="PoppinsLight_12"
                redirectView="Workspaces"
                audioPlugin="SerializeTrack"
                dataId="CREATE_WORKSPACE"
                shiftContextId={shiftContext}
                keys={[
                    { key: btn2, action: '.up' },
                    { key: btn4, action: '.done' },
                    { key: btn5, action: '.cancel' },
                    { key: btn6, action: '.left' },
                    { key: btn7, action: '.down' },
                    { key: btn8, action: '.right' },
                    { key: btn9, action: '.backspace' },
                    { key: btn10, action: '.type' },
                ]}
                keyLabels={[
                    '1',
                    '2',
                    '3',
                    '4',
                    '5',
                    '6',
                    '7',
                    '8',
                    '9',
                    '0',

                    'q',
                    'w',
                    'e',
                    'r',
                    't',
                    'y',
                    'u',
                    'i',
                    'o',
                    'p',

                    'a',
                    's',
                    'd',
                    'f',
                    'g',
                    'h',
                    'j',
                    'k',
                    'l',
                    '&icon::backspace::filled',

                    'z',
                    'x',
                    'c',
                    'v',
                    'b',
                    'n',
                    'm',

                    'Ok',
                    'Exit',
                    'Shift',
                ]}
                keyLabelsShifted={[
                    '-',
                    '=',
                    '_',
                    '.',
                    '!',
                    '%',
                    '&',
                    ':',
                    '$',
                    '#',

                    'Q',
                    'W',
                    'E',
                    'R',
                    'T',
                    'Y',
                    'U',
                    'I',
                    'O',
                    'P',

                    'A',
                    'S',
                    'D',
                    'F',
                    'G',
                    'H',
                    'J',
                    'K',
                    'L',
                    '&icon::backspace::filled',

                    'Z',
                    'X',
                    'C',
                    'V',
                    'B',
                    'N',
                    'M',
                    
                    'Ok',
                    'Exit',
                    'Shift',
                ]}
                keyColors={[
                    '#28535f',
                    '#28535f',
                    '#79a1a3',
                    '#79a1a3',
                    '#28535f',
                    '#28535f',
                    '#79a1a3',
                    '#79a1a3',
                    '#28535f',
                    '#28535f',

                    '#28535f',
                    '#28535f',
                    '#79a1a3',
                    '#79a1a3',
                    '#28535f',
                    '#28535f',
                    '#79a1a3',
                    '#79a1a3',
                    '#28535f',
                    '#28535f',

                    '#28535f',
                    '#28535f',
                    '#79a1a3',
                    '#79a1a3',
                    '#28535f',
                    '#28535f',
                    '#79a1a3',
                    '#79a1a3',
                    '#28535f',
                    '#ffAAAA',

                    '#28535f',
                    '#28535f',
                    '#79a1a3',
                    '#79a1a3',
                    '#28535f',
                    '#28535f',
                    '#79a1a3',
                    '#ffAAAA',
                    '#ffAAAA',
                    '#ffAAAA',
                ]}
                controllerColors={[
                    {
                        controller: 'Default',
                        colors: [
                            { key: A1, color: '#28535f' },
                            { key: A2, color: '#28535f' },
                            { key: A3, color: '#79a1a3' },
                            { key: A4, color: '#79a1a3' },
                            { key: A5, color: '#28535f' },
                            { key: A6, color: '#28535f' },
                            { key: A7, color: '#79a1a3' },
                            { key: A8, color: '#79a1a3' },
                            { key: A9, color: '#28535f' },
                            { key: A10, color: '#28535f' },

                            { key: B1, color: '#28535f' },
                            { key: B2, color: '#28535f' },
                            { key: B3, color: '#79a1a3' },
                            { key: B4, color: '#79a1a3' },
                            { key: B5, color: '#28535f' },
                            { key: B6, color: '#28535f' },
                            { key: B7, color: '#79a1a3' },
                            { key: B8, color: '#79a1a3' },
                            { key: B9, color: '#28535f' },
                            { key: B10, color: '#28535f' },

                            { key: C1, color: '#28535f' },
                            { key: C2, color: '#28535f' },
                            { key: C3, color: '#79a1a3' },
                            { key: C4, color: '#79a1a3' },
                            { key: C5, color: '#28535f' },
                            { key: C6, color: '#28535f' },
                            { key: C7, color: '#79a1a3' },
                            { key: C8, color: '#79a1a3' },
                            { key: C9, color: '#28535f' },
                            { key: C10, color: '#ffAAAA' },

                            { key: D1, color: '#28535f' },
                            { key: D2, color: '#28535f' },
                            { key: D3, color: '#79a1a3' },
                            { key: D4, color: '#79a1a3' },
                            { key: D5, color: '#28535f' },
                            { key: D6, color: '#28535f' },
                            { key: D7, color: '#79a1a3' },
                            { key: D8, color: '#ffAAAA' },
                            { key: D9, color: '#ffAAAA' },
                            { key: D10, color: '#ffAAAA' },
                        ],
                    },
                ]}
            />

            <MainKeys viewName={name} />
        </View>
    );
}
// Color color = { 0xaa, 0xcd, 0xcf };
// Color activeColor = { 0x02, 0x10, 0x14 };
