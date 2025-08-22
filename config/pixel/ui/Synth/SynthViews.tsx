import * as React from '@/libs/react';

import { ClipView } from '../components/ClipView';
import { SeqView } from '../components/SeqView';
import { SynthLayout } from './SynthLayout';

export type Props = {
    track: number;
    synthName: string;
    color: string;
    contextId: number;
    title: string;
};

export function SynthViews({ track, synthName, color, contextId, title }: Props) {
    return (
        <>
            <SynthLayout name={synthName} track={track} synthName={synthName} color={color} title={title} />
            <SynthLayout name={`${synthName}:page2`} track={track} synthName={synthName} color={color} title={title} />
            <SynthLayout name={`${synthName}:page3`} track={track} synthName={synthName} color={color} title={title} />
            <SeqView
                name={`${synthName}Seq`}
                track={track}
                synthName={synthName}
                color={color}
                contextId={contextId}
                title={title}
                includeLength
            />
            <ClipView name={`${synthName}Clips`} track={track} synthName={synthName} color={color} title={title} />
        </>
    );
}
