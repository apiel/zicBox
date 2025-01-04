import * as React from '@/libs/react';

import { ComponentProps } from '@/libs/components/component';
import { GraphEncoder } from '@/libs/components/GraphEncoder';
import { Value } from '@/libs/components/Value';
import { VisibilityContainer } from '@/libs/components/VisibilityContainer';
import { VisibilityContext } from '@/libs/components/VisibilityContext';
import { ScreenWidth } from '../../constants';
import {
    bottomLeft,
    bottomRight,
    height,
    posContainer,
    primary,
    quaternary,
    secondary,
    tertiary,
    topLeft,
    topRight,
} from './constants';
import { SeqValues } from './SeqValues';

export type Props = Omit<ComponentProps, 'position'> & {
    group: number;
    track: number;
    context: number;
};

export function Drum23Values({ group, track, context }: Props) {
    return (
        <>
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
                encoderId={0}
                {...tertiary}
            />
            <Value
                value="Drum23 GAIN_CLIPPING"
                position={bottomLeft}
                group={group}
                track={track}
                encoderId={1}
                {...primary}
            />
            <Value
                value="MMFilter CUTOFF"
                position={topRight}
                group={group}
                track={track}
                encoderId={2}
                {...quaternary}
                USE_STRING_VALUE
            />
            <Value
                value="MMFilter RESONANCE"
                position={bottomRight}
                group={group}
                track={track}
                encoderId={3}
                {...quaternary}
            />
        </Container>
    );
}

function Edit({ group, track, context }: Props) {
    return (
        <>
            <EditDistortion group={group} track={track} context={context} menu={1} />
            <EditWaveform group={group} track={track} context={context} menu={2} />
        </>
    );
}

function EditDistortion({ group, track, context, menu }: Props & { menu: number }) {
    return (
        <Container group={group} context={context} values={{ seq: 0, menu }}>
            <Value
                value="Distortion WAVESHAPE"
                position={topLeft}
                group={group}
                track={track}
                encoderId={0}
                {...tertiary}
            />
            <Value
                value="Distortion DRIVE"
                position={bottomLeft}
                group={group}
                track={track}
                encoderId={1}
                {...primary}
            />
            <Value
                value="Distortion COMPRESS"
                position={topRight}
                group={group}
                track={track}
                encoderId={2}
                {...secondary}
            />
            <Value
                value="Distortion BASS"
                position={bottomRight}
                group={group}
                track={track}
                encoderId={3}
                {...quaternary}
            />
        </Container>
    );
}

function EditWaveform({ group, track, context, menu }: Props & { menu: number }) {
    return (
        <Container group={group} context={context} values={{ seq: 0, menu }}>
            <GraphEncoder
                position={[0, 0, ScreenWidth, height]}
                track={track}
                plugin="Drum23"
                data_id="WAVEFORM"
                RENDER_TITLE_ON_TOP={false}
                encoders={['0 WAVEFORM_TYPE', '2 MACRO', '1 SHAPE']}
            />
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
        <VisibilityContainer position={posContainer} group={group}>
            <VisibilityContext index={10} condition="SHOW_WHEN" value={values.seq} />
            <VisibilityContext index={context} condition="SHOW_WHEN" value={values.menu} />
        </VisibilityContainer>
    );
}
