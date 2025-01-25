import * as React from '@/libs/react';

import { StepEditSample } from '@/libs/components/StepEditSample';
import { View } from '@/libs/components/View';
import { Common } from '../components/Common';
import { TextGridSel } from '../components/TextGridSel';
import { SampleTrack, ScreenWidth } from '../constants';

export type Props = {
    name: string;
};

export function SampleEditorView({ name }: Props) {
    return (
        <View name={name}>
            <StepEditSample
                position={[0, 0, ScreenWidth, 8]}
                data={`SampleSequencer`}
                // group={i}
            />

            <TextGridSel
                items={['Editor', 'Sample', '...', '&empty', 'Seq.', '&icon::musicNote::pixelated']}
                keys={[
                    // { key: 'q', action: 'setView:SampleEditor' },
                    // {
                    //     key: 'w',
                    //     action: viewName === 'Sample' ? 'setView:Sample2' : 'setView:Sample',
                    // },
                    // { key: 'e', action: 'contextToggle:254:1:0' },

                    // // {
                    // //     key: 'a',
                    // //     action: viewName === 'HiHat' ? 'setView:HiHat2' : 'setView:HiHat',
                    // // },
                    // { key: 's', action: 'setView:DrumsSeq' },
                    // { key: 'd', action: `noteOn:${target}:60` },
                ]}
                selected={0}
                contextValue={0}
            />
            <Common selected={1} track={SampleTrack} hideSequencer />
        </View>
    );
}
