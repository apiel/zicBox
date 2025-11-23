/** Description:
This code defines a special visual element called a "Sequential Progress Bar" (`SeqProgressBar`).

Think of it as registering a reusable blueprint for a sophisticated progress tracker, often used when timing is critical, such as during audio playback or multi-stage processes.

**How the Code Works (Basic Idea):**
The code uses a function designed to build user interface elements from pre-defined structural configurations (like a JSON blueprint). It essentially tells the system: "Create a component named `SeqProgressBar` and here are all the specific ways someone can customize it."

**Functionality and Customization:**

This Progress Bar is highly customizable, focusing heavily on audio synchronization and visual feedback:

1.  **Audio Integration:** It requires a primary link or reference to the sound element it is tracking (`audioPlugin`). It also allows for optional configuration of related volume controls.
2.  **Appearance:** Users can control almost every color aspect of the bar, including:
    *   The overall background and foreground colors.
    *   Specific colors for the currently active section, the sections already completed, and the upcoming (inactive) steps.
    *   The color used when a user makes a selection on the bar.
3.  **Step Display:** It includes an option to visually display the individual steps or markers along the timeline, which helps users understand where they are in the sequence.

sha: 51de455d96c80bfbfee06fe25b029b66b74350365741e9f515e67be8e559cf06 
*/
import { getJsonComponent } from '../ui';

export const SeqProgressBar = getJsonComponent<{
    audioPlugin: string;
    volumePlugin?: { plugin: string; param: string };
    bgColor?: string;
    fgColor?: string;
    activeColor?: string;
    inactiveStepColor?: string;
    selectionColor?: string;
    showSteps?: boolean;
}>('SeqProgressBar');
