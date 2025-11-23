/** Description:
This code defines and configures a reusable software module named the "Sequencer Card."

### Overview

In essence, this file acts as a setup instruction for a highly specialized user interface element, typically found in music production or audio control applications. A sequencer is visualized as a grid used to arrange notes or events over time. This code loads the blueprint for that specific interface.

### Core Functionality

The code utilizes a standardized function (`getJsonComponent`) designed to fetch and instantiate pre-defined components from the application's resources. It creates the `SequencerCard` object, making it immediately available for use elsewhere in the program.

### Component Configuration

The structure dictates a comprehensive list of properties that govern the card's behavior, appearance, and interaction:

1.  **Audio Linkage:** It specifies which specific audio engine or "plugin" the sequencer controls.
2.  **Visual Settings:** It controls the look of the grid, including background color, the colors for active and inactive steps, and the general structure (like the maximum number of steps or steps per row).
3.  **Data Management:** It includes identifiers that link the visual interface to the actual pattern data—telling the component where to save or load the musical sequence.
4.  **Hardware/Input Mapping:** It defines properties related to external input devices (like encoders or specific grid keys) that users might use to interact with the sequencing grid.

By defining all these functional and aesthetic settings upfront, the `SequencerCard` is created as a ready-to-use building block for the final application interface.

sha: 829aea8d6aa58b3dec4c792c887237d35d417b103b2f626b00cd8bdafce39a34 
*/
import { getJsonComponent } from '../ui';

export const SequencerCard = getJsonComponent<{
    audioPlugin: string;
    bgColor?: string;
    stepsDataId?: string;
    stepPerRow?: number;
    maxSteps?: number;
    activeStepColor?: string;
    stepLengthColor?: string;
    stepBackground?: string;
    inactiveStepColor?: string;
    stepSelectedColor?: string;
    rowsSelectionColor?: string;
    rowsSelection?: number;
    contextId?: number;
    gridKeys?: (number | string)[];
    encoderId?: number;
}>('SequencerCard');
