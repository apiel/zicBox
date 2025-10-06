import * as React from '@/libs/react';

import { Text } from '@/libs/nativeComponents/Text';
import { rgb } from '@/libs/ui';
import { C4, menuTextColor, ScreenHeight, ScreenWidth, W1_4, W3_4 } from '../constants';
import { Layout } from './Layout';

export type Props = {
    name: string;
};

export function GithubHelp({ name }: Props) {
    return (
        <Layout
            viewName={name}
            color={rgb(170, 170, 170)}
            title="GitHub help"
            skipMainKeys
            noPrevious
            content={
                <>
                    <Text
                        text="To load a remote repo, there are 2 options:"
                        bounds={[10, 30, ScreenWidth - 20, 10]}
                    />
                    <Text
                        text="1. Fork apiel/zicData.git."
                        bounds={[10, 50, ScreenWidth - 20, 10]}
                    />
                    <Text
                        text="2.Create your own repo and copy the content"
                        bounds={[10, 70, ScreenWidth - 20, 10]}
                    />
                    <Text
                        text="   of /opt/zicBox/data into it."
                        bounds={[10, 87, ScreenWidth - 20, 10]}
                    />
                    <Text
                        text="   Use this second option to back up your local changes online."
                        bounds={[10, 105, ScreenWidth - 20, 10]}
                        font="PoppinsLight_8"
                        color={rgb(190, 190, 190)}
                    />

                    <Text
                        text="Warning"
                        bounds={[10, 120, ScreenWidth - 20, 10]}
                        color={rgb(255, 0, 0)}
                    />

                    <Text
                        text=": Loading a repo will replace all local data,"
                        bounds={[62, 120, ScreenWidth - 20, 10]}
                    />
                    <Text
                        text="so make sure to back it up first."
                        bounds={[10, 135, ScreenWidth - 20, 10]}
                    />

                    <Text
                        text="Back"
                        bounds={[W3_4, ScreenHeight - 20, W1_4, 18]}
                        centered={true}
                        color={menuTextColor}
                        keys={[{ key: C4, action: `setView:Github` }]}
                    />
                </>
            }
        />
    );
}
