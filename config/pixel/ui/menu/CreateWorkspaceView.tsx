import * as React from '@/libs/react';

import { workspaceFolder } from '@/grid/audio';
import { View } from '@/libs/nativeComponents/View';
import { Workspaces } from '@/libs/nativeComponents/Workspaces';
import { MainKeys } from '../components/Common';
import { InputForm } from '../components/CreateWorkspaceView';
import { ScreenHeight, ScreenWidth } from '../constants';

export type Props = {
    name: string;
};

export function CreateWorkspaceView({ name }: Props) {
    return (
        <View name={name}>
            <Workspaces
                workspaceFolder={workspaceFolder}
                audioPlugin="SerializeTrack"
                bounds={[0, 0, ScreenWidth, ScreenHeight - 310]}
            />
            <InputForm
                bounds={[0, ScreenHeight - 300, ScreenWidth, 300]}
                redirectView="Workspaces"
                audioPlugin="SerializeTrack"
                dataId="CREATE_WORKSPACE"
            />

            <MainKeys viewName={name} />
        </View>
    );
}
