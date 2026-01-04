import * as React from '@/libs/react';

import { Rect } from '@/libs/nativeComponents/Rect';
import { StringVal } from '@/libs/nativeComponents/StringVal';
import { Text } from '@/libs/nativeComponents/Text';
import { View } from '@/libs/nativeComponents/View';

export type Props = {
    viewName: string;
    content: any;
    color: string;
    synthName?: string;
    title: string;
    noPrevious?: boolean;
};

export function Layout({ viewName, content, color, title, synthName, noPrevious }: Props) {
    return (
        <View name={viewName} {...(noPrevious && { noPrevious: true })}>
            {/* <Rect bounds={[W3_4, 0, W1_4, ScreenHeight]} /> */}
            {/* <Rect bounds={[0, 0, ScreenWidth, ScreenHeight]} /><------- clear background but should not be necessary!! */}
            <Rect bounds={[0, 0, 50, 16]} color={color} />
            <Text text={title} bounds={[2, 0, 40, 16]} font="PoppinsLight_12" bgColor={color} />
            {synthName && (
                <>
                    <StringVal
                        audioPlugin={synthName}
                        param="ENGINE"
                        bounds={[15, 0, 40, 16]}
                        fontLabel="PoppinsLight_12"
                        labelColor="#FFFFFF"
                        unit
                        bgColor={color}
                    />
                    <StringVal audioPlugin={synthName} param="ENGINE" bounds={[60, 0, 80, 16]} fontLabel="PoppinsLight_12" />
                </>
            )}
            {content}
        </View>
    );
}
