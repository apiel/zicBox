import * as React from '@/libs/react';

import { Rect } from '@/libs/nativeComponents/Rect';
import { View } from '@/libs/nativeComponents/View';
import { MainKeys, MuteTracks } from '../components/Common';
import { Title } from '../components/Title';

export type Props = {
    viewName: string;
    content: any;
    color: string;
    synthName: string;
    title?: string;
};

export function Layout({ viewName, content, color, title, synthName }: Props) {
    return (
        <View name={viewName}>
            {title && <Title title={title} />}
            {title && <Rect bounds={[55, 28, 6, 6]} color={color} />}
            {content}
            <MuteTracks />
            <MainKeys synthName={synthName} viewName={viewName} />
        </View>
    );
}
