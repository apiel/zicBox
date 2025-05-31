import * as React from '@/libs/react';

import { Rect } from '@/libs/nativeComponents/Rect';
import { View } from '@/libs/nativeComponents/View';
import { Drum1, MainKeys, MuteTracks } from '../components/Common';
import { Title } from '../components/Title';

export type Props = {
    viewName: string;
    content: any;
    color: string;
    hideTitle?: boolean;
    synthName: string;
};

export function Layout({ viewName, content, color, hideTitle, synthName }: Props) {
    return (
        <View name={viewName}>
            {!hideTitle && <Title title={Drum1} />}
            {!hideTitle && <Rect bounds={[60, 28, 6, 6]} color={color} />}
            {content}
            <MuteTracks />
            <MainKeys synthName={synthName} />
        </View>
    );
}
