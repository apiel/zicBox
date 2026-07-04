import * as React from '@/libs/react';

import { Rect } from '@/libs/nativeComponents/Rect';
import { Text } from '@/libs/nativeComponents/Text';
import { View } from '@/libs/nativeComponents/View';
import { darken } from '@/libs/ui';
import {
    C1,
    C2,
    C3,
    C4,
    menuTextColor,
    ScreenHeight,
    shiftContext,
    shiftVisibilityContext,
    unshiftVisibilityContext,
    W1_4,
    W2_4,
    W3_4
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

            {/* <NoteGrid
                audioPlugin="SynthLoop"
                track={MasterTrack}
                bounds={[20, ScreenHeight - 50, ScreenWidth - 40, 40]}
                rows={2}
                keys={[
                    { key: A1, action: `.key:0` },
                    { key: A2, action: `.key:1` },
                    { key: A3, action: `.key:2` },
                    { key: A4, action: `.key:3` },

                    { key: B1, action: `.key:4` },
                    { key: B2, action: `.key:5` },
                    { key: B3, action: `.key:6` },
                    { key: B4, action: `.key:7` },
                ]}
            /> */}

            <Text
                text="Page 1"
                bounds={[0, ScreenHeight - 18, W1_4, 16]}
                centered={true}
                visibilityContext={[unshiftVisibilityContext]}
                color={menuTextColor}
                keys={[{ key: C1, action: `setView:Page1` }]}
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
                text="Page 2"
                bounds={[W1_4, ScreenHeight - 18, W1_4, 16]}
                centered={true}
                visibilityContext={[unshiftVisibilityContext]}
                color={menuTextColor}
                keys={[{ key: C2, action: `setView:Page2` }]}
            />
            <Rect
                bounds={[W1_4, ScreenHeight - 18, W1_4, 16]}
                color="background"
                visibilityContext={[shiftVisibilityContext]}
            />

            <Text
                text="Page 3"
                bounds={[W2_4, ScreenHeight - 18, W1_4, 16]}
                centered={true}
                visibilityContext={[unshiftVisibilityContext]}
                color={menuTextColor}
                keys={[{ key: C3, action: `setView:Page3` }]}
            />
            <Rect
                bounds={[W2_4, ScreenHeight - 18, W1_4, 16]}
                color="background"
                visibilityContext={[shiftVisibilityContext]}
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
