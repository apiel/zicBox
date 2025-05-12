import * as React from '@/libs/react';

import { Text } from '@/libs/nativeComponents/Text';
import { View } from '@/libs/nativeComponents/View';
import { TextGrid } from '@/libs/tsComponents/TextGrid';
import { rgb } from '@/libs/ui';
import { btnUp, KeyInfoPosition, ScreenWidth } from '../constants';

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
                    { key: 'a', action: 'sh:filebrowser --port 80 -a 0.0.0.0 --noauth -r / &', action2: 'setView:Menu' },
                    { key: 'z', action: 'sh:filebrowser --port 80 -a 0.0.0.0 --noauth -r / &', action2: 'setView:Menu' },
                    { key: 'd', action: 'sh:killall filebrowser', action2: 'setView:Menu' },
                    { key: 'c', action: 'sh:killall filebrowser', action2: 'setView:Menu' },

                    { key: btnUp, action: 'setView:Menu' },
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
                rows={['&empty &empty &empty &empty', 'Start &empty Kill &empty']}
            />
        </View>
    );
}
