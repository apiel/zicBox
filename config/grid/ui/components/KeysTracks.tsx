import * as React from '@/libs/react';

import { HiddenValue } from '@/libs/nativeComponents/HiddenValue';
import { A1, A2, B1, B2 } from '../constants';

export function KeysTracks({ synthName, viewName }: { synthName: string; viewName: string }) {
    return (
        <HiddenValue
            keys={[
                { key: A1, action: viewName === `Drum1` ? `setView:Drum1:page2` : `setView:Drum1` },
                { key: B1, action: `noteOn:Drum1:60` },

                { key: A2, action: `setView:Drum2` },
                { key: B2, action: `noteOn:Drum2:60` },
            ]}
        />
    );
}
