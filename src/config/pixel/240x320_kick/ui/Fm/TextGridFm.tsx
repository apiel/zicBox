import * as React from '@/libs/react';

import { Rect } from '@/libs/components/Rect';
import { TextGridSel } from '../components/TextGridSel';
import { Title } from '../components/Title';
import { ColorTrack2 } from '../constants';

export function TextGridFm({ selected, viewName }: { selected: number; viewName: string }) {
    return (
        <>
            <Title title="Fm" />
            <Rect position={[85, 28, 6, 6]} color={ColorTrack2} />
            <TextGridSel
                ITEM_BACKGROUND={ColorTrack2}
                items={[ 'Fm', '&empty', '...', '&empty', 'Seq.', '&icon::musicNote::pixelated']}
                keys={[
                    { key: 'q', action: viewName === 'Fm' ? 'setView:Fm2' : 'setView:Fm' },
                    { key: 'e', action: 'contextToggle:254:1:0' },
                    { key: 'd', action: 'noteOn:FmDrum:60' },
                ]}
                selected={selected}
                contextValue={0}
            />
        </>
    );
}
