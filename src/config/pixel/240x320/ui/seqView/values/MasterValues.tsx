import * as React from '@/libs/react';

import { Rect } from '@/libs/components/Rect';
import { Value } from '@/libs/components/Value';
import { VisibilityContainer } from '@/libs/components/VisibilityContainer';
import { ScreenWidth } from '@/pixel/240x320/ui/constants';
import {
    bottomRight,
    height,
    posContainer,
    quaternary,
    tertiary,
    topLeft,
    topRight
} from './constants';

export function MasterValues() {
    return (
        <VisibilityContainer position={posContainer} group={0}>
            <Rect position={[0, 0, ScreenWidth, height]} />
            <Value
                value="MasterVolume VOLUME"
                position={topLeft}
                group={0}
                track={0}
                encoderId={0}
                {...tertiary}
            />
            <Value
                value="MasterFilter CUTOFF"
                position={topRight}
                group={0}
                track={0}
                encoderId={2}
                {...quaternary}
                USE_STRING_VALUE
            />
            <Value
                value="MasterFilter RESONANCE"
                position={bottomRight}
                group={0}
                track={0}
                encoderId={3}
                {...quaternary}
            />
        </VisibilityContainer>
    );
}
