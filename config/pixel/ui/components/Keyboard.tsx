import * as React from '@/libs/react';

import { HiddenValue } from '@/libs/nativeComponents/HiddenValue';
import { A1, A2, A3, A4, B1, B2, B3, B4, C1, C2, C3, C4 } from '../constants';
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
                    <HiddenValue keys={[{ key: A1, action: `noteOn:${synthName}:67` }]} />
                    <HiddenValue keys={[{ key: A2, action: `noteOn:${synthName}:69` }]} />
                    <HiddenValue keys={[{ key: A3, action: `noteOn:${synthName}:71` }]} />
                    <HiddenValue keys={[{ key: A4, action: `noteOn:${synthName}:72` }]} />

                    <HiddenValue keys={[{ key: B1, action: `noteOn:${synthName}:60` }]} />
                    <HiddenValue keys={[{ key: B2, action: `noteOn:${synthName}:62` }]} />
                    <HiddenValue keys={[{ key: B3, action: `noteOn:${synthName}:64` }]} />
                    <HiddenValue keys={[{ key: B4, action: `noteOn:${synthName}:65` }]} />

                    <HiddenValue keys={[{ key: C1, action: `noteOn:${synthName}:55` }]} />
                    <HiddenValue keys={[{ key: C2, action: `noteOn:${synthName}:56` }]} />
                    <HiddenValue keys={[{ key: C3, action: `noteOn:${synthName}:58` }]} />

                    {/* // Should be shift but exit for now */}
                    <HiddenValue keys={[{ key: C4, action: `setView:${synthName}#track` }]} />
                </>
            }
        />
    );
}
