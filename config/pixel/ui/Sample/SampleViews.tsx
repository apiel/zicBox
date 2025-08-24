import * as React from '@/libs/react';

import { ClipView } from '../components/ClipView';
import { SeqView } from '../components/SeqView';
import { SampleLayout } from './SampleLayout';

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
            <SampleLayout
                name={synthName}
                track={track}
                synthName={synthName}
                color={color}
                title={title}
            />

            <SampleLayout
                name={`${synthName}:page2`}
                track={track}
                synthName={synthName}
                color={color}
                title={title}
            />

            <SampleLayout
                name={`${synthName}:page3`}
                track={track}
                synthName={synthName}
                color={color}
                title={title}
            />

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
