import * as React from '@/libs/react';

import { CreateWorkspaceView } from './CreateWorkspaceView';
import { ShutdownView } from './ShutdownView';
import { WorkspacesView } from './WorkspacesView';

export function Menu() {
    return (
        <>
            <CreateWorkspaceView name="CreateWorkspace" />
            <WorkspacesView name="Workspaces" />
            <ShutdownView name="Shutdown" />
        </>
    );
}
