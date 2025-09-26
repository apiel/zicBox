import * as React from '@/libs/react';

import { HiddenValue } from '@/libs/nativeComponents/HiddenValue';
import { KnobAction } from '@/libs/nativeComponents/KnobAction';
import { Text } from '@/libs/nativeComponents/Text';
import { A1, A2, A3, A4, B1, B2, B3, B4, C1, C2, C3, C4 } from '../constants';
import { enc4 } from '../constantsValue';
import { Layout } from './Layout';

export type Props = {
    name: string;
    track: number;
    synthName: string;
    color: string;
    title: string;
};

export function Keyboard({ name, track, synthName, color, title }: Props) {
    return (
        <Layout
            viewName={name}
            color={color}
            synthName={synthName}
            title={title}
            skipMainKeys
            content={
                <>
                    <Text fontSize={32} text="Exit" bounds={enc4.bounds} font="PoppinsLight_16" />
                    <KnobAction action={`setView:${synthName}`} encoderId={enc4.encoderId} />

                    <HiddenValue keys={[
                        { key: A1, action: `noteOn:${synthName}:67` },
                        { key: A2, action: `noteOn:${synthName}:69` },
                        { key: A3, action: `noteOn:${synthName}:71` },
                        { key: A4, action: `noteOn:${synthName}:72` },

                        { key: B1, action: `noteOn:${synthName}:60` },
                        { key: B2, action: `noteOn:${synthName}:62` },
                        { key: B3, action: `noteOn:${synthName}:64` },
                        { key: B4, action: `noteOn:${synthName}:65` },

                        { key: C1, action: `noteOn:${synthName}:53` },
                        { key: C2, action: `noteOn:${synthName}:55` },
                        { key: C3, action: `noteOn:${synthName}:56` },
                        { key: C4, action: `noteOn:${synthName}:58` },
                    ]} />
                </>
            }
        />
    );
}
