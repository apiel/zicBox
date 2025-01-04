import * as React from '@/libs/react';

import { ComponentProps } from '@/libs/components/component';
import { Value } from '@/libs/components/Value';
import { VisibilityContainer } from '@/libs/components/VisibilityContainer';
import { VisibilityContext } from '@/libs/components/VisibilityContext';
import {
    bottomLeft,
    bottomRight,
    posContainer,
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
        <VisibilityContainer position={posContainer} group={group}>
            <VisibilityContext index={10} condition="SHOW_WHEN_OVER" value={0} />
            <Value
                value="Sequencer STEP_NOTE"
                position={topLeft}
                group={group}
                track={track}
                encoderId={0}
                {...tertiary}
            />
            <Value
                value="Sequencer STEP_CONDITION"
                position={bottomLeft}
                group={group}
                track={track}
                encoderId={1}
                {...primary}
            />
            <Value
                value="Sequencer STEP_VELOCITY"
                position={topRight}
                group={group}
                track={track}
                encoderId={2}
                {...secondary}
            />
            <Value
                value="Sequencer STEP_MOTION"
                position={bottomRight}
                group={group}
                track={track}
                encoderId={3}
                {...quaternary}
            />
        </VisibilityContainer>
    );
}
