import * as React from '@/libs/react';

import { Value } from '@/libs/components/Value';
import { VisibilityContainer } from '@/libs/components/VisibilityContainer';
import { PrimaryBar, QuaternaryBar, ScreenWidth, SecondaryBar, TertiaryBar, W2_4 } from '../constants';

const valueH = 29;
const rowSpacing = 8;

const topLeft = [0, 0, W2_4 - 2, valueH];
const bottomLeft = [0, valueH + rowSpacing, W2_4 - 2, valueH];
const topRight = [W2_4, 0, W2_4 - 2, valueH];
const bottomRight = [W2_4, valueH + rowSpacing, W2_4 - 2, valueH];

const posContainer = [0, 222, ScreenWidth, valueH * 2 + 2];

const base = {
    SHOW_LABEL_OVER_VALUE: 6,
    LABEL_FONT_SIZE: 8,
};

export const primary = {
    LABEL_COLOR: 'primary',
    BAR_COLOR: PrimaryBar,
    ...base,
};

export const secondary = {
    LABEL_COLOR: 'secondary',
    BAR_COLOR: SecondaryBar,
    ...base,
};

export const tertiary = {
    LABEL_COLOR: 'tertiary',
    BAR_COLOR: TertiaryBar,
    ...base,
};

export const quaternary = {
    LABEL_COLOR: 'quaternary',
    BAR_COLOR: QuaternaryBar,
    ...base,
};

// export function Drum23Values({ group, track }: { group: number; track: number }) {
//     return (
//             <Text text="Drum23Values" position={posContainer} />
//     );
// }

export function Drum23Values({ group, track }: { group: number; track: number }) {
    return (
        <VisibilityContainer position={posContainer} group={group}>
            <Value
                value="Volume VOLUME"
                position={topLeft}
                group={group}
                track={track}
                encoder_id={0}
                {...tertiary}
            />
            <Value
                value="Drum23 GAIN_CLIPPING"
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
        </VisibilityContainer>
    );
}

export function SampleValues({ group, track }: { group: number; track: number }) {
    return (
        <VisibilityContainer position={posContainer} group={group}>
            <Value
                value="Volume VOLUME"
                position={topLeft}
                group={group}
                track={track}
                encoder_id={0}
                {...tertiary}
            />
            <Value
                value="Volume GAIN_CLIPPING"
                position={bottomLeft}
                group={group}
                track={track}
                encoder_id={1}
                {...primary}
            />
            <Value
                value="Sample START"
                position={topRight}
                group={group}
                track={track}
                encoder_id={2}
                {...quaternary}
                USE_STRING_VALUE
            />
            <Value
                value="Sample LOOP_LENGTH"
                position={bottomRight}
                group={group}
                track={track}
                encoder_id={3}
                {...quaternary}
            />
        </VisibilityContainer>
    );
}
