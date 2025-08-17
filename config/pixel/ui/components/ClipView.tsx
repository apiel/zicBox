import * as React from '@/libs/react';

import { Clips } from '@/libs/nativeComponents/Clips';
import { Text } from '@/libs/nativeComponents/Text';
import { rgb } from '@/libs/ui';
import { A1, A2, A3, A4, C2, ScreenHeight, ScreenWidth, shiftContext, W1_4, W2_4, W3_4 } from '../constants';
import { top } from '../constantsValue';
import { Layout } from './Layout';
import { shiftVisibilityContext, unshiftVisibilityContext } from './ShiftLayout';

export type Props = {
    name: string;
    track: number;
    synthName: string;
    color: string;
    title: string;
};

export function ClipView({ name, track, synthName, color, title }: Props) {
    return (
        <Layout
            viewName={name}
            color={color}
            synthName={synthName}
            title={title}
            content={
                <>
                    <Text
                        text="Select clip preset to LOAD."
                        bounds={[0, top, ScreenWidth, 18]}
                        centered={true}
                        color={rgb(100, 100, 100)}
                        visibilityContext={[unshiftVisibilityContext]}
                    />
                    <Text
                        text="Select clip preset to SAVE."
                        bounds={[0, top, ScreenWidth, 18]}
                        centered={true}
                        color={rgb(100, 100, 100)}
                        visibilityContext={[shiftVisibilityContext]}
                    />
                    <Clips
                        bounds={[20, top + 20, ScreenWidth - 40, 40]}
                        track={track}
                        color={color}
                        visibleCount={4}
                        keys={[
                            { key: A1, action: `.set:1`, context: { id: shiftContext, value: 0 } },
                            { key: A1, action: `.save:1`, context: { id: shiftContext, value: 1 } },

                            { key: A2, action: `.set:2`, context: { id: shiftContext, value: 0 } },
                            { key: A2, action: `.save:2`, context: { id: shiftContext, value: 1 } },

                            { key: A3, action: `.set:3`, context: { id: shiftContext, value: 0 } },
                            { key: A3, action: `.save:3`, context: { id: shiftContext, value: 1 } },

                            { key: A4, action: `.set:4`, context: { id: shiftContext, value: 0 } },
                            { key: A4, action: `.save:4`, context: { id: shiftContext, value: 1 } },

                            { key: C2, action: `.bank`, context: { id: shiftContext, value: 0 }, multipleKeyHandler: true },
                        ]}
                    />
                    <Clips
                        bounds={[20, top + 20 + 40 + 2, ScreenWidth - 40, 40]}
                        track={track}
                        addIndex={4}
                        color={color}
                        visibleCount={4}
                        keys={[
                            { key: C2, action: `.bank`, context: { id: shiftContext, value: 0 } },
                        ]}
                        // keys={[
                        //     { key: B1, action: `.set:1`, context: { id: shiftContext, value: 0 } },
                        //     { key: B1, action: `.save:1`, context: { id: shiftContext, value: 1 } },

                        //     { key: B2, action: `.set:2`, context: { id: shiftContext, value: 0 } },
                        //     { key: B2, action: `.save:2`, context: { id: shiftContext, value: 1 } },

                        //     { key: B3, action: `.set:3`, context: { id: shiftContext, value: 0 } },
                        //     { key: B3, action: `.save:3`, context: { id: shiftContext, value: 1 } },

                        //     { key: B4, action: `.set:4`, context: { id: shiftContext, value: 0 } },
                        //     { key: B4, action: `.save:4`, context: { id: shiftContext, value: 1 } },

                        //     { key: B5, action: `.set:5`, context: { id: shiftContext, value: 0 } },
                        //     { key: B5, action: `.save:5`, context: { id: shiftContext, value: 1 } },

                        //     { key: B6, action: `.set:6`, context: { id: shiftContext, value: 0 } },
                        //     { key: B6, action: `.save:6`, context: { id: shiftContext, value: 1 } },

                        //     { key: B7, action: `.set:7`, context: { id: shiftContext, value: 0 } },
                        //     { key: B7, action: `.save:7`, context: { id: shiftContext, value: 1 } },

                        //     { key: B8, action: `.set:8`, context: { id: shiftContext, value: 0 } },
                        //     { key: B8, action: `.save:8`, context: { id: shiftContext, value: 1 } },

                        //     { key: B9, action: `.set:9`, context: { id: shiftContext, value: 0 } },
                        //     { key: B9, action: `.save:9`, context: { id: shiftContext, value: 1 } },

                        //     { key: B10, action: `.set:10`, context: { id: shiftContext, value: 0 } },
                        //     { key: B10, action: `.save:10`, context: { id: shiftContext, value: 1 } },

                        //     { key: B11, action: `.bank`, context: { id: shiftContext, value: 0 } },
                        //     { key: B11, action: `.reload`, context: { id: shiftContext, value: 1 } },
                        // ]}
                    />

                    <Text
                        text="Bank"
                        bounds={[W1_4, ScreenHeight - 20, W1_4, 18]}
                        centered={true}
                        color={rgb(100, 100, 100)}
                    />

                    <Text
                        text="Exit"
                        bounds={[W2_4, ScreenHeight - 20, W1_4, 18]}
                        centered={true}
                        color={rgb(100, 100, 100)}
                    />

                    <Text
                        text="Save"
                        bounds={[W3_4, ScreenHeight - 20, W1_4, 18]}
                        centered={true}
                        color={rgb(100, 100, 100)}
                    />
                </>
            }
        />
    );
}
