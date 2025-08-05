import * as React from '@/libs/react';

import { Keyboard2 } from '@/libs/nativeComponents/Keyboard2';
import { View } from '@/libs/nativeComponents/View';
import { MainKeys } from '../components/Common';
import {
    btn10,
    btn2,
    btn4,
    btn5,
    btn6,
    btn7,
    btn8,
    btn9,
    ScreenWidth,
    shiftContext
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
                    'a',
                    'b',
                    'c',
                    'd',
                    'e',
                    'f',
                    'g',
                    'h',
                    'i',
                    'j',
                    'k',
                    'l',
                    'm',
                    'n',
                    'o',
                    'p',
                    'q',
                    'r',
                    's',
                    't',
                    'u',
                    'v',
                    'w',
                    'x',
                    'y',
                    'z',
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
                    '&icon::backspace::filled',
                    'Ok',
                    'Exit',
                    'Shift',
                ]}
                keyLabelsShifted={[
                    'A',
                    'B',
                    'C',
                    'D',
                    'E',
                    'F',
                    'G',
                    'H',
                    'I',
                    'J',
                    'K',
                    'L',
                    'M',
                    'N',
                    'O',
                    'P',
                    'Q',
                    'R',
                    'S',
                    'T',
                    'U',
                    'V',
                    'W',
                    'X',
                    'Y',
                    'Z',
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
                    '&icon::backspace::filled',
                    'Ok',
                    'Exit',
                    'Shift',
                ]}

                keyColors={[
                    'primary'
                ]}
            />

            <MainKeys viewName={name} />
        </View>
    );
}
