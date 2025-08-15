import * as React from '@/libs/react';

import { Rect } from '@/libs/nativeComponents/Rect';
import { View } from '@/libs/nativeComponents/View';
import { MainKeys } from './Common';
import { Title } from './Title';

export type Props = {
    viewName: string;
    content: any;
    color: string;
    synthName?: string;
    title?: string;
};

export function Layout({ viewName, content, color, title, synthName }: Props) {
    return (
        <View name={viewName}>
            {title && <Title title={title} />}
            {title && <Rect bounds={[5, 6, 6, 6]} color={color} />}
            {content}
            <MainKeys synthName={synthName} viewName={viewName} />
        </View>
    );
}
