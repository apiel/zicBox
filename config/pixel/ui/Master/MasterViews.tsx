import * as React from '@/libs/react';

import { Main2View } from './Main2View';
import { MainView } from './MainView';

export function MasterViews() {
    return (
        <>
            <MainView name="Master" />
            <Main2View name="Master:page2" />
        </>
    );
}
