import * as React from '@/libs/react';

import { SequencerCard } from '@/libs/nativeComponents/SequencerCard';
import { DrumLayout } from './Layout';

export type Props = {
    name: string;
    track: number;
    synthName: string;
    color: string;
};

export function FxView({ name, track, synthName, color }: Props) {
    return (
        <DrumLayout
            viewName={name}
            color={color}
            synthName={synthName}
            content={
                <>
                    <SequencerCard
                        bounds={[10, 80, 200, 100]}
                        audioPlugin={`Sequencer`}
                        track={track}
                        contextId={50}
                        rowsSelection={2}
                    />
                </>
            }
        />
    );
}
