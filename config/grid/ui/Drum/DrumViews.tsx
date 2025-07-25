import * as React from '@/libs/react';

import { DrumSeqView } from '../components/DrumSeqView';
import { MainView } from './MainView';

export type Props = {
    track: number;
    synthName: string;
    color: string;
    contextId: number;
    title: string;
};

export function DrumViews({ track, synthName, color, contextId, title }: Props) {
    return (
        <>
            <MainView name={synthName} track={track} synthName={synthName} color={color} title={title} />
            <DrumSeqView
                name={`${synthName}Seq`}
                track={track}
                synthName={synthName}
                color={color}
                contextId={contextId}
                title={title}
            />
        </>
    );
}
