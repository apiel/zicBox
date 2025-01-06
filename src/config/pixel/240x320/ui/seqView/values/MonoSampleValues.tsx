import * as React from '@/libs/react';

import { ComponentProps } from '@/libs/components/component';
import { Log } from '@/libs/components/Log';
import { Rect } from '@/libs/components/Rect';
import { Sample } from '@/libs/components/Sample';
import { Value } from '@/libs/components/Value';
import { VisibilityContainer } from '@/libs/components/VisibilityContainer';
import { VisibilityContext } from '@/libs/components/VisibilityContext';
import { ScreenWidth } from '@/pixel/240x320/ui/constants';
import {
    bottomLeft,
    bottomRight,
    height,
    posContainerValues,
    posTopContainer,
    primary,
    quaternary,
    secondary,
    tertiary,
    topLeft,
    topRight
} from './constants';
import { SeqValues } from './SeqValues';

export type Props = Omit<ComponentProps, 'position'> & {
    group: number;
    track: number;
    context: number;
};

export function MonoSampleValues({ group, track, context }: Props) {
    return (
        <>
            <Log text="MonoSampleValues" />

            <VisibilityContainer position={posTopContainer} group={group}>
                <Sample
                    position={[0, 0, ScreenWidth, 70]}
                    track={track}
                    plugin="MonoSample SAMPLE_BUFFER SAMPLE_INDEX"
                    loop_points_color={'tertiary'}
                />
            </VisibilityContainer>

            <Edit group={group} track={track} context={context} />
            <Main group={group} track={track} context={context} />
            <SeqValues group={group} track={track} />
        </>
    );
}

function Main({ group, track, context }: Props) {
    return (
        <Container group={group} context={context} values={{ seq: 0, menu: 0 }}>
            <Value
                value="Volume VOLUME"
                position={topLeft}
                group={group}
                track={track}
                encoder_id={0}
                {...tertiary}
            />
            <Value
                value="Volume DRIVE"
                position={bottomLeft}
                group={group}
                track={track}
                encoder_id={1}
                {...primary}
            />
            <Value
                value="MMFilter CUTOFF"
                position={topRight}
                group={group}
                track={track}
                encoder_id={2}
                {...quaternary}
                USE_STRING_VALUE
            />
            <Value
                value="MMFilter RESONANCE"
                position={bottomRight}
                group={group}
                track={track}
                encoder_id={3}
                {...quaternary}
            />
        </Container>
    );
}

function Edit({ group, track, context }: Props) {
    return (
        <>
            <EditFx group={group} track={track} context={context} />
            <EditSample group={group} track={track} context={context} />
            <EditLoop group={group} track={track} context={context} />
        </>
    );
}

function EditFx({ group, track, context }: Props) {
    const topFull = [...topLeft];
    topFull[2] = ScreenWidth;
    return (
        <Container group={group} context={context} values={{ seq: 0, menu: 1 }}>
            <Rect position={[0, 0, ScreenWidth, height]} />

            <Value
                value="Volume GAIN_CLIPPING"
                position={topLeft}
                group={group}
                track={track}
                encoder_id={0}
                {...secondary}
            />
            <Value
                value="SampleRateReducer SAMPLE_STEP"
                position={topRight}
                group={group}
                track={track}
                encoder_id={2}
                {...quaternary}
            /> 
        </Container>
    );
}

function EditSample({ group, track, context }: Props) {
    const topFull = [...topLeft];
    topFull[2] = ScreenWidth;
    return (
        <Container group={group} context={context} values={{ seq: 0, menu: 2 }}>
            <Rect position={[0, 0, ScreenWidth, height]} />

            <Value
                value="MonoSample BROWSER"
                position={topFull}
                group={group}
                track={track}
                encoder_id={0}
                {...tertiary}
                // VALUE_FONT_SIZE={8}
            />
            <Value
                value="MonoSample START"
                position={bottomLeft}
                group={group}
                track={track}
                encoder_id={1}
                {...quaternary}
            />
            <Value
                value="MonoSample END"
                position={bottomRight}
                group={group}
                track={track}
                encoder_id={3}
                {...quaternary}
            />
        </Container>
    );
}

function EditLoop({ group, track, context }: Props) {
    return (
        <Container group={group} context={context} values={{ seq: 0, menu: 3 }}>
            <Rect position={[0, 0, ScreenWidth, height]} />

            <Value
                value="MonoSample LOOP_POSITION"
                position={topLeft}
                group={group}
                track={track}
                encoder_id={0}
                label="Position"
                {...tertiary}
            />
            <Value
                value="MonoSample LOOP_LENGTH"
                position={topRight}
                group={group}
                track={track}
                encoder_id={2}
                label="Length"
                {...tertiary}
            />
            <Value
                value="MonoSample LOOP_RELEASE"
                position={bottomLeft}
                group={group}
                track={track}
                encoder_id={1}
                {...quaternary}
            />
            {/* <Value
                value="MonoSample END"
                position={bottomRight}
                group={group}
                track={track}
                encoder_id={3}
                {...quaternary}
            /> */}
        </Container>
    );
}

function Container({
    group,
    context,
    values,
}: {
    group: number;
    context: number;
    values: { seq: number; menu: number };
}) {
    return (
        <VisibilityContainer position={posContainerValues} group={group}>
            <VisibilityContext index={10} condition="SHOW_WHEN" value={values.seq} />
            <VisibilityContext index={context} condition="SHOW_WHEN" value={values.menu} />
        </VisibilityContainer>
    );
}
