import * as React from '@/libs/react';

import { Rect } from '@/libs/components/Rect';
import { ColorTrack3, ColorTrack4, ColorTrack5 } from '../constants';
import { TextGridSel } from './TextGridSel';
import { Title } from './Title';

export function TextGridDrums({
    selected,
    viewName,
    target,
}: {
    selected: number;
    viewName: string;
    target: string;
}) {
    let color = ColorTrack3;
    let rectX = 62;
    if (target === 'HiHat') {
        color = ColorTrack4;
        rectX = 58;
    } else if (target === 'Sample') {
        color = ColorTrack5;
        rectX = 55;
    }
    return (
        <>
            <Title title={target} />
            <Rect position={[rectX, 28, 6, 6]} color={color} />
            <TextGridSel
                items={['&empty', 'Sample', '...', '&empty', 'Seq.', '&icon::musicNote::pixelated']}
                keys={[
                    // { key: 'q', action: viewName === 'Snare' ? 'setView:Snare2' : 'setView:Snare' },
                    { key: 'w', action: viewName === 'Sample' ? 'setView:Sample2' : 'setView:Sample' },
                    { key: 'e', action: 'contextToggle:254:1:0' },

                    // {
                    //     key: 'a',
                    //     action: viewName === 'HiHat' ? 'setView:HiHat2' : 'setView:HiHat',
                    // },
                    { key: 's', action: 'setView:DrumsSeq' },
                    { key: 'd', action: `noteOn:${target}:60` },
                ]}
                selected={selected}
                contextValue={0}
                ITEM_BACKGROUND={color}
            />
        </>
    );
}
