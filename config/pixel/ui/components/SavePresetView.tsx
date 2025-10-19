import * as React from '@/libs/react';

import { SavePreset } from '@/libs/nativeComponents/SavePreset';
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

export function SavePresetView({ name, track, synthName, color, title }: Props) {
    return (
        <Layout
            viewName={name}
            color={color}
            synthName={synthName}
            title={title}
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
                        text="Save preset"
                        bounds={[230, 3, 100, 20]}
                        font="PoppinsLight_12"
                        color="#b6b6b6"
                    />

                    <SavePreset
                        bounds={[20, 60, ScreenWidth - 40, 30]}
                        audioPlugin={synthName}
                        track={track}
                        keys={[
                            { key: C1, action: `.save` },
                            { key: C2, action: `.backspace` },
                            { key: C3, action: `.jump` },
                        ]}
                    />

                    <Text
                        text="Save"
                        bounds={[0, ScreenHeight - 20, W1_4, 18]}
                        centered={true}
                        color={menuTextColor}
                    />
                    <Text
                        text="&icon::backspace::filled"
                        bounds={[W1_4, ScreenHeight - 18, W1_4, 18]}
                        centered={true}
                        color={menuTextColor}
                    />
                    <Text
                        text="ab AB 01 .-"
                        bounds={[W2_4, ScreenHeight - 20, W1_4, 18]}
                        centered={true}
                        color={menuTextColor}
                    />
                    <Text
                        text="Exit"
                        bounds={[W3_4, ScreenHeight - 20, W1_4, 18]}
                        centered={true}
                        color={menuTextColor}
                        keys={[{ key: C4, action: `setView:&previous` }]}
                    />
                </>
            }
        />
    );
}
