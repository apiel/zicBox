import * as React from '@/libs/react';

import { Text } from '@/libs/nativeComponents/Text';
import { View } from '@/libs/nativeComponents/View';
import { TextGrid } from '@/libs/tsComponents/TextGrid';
import { rgb } from '@/libs/ui';
import { btn10, btn6, btn7, KeyInfoPosition, ScreenWidth } from '../constants';

export type Props = {
    name: string;
};

export function FileServerView({ name }: Props) {
    return (
        <View name={name}>
            <Text
                bounds={[0, 0, ScreenWidth, 20]}
                text="Web server for files..."
                centered
                keys={[
                    { key: btn6, action: 'sh:filebrowser --port 80 -a 0.0.0.0 --noauth -r / &', action2: 'setView:Clips' },
                    { key: btn7, action: 'sh:killall filebrowser', action2: 'setView:Clips' },

                    { key: btn10, action: 'setView:Clips' },
                ]}
            />

            <Text
                bounds={[0, 20, ScreenWidth, 20]}
                text="Running  on  http://zic.local:80/"
                color={rgb(156, 156, 156)}
                centered
            />

            <TextGrid
                bounds={KeyInfoPosition}
                rows={['&empty &empty &empty &empty &empty', 'Start Kill &empty &empty Exit']}
            />
        </View>
    );
}
