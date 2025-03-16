import * as React from '@/libs/react';

import { List } from '@/libs/nativeComponents/List';
import { View } from '@/libs/nativeComponents/View';
import { TextGrid } from '@/libs/tsComponents/TextGrid';
import { btn1, btn2, btn5, btn6, btn7, KeyInfoPosition, ScreenWidth } from '../constants';

export type Props = {
    name: string;
};

export function MenuView({ name }: Props) {
    return (
        <View name={name}>
            <List
                bounds={[0, 0, ScreenWidth, 280]}
                items={[
                    // 'Tape', 
                    'Workspaces', 
                    'Shutdown'
                ]}
                keys={[
                    { key: btn1, action: '.setView' },
                    { key: btn2, action: '.up' },
                    { key: btn5, action: '.setView' },
                    { key: btn6, action: '.down' },
                ]}
            />

            <TextGrid
                bounds={KeyInfoPosition}
                rows={[
                    '&empty &icon::arrowUp::filled &empty &empty',
                    'Select &icon::arrowDown::filled Exit &empty',
                ]}
                keys={[{ key: btn7, action: 'setView:Clips' }]}
            />
        </View>
    );
}
