import { pixelController } from '@/libs/controllers/pixelController';
import { applyZic } from '@/libs/core';
import * as React from '@/libs/react';
import { addZoneEncoder, setScreenSize, setWindowPosition } from '@/libs/ui';
import { ClipsView } from './clipsView';
import { ScreenHeight, ScreenWidth, W1_2, W1_4, W3_4 } from './constants';
import { MenuView } from './menuView';
import { CreateWorkspaceView } from './menuView/CreateWorkspaceView';
import { ShutdownView } from './menuView/ShutdownView';
import { WorkspacesView } from './menuView/WorkspacesView';
import { SeqView } from './seqView';
import { TapeView } from './tapeView';
import { SaveTapeView } from './tapeView/SaveTapeView';

pixelController('rpi0_4enc_6btn');

setWindowPosition(400, 500);
setScreenSize(ScreenWidth, ScreenHeight);

addZoneEncoder([0, 0, W1_4, ScreenHeight]);
addZoneEncoder([W1_4, 0, W1_4, ScreenHeight]);
addZoneEncoder([W1_2, 0, W1_4, ScreenHeight]);
addZoneEncoder([W3_4, 0, W1_4, ScreenHeight]);

applyZic(<SeqView name="Home" />);
applyZic(<ClipsView name="Clips" />);
applyZic(<MenuView name="Menu" />);
applyZic(<WorkspacesView name="Workspaces" />);
applyZic(<CreateWorkspaceView name="CreateWorkspace" />);
applyZic(<ShutdownView name="Shutdown" />);

// for (let i = 1; i <= 8; i++) {
//     applyZic(<TapeView name={`Tape_track${i}`} />);
// }
applyZic(<TapeView name={`Tape`} />);
applyZic(<SaveTapeView name={`SaveTape`} />);


// applyZic([{ print: "End aplying UI config" }]);