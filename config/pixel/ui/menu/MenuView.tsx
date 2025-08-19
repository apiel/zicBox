import * as React from '@/libs/react';

import { Text } from '@/libs/nativeComponents/Text';
import { rgb } from '@/libs/ui';
import { Layout } from '../components/Layout';
import { ScreenHeight, W1_4, W2_4, W3_4 } from '../constants';

export type Props = {
    name: string;
};

export function MenuView({ name }: Props) {
    const menuTextColor = rgb(170, 170, 170);
    return (
        <Layout
            viewName={name}
            color={menuTextColor}
            title="Menu"
            content={
                <>
                    <Text
                        text="Load"
                        bounds={[0, ScreenHeight - 80, W1_4, 16]}
                        centered={true}
                        color={menuTextColor}
                    />
                    <Text
                        text="Exit"
                        bounds={[W1_4, ScreenHeight - 80, W1_4, 16]}
                        centered={true}
                        color={menuTextColor}
                    />
                    <Text
                        text="Shutdown"
                        bounds={[W2_4, ScreenHeight - 80, W1_4, 16]}
                        centered={true}
                        color={menuTextColor}
                    />
                    <Text
                        text="Shift"
                        bounds={[W3_4, ScreenHeight - 80, W1_4, 16]}
                        centered={true}
                        color={menuTextColor}
                    />

                    <Text
                        text="Hello"
                        bounds={[0, ScreenHeight - 50, W1_4, 16]}
                        centered={true}
                        color={menuTextColor}
                    />
                    <Text
                        text="Exit"
                        bounds={[W1_4, ScreenHeight - 50, W1_4, 16]}
                        centered={true}
                        color={menuTextColor}
                    />
                    <Text
                        text="Shutdown"
                        bounds={[W2_4, ScreenHeight - 50, W1_4, 16]}
                        centered={true}
                        color={menuTextColor}
                    />
                    <Text
                        text="Shift"
                        bounds={[W3_4, ScreenHeight - 50, W1_4, 16]}
                        centered={true}
                        color={menuTextColor}
                    />

                    <Text
                        text="Load"
                        bounds={[0, ScreenHeight - 20, W1_4, 16]}
                        centered={true}
                        color={menuTextColor}
                    />
                    <Text
                        text="Exit"
                        bounds={[W1_4, ScreenHeight - 20, W1_4, 16]}
                        centered={true}
                        color={menuTextColor}
                    />
                    <Text
                        text="Shutdown"
                        bounds={[W2_4, ScreenHeight - 20, W1_4, 16]}
                        centered={true}
                        color={menuTextColor}
                    />
                    <Text
                        text="Shift"
                        bounds={[W3_4, ScreenHeight - 20, W1_4, 16]}
                        centered={true}
                        color={menuTextColor}
                    />
                </>
            }
        />
    );
}
