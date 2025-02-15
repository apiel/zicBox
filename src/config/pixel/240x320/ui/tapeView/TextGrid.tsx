{
    /* progressbar */
}
import * as React from '@/libs/react';

import { Keymaps } from '@/libs/components/Keymaps';
import { TextGrid } from '@/libs/components/TextGrid';
import { KeyInfoPosition } from '@/pixel/240x320/ui/constants';

export function TextGridTape() {
    return (
        <TextGrid
            bounds={KeyInfoPosition}
            rows={[
                '&icon::play::filled Save &empty',
                '&icon::arrowLeft::filled Home &icon::arrowRight::filled',
            ]}
        >
            <Keymaps
                keys={[
                    { key: 'w', action: 'setView:SaveTape' },
                    { key: 's', action: 'setView:Home' },
                ]}
            />
        </TextGrid>
    );
}
