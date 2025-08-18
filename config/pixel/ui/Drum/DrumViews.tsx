import * as React from '@/libs/react';

import { ClipView } from '../components/ClipView';
import { GridSeqView } from '../components/GridSeqView';
import { DrumLayout } from './DrumLayout';

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
            <DrumLayout name={synthName} track={track} synthName={synthName} color={color} title={title} />
            <DrumLayout name={`${synthName}:page2`} track={track} synthName={synthName} color={color} title={title} />
            <DrumLayout name={`${synthName}:page3`} track={track} synthName={synthName} color={color} title={title} />
            <GridSeqView
                name={`${synthName}Seq`}
                track={track}
                synthName={synthName}
                color={color}
                contextId={contextId}
                title={title}
            />
            <ClipView name={`${synthName}Clips`} track={track} synthName={synthName} color={color} title={title} />
        </>
    );
}
