import * as React from '@/libs/react';

import { ComponentProps } from '@/libs/components/component';
import { DrumEnvelop } from '@/libs/components/DrumEnvelop';
import { GraphEncoder } from '@/libs/components/GraphEncoder';
import { KnobValue } from '@/libs/components/KnobValue';
import { Log } from '@/libs/components/Log';
import { Rect } from '@/libs/components/Rect';
import { Spectrogram } from '@/libs/components/Spectrogram';
import { Value } from '@/libs/components/Value';
import { VisibilityContainer } from '@/libs/components/VisibilityContainer';
import { VisibilityContext } from '@/libs/components/VisibilityContext';
import { ScreenWidth, W1_4, W3_4 } from '../../constants';
import {
    bottomLeft,
    bottomRight,
    encoderH,
    height,
    posContainerValues,
    posTopContainer,
    primary,
    quaternary,
    secondary,
    tertiary,
    topLeft,
    topRight,
} from './constants';
import { SeqValues } from './SeqValues';

export type Props = Omit<ComponentProps, 'bounds'> & {
    group: number;
    track: number;
    context: number;
};

export function Drum23Values({ group, track, context }: Props) {
    return (
        <>
            <Log text="Drum23Values" />
            
            <VisibilityContainer bounds={posTopContainer} group={group}>
                <Spectrogram
                    bounds={[0, 0, ScreenWidth, 70]}
                    track={0}
                    data="Spectrogram BUFFER"
                    text="Pixel"
                    raw_buffer
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
                bounds={topLeft}
                group={group}
                track={track}
                encoder_id={0}
                {...tertiary}
            />
            <Value
                value="Drum23 GAIN_CLIPPING"
                bounds={bottomLeft}
                group={group}
                track={track}
                encoder_id={1}
                {...primary}
            />
            <Value
                value="MMFilter CUTOFF"
                bounds={topRight}
                group={group}
                track={track}
                encoder_id={2}
                {...quaternary}
                USE_STRING_VALUE
            />
            <Value
                value="MMFilter RESONANCE"
                bounds={bottomRight}
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
            <EditDistortion group={group} track={track} context={context} menu={1} />
            <EditWaveform group={group} track={track} context={context} menu={2} />
            <EditEnvAmp group={group} track={track} context={context} menu={3} />
            <EditEnvFreq group={group} track={track} context={context} menu={4} />
            <EditClick group={group} track={track} context={context} menu={5} />
        </>
    );
}

function EditDistortion({ group, track, context, menu }: Props & { menu: number }) {
    return (
        <Container group={group} context={context} values={{ seq: 0, menu }}>
            <Value
                value="Distortion WAVESHAPE"
                bounds={topLeft}
                group={group}
                track={track}
                encoder_id={0}
                {...tertiary}
            />
            <Value
                value="Distortion DRIVE"
                bounds={bottomLeft}
                group={group}
                track={track}
                encoder_id={1}
                {...primary}
            />
            <Value
                value="Distortion COMPRESS"
                bounds={topRight}
                group={group}
                track={track}
                encoder_id={2}
                {...secondary}
            />
            <Value
                value="Distortion BASS"
                bounds={bottomRight}
                group={group}
                track={track}
                encoder_id={3}
                {...quaternary}
            />
        </Container>
    );
}

function EditWaveform({ group, track, context, menu }: Props & { menu: number }) {
    return (
        <Container group={group} context={context} values={{ seq: 0, menu }}>
            <GraphEncoder
                bounds={[0, 0, ScreenWidth, height]}
                track={track}
                plugin="Drum23"
                data_id="WAVEFORM"
                RENDER_TITLE_ON_TOP={false}
                encoders={['0 WAVEFORM_TYPE', '2 MACRO', '1 SHAPE']}
            />
        </Container>
    );
}

function EditEnvAmp({ group, track, context, menu }: Props & { menu: number }) {
    return (
        <Container group={group} context={context} values={{ seq: 0, menu }}>
            <Rect bounds={[0, 0, ScreenWidth, height]} />

            <DrumEnvelop
                bounds={[0, 0, W3_4 - 2, height]}
                track={track}
                plugin="Drum23"
                envelop_data_id="0"
                RENDER_TITLE_ON_TOP={false}
                encoder_time={0}
                encoder_phase={1}
                encoder_modulation={2}
            />

            <KnobValue
                bounds={[W3_4, (height - encoderH) * 0.5, W1_4, encoderH]}
                track={track}
                value="Drum23 DURATION"
                encoder_id={3}
                color="quaternary"
            />
        </Container>
    );
}

function EditEnvFreq({ group, track, context, menu }: Props & { menu: number }) {
    return (
        <Container group={group} context={context} values={{ seq: 0, menu }}>
            <Rect bounds={[0, 0, ScreenWidth, height]} />

            <DrumEnvelop
                bounds={[0, 0, W3_4 - 2, height]}
                track={track}
                plugin="Drum23"
                envelop_data_id="4"
                RENDER_TITLE_ON_TOP={false}
                encoder_time={0}
                encoder_phase={1}
                encoder_modulation={2}
            />

            <KnobValue
                bounds={[W3_4, (height - encoderH) * 0.5, W1_4, encoderH]}
                track={track}
                value="Drum23 PITCH"
                encoder_id={3}
                color="secondary"
            />
        </Container>
    );
}

function EditClick({ group, track, context, menu }: Props & { menu: number }) {
    return (
        <Container group={group} context={context} values={{ seq: 0, menu }}>
            <Value
                value="Drum23 CLICK"
                bounds={topLeft}
                group={group}
                track={track}
                encoder_id={0}
                {...tertiary}
            />
            <Value
                value="Drum23 CLICK_DURATION"
                bounds={bottomLeft}
                group={group}
                track={track}
                encoder_id={1}
                {...primary}
            />
            <Value
                value="Drum23 CLICK_CUTOFF"
                bounds={topRight}
                group={group}
                track={track}
                encoder_id={2}
                {...secondary}
                USE_STRING_VALUE
            />
            <Value
                value="Drum23 CLICK_RESONANCE"
                bounds={bottomRight}
                group={group}
                track={track}
                encoder_id={3}
                {...quaternary}
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
        <VisibilityContainer bounds={posContainerValues} group={group}>
            <VisibilityContext index={10} condition="SHOW_WHEN" value={values.seq} />
            <VisibilityContext index={context} condition="SHOW_WHEN" value={values.menu} />
        </VisibilityContainer>
    );
}
