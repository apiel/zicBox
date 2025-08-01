import * as React from '@/libs/react';

import { MainView } from './MainView';

export type Props = {
    track: number;
    synthName: string;
    color: string;
    contextId: number;
    title: string;
};

// /!\ if only one view, we could simply move main view here...

export function SampleViews({ track, synthName, color, contextId, title }: Props) {
    return (
        <>
            <MainView name={synthName} track={track} synthName={synthName} color={color} title={title} />
        </>
    );
}
