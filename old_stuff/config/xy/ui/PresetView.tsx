import * as React from '@/libs/react';

import { Preset } from '@/libs/nativeComponents/Preset';
import { Text } from '@/libs/nativeComponents/Text';
import { Layout } from './components/Layout';
import { TextArray } from './components/TextArray';
import { A1, A2, A3, A4, B1, B2, B7, B8, bgColor, deleteContext, ScreenHeight, ScreenWidth } from './constants';

export type Props = {
    name: string;
    track: number;
    synthName: string;
    color: string;
    title: string;
};

export function PresetView({ name, track, synthName, color, title }: Props) {
    return (
        <Layout
            viewName={name}
            color={color}
            synthName={synthName}
            title={title}
            noPrevious
            content={
                <>
                    <Text text="Preset:" bounds={[10, 40, 100, 20]} font="PoppinsLight_12" color="#b6b6b6" />
                    <Preset
                        bounds={[10, 60, ScreenWidth - 20, 120]}
                        bgColor={bgColor}
                        audioPlugin={synthName}
                        track={track}
                        visibleItems={5}
                        folder="data/presets/SynthMultiEngine"
                        keys={[
                            { key: A1, action: `.loadTrig` }, // if already loaded, pressing it will trigger the sound
                            { key: A2, action: `.restore`, action2: `setView:${synthName}PresetSave` },
                            { key: A3, action: `.restore`, action2: `setView:&previous` },
                            { key: A4, action: `.exit`, action2: `setView:&previous` },

                            { key: B1, action: `.up` },
                            { key: B2, action: `.down` },

                            { key: B7, action: `.delete`, context: { id: deleteContext, value: 1 } },
                            { key: B8, action: `contextToggle:${deleteContext}:1:0` },
                        ]}
                    />

                    <TextArray texts={['Load', 'Save', 'Cancel', 'Done', '---']} top={ScreenHeight - 40} />
                    <TextArray
                        texts={['&icon::arrowUp::filled', '&icon::arrowDown::filled', '---', '---', '---', '---', '---', '&icon::trash']}
                        top={ScreenHeight - 20}
                        visibilityContext={[{ condition: 'SHOW_WHEN', index: deleteContext, value: 0 }]}
                    />
                    <TextArray
                        texts={['---', '---', '---', '---', '---', '---', ['Yes', '#ffacac', '#ffffff'], '&icon::trash']}
                        top={ScreenHeight - 20}
                        visibilityContext={[{ condition: 'SHOW_WHEN', index: deleteContext, value: 1 }]}
                    />
                </>
            }
        />
    );
}
