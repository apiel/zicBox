import * as React from '@/libs/react';

import { ComponentProps } from '@/libs/components/component';
import { Value } from '@/libs/components/Value';
import { VisibilityContainer } from '@/libs/components/VisibilityContainer';
import { VisibilityContext } from '@/libs/components/VisibilityContext';
import {
    bottomLeft,
    bottomRight,
    posContainerValues,
    primary,
    quaternary,
    secondary,
    tertiary,
    topLeft,
    topRight
} from './constants';

export type Props = Omit<ComponentProps, 'position'> & {
    group: number;
    track: number;
};

export function SeqValues({ group, track }: Props) {
    return (
        <VisibilityContainer position={posContainerValues} group={group}>
            <VisibilityContext index={10} condition="SHOW_WHEN_OVER" value={0} />
            <Value
                value="Sequencer STEP_NOTE"
                position={topLeft}
                group={group}
                track={track}
                encoder_id={0}
                {...tertiary}
            />
            <Value
                value="Sequencer STEP_CONDITION"
                position={bottomLeft}
                group={group}
                track={track}
                encoder_id={1}
                {...primary}
            />
            <Value
                value="Sequencer STEP_VELOCITY"
                position={topRight}
                group={group}
                track={track}
                encoder_id={2}
                {...secondary}
            />
            <Value
                value="Sequencer STEP_MOTION"
                position={bottomRight}
                group={group}
                track={track}
                encoder_id={3}
                {...quaternary}
            />
        </VisibilityContainer>
    );
}
