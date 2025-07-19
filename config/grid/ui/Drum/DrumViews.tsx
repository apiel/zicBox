import * as React from '@/libs/react';

import { Drum2 } from '../components/Common';
import { DrumSeqView } from '../components/DrumSeqView';
import { seqContextDrum2 } from '../constantsValue';
import { MainView } from './MainView';

export type Props = {
    track: number;
    synthName: string;
    color: string;
};

export function DrumViews({ track, synthName, color }: Props) {
    return (
        <>
            <MainView name={synthName} track={track} synthName={synthName} color={color} />
            <DrumSeqView
                name={`${synthName}Seq`}
                track={track}
                synthName={synthName}
                color={color}
                contextId={seqContextDrum2}
                title={Drum2}
            />
        </>
    );
}
