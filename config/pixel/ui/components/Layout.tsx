import * as React from '@/libs/react';

import { Rect } from '@/libs/nativeComponents/Rect';
import { Text } from '@/libs/nativeComponents/Text';
import { View } from '@/libs/nativeComponents/View';
import { darken } from '@/libs/ui';
import { MainKeys } from './Common';

export type Props = {
    viewName: string;
    content: any;
    color: string;
    synthName?: string;
    title?: string;
    skipMainKeys?: boolean;
};

export function Layout({
    viewName,
    content,
    color,
    title,
    synthName,
    skipMainKeys,
}: Props) {
    const inactiveColor = darken(color, 0.5);
    return (
        <View name={viewName}>
            {title && (
                <Text fontSize={16} text={title} bounds={[37, 0, 100, 16]} font="PoppinsLight_8" />
            )}
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
                </>
            )}
            {content}
            {!skipMainKeys && <MainKeys synthName={synthName} viewName={viewName} />}
        </View>
    );
}
