import * as React from '@/libs/react';

import { CreateWorkspaceView } from '../menu/CreateWorkspaceView';
import { TempoView } from '../Tempo/TempoView';
import { FileServerView } from './FileServerView';
import { MenuView } from './menuView';
import { ShutdownView } from './ShutdownView';
import { WorkspacesView } from './WorkspacesView';

export function Menu() {
    return (
        <>
            <MenuView name="Menu" />
            <CreateWorkspaceView name="CreateWorkspace" />
            <WorkspacesView name="Workspaces" />
            <ShutdownView name="Shutdown" />
            <TempoView name="Tempo" />
            <FileServerView name="FileServer" />
        </>
    );
}
