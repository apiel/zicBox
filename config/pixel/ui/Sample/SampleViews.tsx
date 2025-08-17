import * as React from '@/libs/react';

import { ClipView } from '../components/ClipView';
import { GridSeqView } from '../components/GridSeqView';
import { Main2View } from './Main2View';
import { Main3View } from './Main3View';
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
            <MainView
                name={synthName}
                track={track}
                synthName={synthName}
                color={color}
                title={title}
            />

            <Main2View
                name={`${synthName}:page2`}
                track={track}
                synthName={synthName}
                color={color}
                title={title}
            />

            <Main3View
                name={`${synthName}:page3`}
                track={track}
                synthName={synthName}
                color={color}
                title={title}
            />

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
