import * as React from '@/libs/react';

import { VisibilityContext } from '@/libs/nativeComponents/component';
import { GhRepo } from '@/libs/nativeComponents/GhRepo';
import { GitHub } from '@/libs/nativeComponents/GitHub';
import { Rect } from '@/libs/nativeComponents/Rect';
import { Text } from '@/libs/nativeComponents/Text';
import { rgb } from '@/libs/ui';
import {
    C1,
    C3,
    C4,
    githubContext,
    githubContext2,
    menuTextColor,
    ScreenHeight,
    ScreenWidth,
    W1_4,
    W2_4,
    W3_4,
} from '../constants';
import { enc1 } from '../constantsValue';
import { Layout } from './Layout';

export type Props = {
    name: string;
};

const enterCodeContext: VisibilityContext = {
    condition: 'SHOW_WHEN',
    index: githubContext,
    value: 0,
};
const authenticatedContext: VisibilityContext = {
    condition: 'SHOW_WHEN',
    index: githubContext,
    value: 2,
};

export function Github({ name }: Props) {
    return (
        <Layout
            viewName={name}
            color={rgb(170, 170, 170)}
            title="GitHub"
            skipMainKeys
            noPrevious
            content={
                <>
                    <GhRepo bounds={[110, 2, ScreenWidth - 110, 20]} color={rgb(204, 203, 203)} />

                    <Rect
                        bounds={[20, 30, ScreenWidth - 40, 10]}
                        color="background"
                        visibilityContext={[authenticatedContext]}
                    />
                    <Text
                        text="Go to https://github.com/login/device"
                        bounds={[20, 30, ScreenWidth - 40, 10]}
                        centered
                        visibilityContext={[enterCodeContext]}
                    />

                    <Rect
                        bounds={[20, 50, ScreenWidth - 40, 10]}
                        color="background"
                        visibilityContext={[authenticatedContext]}
                    />
                    <Text
                        text="And enter following code:"
                        bounds={[20, 50, ScreenWidth - 40, 10]}
                        centered
                        visibilityContext={[enterCodeContext]}
                    />

                    <GitHub
                        bounds={[20, 70, ScreenWidth - 40, 30]}
                        keys={[
                            { key: C1, action: `.next`, context: { id: githubContext, value: 0 } },
                            {
                                key: C1,
                                action: `contextToggle:${githubContext2}:1:0`,
                                context: { id: githubContext, value: 2 },
                            },

                            {
                                key: C3,
                                action: `.refresh`,
                                context: { id: githubContext, value: 0 },
                            },
                            { key: C3, action: `.save`, context: { id: githubContext, value: 2 } },
                        ]}
                        encoderId={enc1.encoderId}
                        contextId={githubContext}
                        default="default_pixel"
                    />

                    <Rect
                        bounds={[20, 120, ScreenWidth - 40, 20]}
                        color="background"
                        visibilityContext={[authenticatedContext]}
                    />
                    <Text
                        text="Once you enter the code, press next."
                        bounds={[20, 120, ScreenWidth - 40, 16]}
                        centered
                        visibilityContext={[enterCodeContext]}
                    />
                    <Rect
                        bounds={[0, 120, ScreenWidth, 20]}
                        color="background"
                        visibilityContext={[{ index: githubContext2, value: 0, condition: 'SHOW_WHEN' },]}
                    />
                    <Text
                        text="Loading repo, will replace existing preset. Save first..."
                        bounds={[0, 120, ScreenWidth, 16]}
                        centered
                        visibilityContext={[{ index: githubContext2, value: 1, condition: 'SHOW_WHEN' },]}
                    />

                    <Text
                        text="Load"
                        bounds={[0, ScreenHeight - 20, W1_4, 18]}
                        centered={true}
                        color={menuTextColor}
                        visibilityContext={[authenticatedContext]}
                    />
                    <Text
                        text="Next"
                        bounds={[0, ScreenHeight - 20, W1_4, 18]}
                        centered={true}
                        color={menuTextColor}
                        keys={[{ key: C4, action: `setView:&previous` }]}
                        visibilityContext={[enterCodeContext]}
                    />

                    <Rect
                        bounds={[W1_4, ScreenHeight - 20, W1_4, 18]}
                        color="background"
                        visibilityContext={[
                            { index: githubContext2, value: 0, condition: 'SHOW_WHEN' },
                        ]}
                    />
                    <Text
                        text="Continue"
                        bounds={[W1_4, ScreenHeight - 20, W1_4, 18]}
                        centered={true}
                        bgColor="#4cb663ff"
                        visibilityContext={[
                            { index: githubContext2, value: 1, condition: 'SHOW_WHEN' },
                        ]}
                    />

                    <Text
                        text="Save"
                        bounds={[W2_4, ScreenHeight - 20, W1_4, 18]}
                        centered={true}
                        color={menuTextColor}
                        visibilityContext={[authenticatedContext]}
                    />
                    <Text
                        text="Refresh"
                        bounds={[W2_4, ScreenHeight - 20, W1_4, 18]}
                        centered={true}
                        color={menuTextColor}
                        visibilityContext={[enterCodeContext]}
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
