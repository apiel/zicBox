import * as React from '@/libs/react';

import { HiddenValue } from '@/libs/nativeComponents/HiddenValue';
import { A1, B1 } from '../constants';

export function KeysTracks({
    synthName,
    viewName,
}: {
    synthName: string;
    viewName: string;
}) {
    return (
        <HiddenValue keys={[
            { key: A1, action: viewName === `${synthName}` ? `setView:${synthName}:page2` : `setView:${synthName}` },
            { key: B1, action: `noteOn:${synthName}:60` },
        ]} />
    );
}
