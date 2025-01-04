import * as React from '@/libs/react';

import { ComponentProps } from '@/libs/components/component';
import { Rect } from '@/libs/components/Rect';
import { Value } from '@/libs/components/Value';
import { VisibilityContainer } from '@/libs/components/VisibilityContainer';
import { VisibilityContext } from '@/libs/components/VisibilityContext';
import { ScreenWidth } from '@/pixel/240x320/ui/constants';
import {
    bottomLeft,
    bottomRight,
    height,
    posY,
    primary,
    quaternary,
    tertiary,
    topLeft,
    topRight,
    valueH,
} from './constants';

export type Props = Omit<ComponentProps, 'position'> & {
    group: number;
    track: number;
    context: number;
};

export function DrumSampleValues({ group, track, context }: Props) {
    return (
        <>
            <Edit group={group} track={track} context={context} />
            <Main group={group} track={track} context={context} />
        </>
    );
}

function Main({ group, track, context }: Props) {
    return (
        <VisibilityContainer position={[0, posY, ScreenWidth, valueH * 2 + 2]} group={group}>
            <VisibilityContext index={10} condition="SHOW_WHEN" value={0} />
            <VisibilityContext index={context} condition="SHOW_WHEN" value={0} />

            <Value
                value="Volume VOLUME"
                position={topLeft}
                group={group}
                track={track}
                encoderId={0}
                {...tertiary}
            />
            <Value
                value="Volume GAIN_CLIPPING"
                position={bottomLeft}
                group={group}
                track={track}
                encoderId={1}
                {...primary}
            />
            <Value
                value="Volume DRIVE"
                position={topRight}
                group={group}
                track={track}
                encoderId={2}
                {...quaternary}
            />
            <Value
                value="SampleRateReducer SAMPLE_STEP"
                position={bottomRight}
                group={group}
                track={track}
                encoderId={3}
                {...quaternary}
            />
        </VisibilityContainer>
    );
}

function Edit({ group, track, context }: Props) {
    return (
        <VisibilityContainer position={[0, posY, ScreenWidth, valueH * 2 + 2]} group={group}>
            <VisibilityContext index={10} condition="SHOW_WHEN" value={0} />
            <VisibilityContext index={context} condition="SHOW_WHEN" value={1} />

            <Rect position={[0, 0, ScreenWidth, height]} />

            <Value
                value="DrumSample BROWSER"
                position={topLeft}
                group={group}
                track={track}
                encoderId={0}
                {...tertiary}
                // VALUE_FONT_SIZE={8}
            />
            <Value
                value="DrumSample START"
                position={bottomLeft}
                group={group}
                track={track}
                encoderId={1}
                {...quaternary}
            />
            <Value
                value="DrumSample END"
                position={bottomRight}
                group={group}
                track={track}
                encoderId={3}
                {...quaternary}
            />
        </VisibilityContainer>
    );
}
