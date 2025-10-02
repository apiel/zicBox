import * as React from '@/libs/react';

import { GitHub } from '@/libs/nativeComponents/GitHub';
import { Text } from '@/libs/nativeComponents/Text';
import { rgb } from '@/libs/ui';
import { C3, menuTextColor, ScreenHeight, ScreenWidth, W1_4, W2_4 } from '../constants';
import { Layout } from './Layout';

export type Props = {
    name: string;
};

export function Github({ name }: Props) {
    return (
        <Layout
            viewName={name}
            color={rgb(170, 170, 170)}
            title="GitHub"
            skipMainKeys
            content={
                <>
                    <GitHub bounds={[20, 70, ScreenWidth - 40, 30]} />
                    <Text
                        text="Exit"
                        bounds={[W2_4, ScreenHeight - 20, W1_4, 18]}
                        centered={true}
                        color={menuTextColor}
                        keys={[{ key: C3, action: `setView:&previous` }]}
                    />
                </>
            }
        />
    );
}
