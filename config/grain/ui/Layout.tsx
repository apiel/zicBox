import * as React from '@/libs/react';

import { Rect } from '@/libs/nativeComponents/Rect';
import { Text } from '@/libs/nativeComponents/Text';
import { View } from '@/libs/nativeComponents/View';
import { darken } from '@/libs/ui';
import {
    C1,
    C4,
    menuTextColor,
    ScreenHeight,
    shiftContext,
    shiftVisibilityContext,
    unshiftVisibilityContext,
    W1_4,
    W3_4,
} from './constants';

export type Props = {
    viewName: string;
    content: any;
    color: string;
    synthName?: string;
    title?: string;
    noPrevious?: boolean;
};

export function Layout({ viewName, content, color, title, synthName, noPrevious }: Props) {
    const inactiveColor = darken(color, 0.5);
    return (
        <View name={viewName} {...(noPrevious && { noPrevious: true })}>
            {title && (
                <>
                    <Rect
                        bounds={[5, 6, 6, 6]}
                        color={!viewName.includes(':page') ? color : inactiveColor}
                    />
                    <Rect
                        bounds={[15, 6, 6, 6]}
                        color={viewName.includes(':page2') ? color : inactiveColor}
                    />
                    <Rect
                        bounds={[25, 6, 6, 6]}
                        color={viewName.includes(':page3') ? color : inactiveColor}
                    />
                    <Text
                        fontSize={16}
                        text={title}
                        bounds={[37, 0, 100, 16]}
                        font="PoppinsLight_8"
                    />
                </>
            )}
            {content}

            {/* <HiddenValue // When shifted
                keys={[
                    { key: C1, action: `playPause` },
                    {
                        key: C2,
                        action: `contextToggle:${shiftContext}:1:0`,
                        action2: `setView:Menu`,
                    },
                    // { key: C2, action: `setContext:${shiftContext}:0`, action2: `setView:Menu` },
                    // { key: C3, action: `audioEvent:RELOAD_VARIATION` },
                    { key: C4, action: `contextToggle:${shiftContext}:1:0` },
                ]}
                visibilityContext={[{ index: shiftContext, value: 1, condition: 'SHOW_WHEN' }]}
            /> */}

            <Rect
                bounds={[0, ScreenHeight - 18, W1_4, 16]}
                color="background"
                visibilityContext={[unshiftVisibilityContext]}
            />
            <Text
                text="&icon::play::filled"
                bounds={[0, ScreenHeight - 18, W1_4, 16]}
                centered={true}
                visibilityContext={[shiftVisibilityContext]}
                color={menuTextColor}
                keys={[{ key: C1, action: `playPause` }]}
            />

            <Text
                text="Shift"
                bounds={[W3_4, ScreenHeight - 20, W1_4, 16]}
                centered={true}
                color={menuTextColor}
                keys={[{ key: C4, action: `contextToggle:${shiftContext}:1:0` }]}
            />
        </View>
    );
}
