import * as React from '@/libs/react';

import { GridSeqView } from '../components/GridSeqView';
import { Main2View } from './Main2View';
import { MainView } from './MainView';

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
            <GridSeqView
                name={`${synthName}Seq`}
                track={track}
                synthName={synthName}
                color={color}
                contextId={contextId}
                title={title}
                includeLength
            />
        </>
    );
}
