/** Description:
This code acts as a structural blueprint for a specific user interface element within the application, specifically a visual component named the "Timeline."

**What This Code Does**

In simple terms, this defines the official recipe for building a powerful timeline interface.

1.  **Component Creation:** It uses a utility helper function (`getJsonComponent`) that specializes in constructing complex UI elements based on configuration files (like defining a reusable building block). The resulting element is exported for use throughout the application.

2.  **Defining Configuration:** The lengthy list of names (like `barColor`, `workspaceFolder`, `sequencerPlugin`) specifies all the mandatory and optional configuration settings that the Timeline needs to function correctly. These settings fall into two main categories:
    *   **Functionality:** Which external tools or specialized programs (plugins) the timeline relies on (e.g., `timelinePlugin`, `enginePlugin`).
    *   **Aesthetics and Data:** Where the data files are located (e.g., `workspaceFolder`, `clipFolder`) and the colors used for various visual elements (e.g., `gridColor`, `clipColor`, `textColor`).

By establishing this structure, the code ensures that whenever a Timeline is placed in the application, it receives all the necessary information to look right, access its data correctly, and interact properly with other software modules.

sha: db84538a69a36ff0dbcd723d9c2998e6aee07c50e4281748e56ed5164b14350d 
*/
import { getJsonComponent } from '../ui';

export const Timeline = getJsonComponent<{
    timelinePlugin: string;
    timelineDataId?: string;
    loadClipDataId?: string;
    workspaceFolder: string;
    clipFolder: string;
    scrollEncoderId?: number;
    background?: string;
    gridColor?: string;
    barColor?: string;
    clipColor?: string;
    loopColor?: string;
    textColor?: string;
    selectedColor?: string;
    sequencerPlugin?: string;
    enginePlugin?: string;
    trackContextId?: number;
    stepContextId?: number;
    viewStepStartContextId?: number;
    defaultSelectedTrack?: number;
    trackMin?: number;
    trackMax?: number;
}>('Timeline');
