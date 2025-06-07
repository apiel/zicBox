import * as React from '@/libs/react';

import { TempoView } from '../Tempo/TempoView';
import { CreateWorkspaceView } from './CreateWorkspaceView';
import { FileServerView } from './FileServerView';
import { ShutdownView } from './ShutdownView';
import { WorkspacesView } from './WorkspacesView';

export function Menu() {
    return (
        <>
            <CreateWorkspaceView name="CreateWorkspace" />
            <WorkspacesView name="Workspaces" />
            <ShutdownView name="Shutdown" />
            <TempoView name="Tempo" />
            <FileServerView name="FileServer" />
        </>
    );
}
