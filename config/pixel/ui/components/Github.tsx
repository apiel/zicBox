import * as React from '@/libs/react';

import { VisibilityContext } from '@/libs/nativeComponents/component';
import { GitHub } from '@/libs/nativeComponents/GitHub';
import { Text } from '@/libs/nativeComponents/Text';
import { rgb } from '@/libs/ui';
import { C1, C3, C4, githubContext, menuTextColor, ScreenHeight, ScreenWidth, W1_4, W2_4, W3_4 } from '../constants';
import { enc1 } from '../constantsValue';
import { Layout } from './Layout';

export type Props = {
    name: string;
};

const notAuthenticated: VisibilityContext = { condition: 'SHOW_WHEN_NOT', index: githubContext, value: 2 }

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
                    <Text
                        text="Go to https://github.com/login/device"
                        bounds={[20, 30, ScreenWidth - 40, 10]}
                        centered
                        visibilityContext={[notAuthenticated]}
                    />
                    <Text
                        text="And enter following code:"
                        bounds={[20, 50, ScreenWidth - 40, 10]}
                        centered
                        visibilityContext={[notAuthenticated]}
                    />
                    <GitHub
                        bounds={[20, 70, ScreenWidth - 40, 30]}
                        keys={[{ key: C3, action: `.refresh` }, { key: C1, action: `.next`, context: { id: githubContext, value: 0 } }]}
                        encoderId={enc1.encoderId}
                        contextId={githubContext}
                        default="default_pixel"
                    />
                    <Text
                        text="Once you enter the code, press next."
                        bounds={[20, 120, ScreenWidth - 40, 10]}
                        centered
                        visibilityContext={[notAuthenticated]}
                    />

                    <Text
                        text="Next"
                        bounds={[0, ScreenHeight - 20, W1_4, 18]}
                        centered={true}
                        color={menuTextColor}
                        keys={[{ key: C4, action: `setView:&previous` }]}
                        visibilityContext={[notAuthenticated]}
                    />

                    <Text
                        text="Refresh"
                        bounds={[W2_4, ScreenHeight - 20, W1_4, 18]}
                        centered={true}
                        color={menuTextColor}
                        visibilityContext={[notAuthenticated]}
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
