import * as React from '@/libs/react';

import { DrumSeqView } from '../components/DrumSeqView';
import { seqContextDrum1 } from '../constantsValue';
import { Main2View } from './Main2View';
import { MainView } from './MainView';

export type Props = {
    track: number;
    synthName: string;
    color: string;
};

export function KickViews({ track, synthName, color }: Props) {
    return (
        <>
            <MainView name={synthName} track={track} synthName={synthName} color={color} />
            <Main2View
                name={`${synthName}:page2`}
                track={track}
                synthName={synthName}
                color={color}
            />
            <DrumSeqView
                name={`${synthName}Seq`}
                track={track}
                synthName={synthName}
                color={color}
                contextId={seqContextDrum1}
            />
        </>
    );
}
