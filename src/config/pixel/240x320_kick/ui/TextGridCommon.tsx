{
    /* progressbar */
}
import * as React from '@/libs/react';

import { Keymaps } from '@/libs/components/Keymaps';
import { TextGrid } from '@/libs/components/TextGrid';
import { VisibilityContext } from '@/libs/components/VisibilityContext';
import { KeyInfoPosition } from '@/pixel/240x320/ui/constants';

export function TextGridCommon() {
    return (
        <TextGrid
            position={KeyInfoPosition}
            rows={[
                'Page1 Page2 ^...',
                'Seq. Menu &icon::play::filled',
            ]}
        >
            <VisibilityContext index={254} condition="SHOW_WHEN" value={1} />
            <Keymaps keys={[{ key: 'e', action: 'contextToggle:254:1:0' }]} />
        </TextGrid>
    );
}
