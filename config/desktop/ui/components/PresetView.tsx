import * as React from '@/libs/react';

import { Preset } from '@/libs/nativeComponents/Preset';
import { StringVal } from '@/libs/nativeComponents/StringVal';
import { Text } from '@/libs/nativeComponents/Text';
import {
    C1,
    C2,
    C3,
    C4,
    menuTextColor,
    ScreenHeight,
    ScreenWidth,
    W1_4,
    W2_4,
    W3_4
} from '../constants';
import { Layout } from './Layout';

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
                    <StringVal
                        audioPlugin={synthName}
                        param="ENGINE"
                        bounds={[52, 3, 60, 20]}
                        fontLabel="PoppinsLight_12"
                        labelColor="#FFFFFF"
                        unit
                    />

                    <StringVal
                        audioPlugin={synthName}
                        param="ENGINE"
                        bounds={[90, 3, 60, 20]}
                        fontLabel="PoppinsLight_12"
                    />

                    <Text
                        text="Preset"
                        bounds={[260, 3, 100, 20]}
                        font="PoppinsLight_12"
                        color="#b6b6b6"
                    />

                    <Preset
                        bounds={[20, 60, ScreenWidth - 40, 30]}
                        audioPlugin={synthName}
                        track={track}
                        folder="data/presets/SynthMultiEngine"
                        keys={[
                            { key: C1, action: `.restore`, action2: `setView:${synthName}SavePreset` },
                            { key: C2, action: `.loadTrig` }, // if already loaded, pressing it will trigger the sound
                            { key: C3, action: `.restore`, action2: `setView:&previous` },
                            { key: C4, action: `.exit`, action2: `setView:&previous` },
                        ]}
                    />

                    <Text
                        text="Save"
                        bounds={[0, ScreenHeight - 20, W1_4, 18]}
                        centered={true}
                        color={menuTextColor}
                        // Would be great that when clicking on save, it forward the name of the currently selected preset
                    />
                    <Text
                        text="Load/Trig"
                        bounds={[W1_4, ScreenHeight - 20, W1_4, 18]}
                        centered={true}
                        color={menuTextColor}
                    />
                    <Text
                        text="Cancel"
                        bounds={[W2_4, ScreenHeight - 20, W1_4, 18]}
                        centered={true}
                        color={menuTextColor}
                    />
                    <Text
                        text="Done"
                        bounds={[W3_4, ScreenHeight - 20, W1_4, 18]}
                        centered={true}
                        color={menuTextColor}
                    />
                </>
            }
        />
    );
}
