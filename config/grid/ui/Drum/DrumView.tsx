import * as React from '@/libs/react';

import { Main2View } from './Main2View';
import { MainView } from './MainView';
import { SeqView } from './SeqView';

export type Props = {
    track: number;
    synthName: string;
    color: string;
};

export function DrumView({ track, synthName, color }: Props) {
    return (
        <>
            <MainView name={synthName} track={track} synthName={synthName} color={color} />
            <Main2View
                name={`${synthName}:page2`}
                track={track}
                synthName={synthName}
                color={color}
            />
            <SeqView name={`${synthName}Seq`} track={track} synthName={synthName} color={color} />
        </>
    );
}
