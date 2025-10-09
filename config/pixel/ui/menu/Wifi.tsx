import * as React from '@/libs/react';

import { Text } from '@/libs/nativeComponents/Text';
import { Wifi } from '@/libs/nativeComponents/Wifi';
import { rgb } from '@/libs/ui';
import { Layout } from '../components/Layout';
import {
    C1,
    C2,
    C3,
    C4,
    menuTextColor,
    ScreenHeight,
    ScreenWidth,
    W1_4,
    W2_4,
    W3_4,
    wifiContext,
} from '../constants';
import { enc1, enc2, enc3 } from '../constantsValue';

export type Props = {
    name: string;
};

export function WifiView({ name }: Props) {
    return (
        <Layout
            viewName={name}
            color={rgb(170, 170, 170)}
            title="Wifi"
            noPrevious
            content={
                <>
                    <Wifi
                        bounds={[10, 70, ScreenWidth - 20, 40]}
                        keys={[
                            { key: C1, action: `.backspace` },
                            { key: C2, action: `.connect` },
                            { key: C3, action: `.scan` },
                        ]}
                        ssidEncoderId={enc1.encoderId}
                        passwordEncoderId={enc2.encoderId}
                        cursorEncoderId={enc3.encoderId}
                        contextId={wifiContext}
                    />
                    <Text
                        text="&icon::backspace::filled"
                        bounds={[0, ScreenHeight -18, W1_4, 18]}
                        centered={true}
                        color={menuTextColor}
                    />
                    <Text
                        text="Connect"
                        bounds={[W1_4, ScreenHeight - 20, W1_4, 18]}
                        centered={true}
                        color={menuTextColor}
                    />
                    <Text
                        text="Refresh"
                        bounds={[W2_4, ScreenHeight - 20, W1_4, 18]}
                        centered={true}
                        color={menuTextColor}
                    />
                    <Text
                        text="Exit"
                        bounds={[W3_4, ScreenHeight - 20, W1_4, 18]}
                        centered={true}
                        color={menuTextColor}
                        keys={[{ key: C4, action: `setView:&previous` }]}
                    />
                </>
            }
        />
    );
}
