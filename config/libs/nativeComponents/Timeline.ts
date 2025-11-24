/** Description:
This code defines and configures a central part of the application's user interface, specifically a feature referred to as the "Timeline."

The process works by utilizing a dedicated internal tool (`getJsonComponent`) to register this component structure within the system. Essentially, the code tells the application, "Here is the standard way to build a Timeline."

The extensive list of configuration options attached to the Timeline serves as a mandatory blueprint. This blueprint dictates exactly what settings must or can be provided whenever someone uses this component, ensuring consistency and preventing errors.

These configuration settings fall into several categories:

1.  **File Management and Location:** Settings like `workspaceFolder` and `clipFolder` specify where the Timeline should look for related files and content.
2.  **Visual Customization:** A wide array of color properties (e.g., `gridColor`, `barColor`, `textColor`) are defined, allowing precise control over the visual appearance of the component.
3.  **Behavior and Logic:** Advanced properties link the Timeline to the application's core functionality, specifying which software plugins (`sequencerPlugin`) should control sequencing behavior or which internal IDs (`trackContextId`) manage tracking and stepping logic.

In short, this code establishes the technical foundation, required settings, and visual standards for the Timeline component used throughout the software.

sha: fabc5b739f639e1a22aaab66826f058867f25a2eb634c946047befc2b222b3a4 
*/
import { getJsonComponent } from '../ui';

export const Timeline = getJsonComponent<{
    audioPlugin: string;
    timelineDataId?: string;
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
