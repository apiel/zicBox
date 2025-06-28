import * as React from '@/libs/react';

import { Drum1 } from '../components/Common';
import { Layout } from '../components/Layout';

export type Props = {
    viewName: string;
    content: any;
    color: string;
    hideTitle?: boolean;
    synthName: string;
};

export function DrumLayout({ viewName, content, color, hideTitle, synthName }: Props) {
    return (
        <Layout
            viewName={viewName}
            content={content}
            color={color}
            title={hideTitle ? undefined : Drum1}
            synthName={synthName}
        />
    );
}
