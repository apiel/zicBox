import * as React from '@/libs/react';

import { View } from '@/libs/nativeComponents/View';
import { MainKeys } from '../components/Common';
import { InputForm } from '../components/CreateWorkspaceView';
import {
    ScreenWidth
} from '../constants';

export type Props = {
    name: string;
};

export function CreateWorkspaceView({ name }: Props) {
    return (
        <View name={name}>
            <InputForm
                bounds={[0, 40, ScreenWidth, 300]}
                redirectView="Workspaces"
                audioPlugin="SerializeTrack"
                dataId="CREATE_WORKSPACE"
            />

            <MainKeys viewName={name} />
        </View>
    );
}
