/** Description:
This file is responsible for setting up and defining the central musical timing widget within the application: the **Sequencer**.

Think of the Sequencer as a highly customizable programmable control panel—like a piano roll or drum machine—used for arranging sounds over time.

**How the Setup Works**

The code uses a specialized helper function to create the Sequencer. This function, often called a "factory," loads the widget's structural blueprint and default settings from an external source (such as a configuration file). This streamlined approach ensures that the complex component is built reliably according to a standard definition. The final, fully configured Sequencer is then made available for use throughout the rest of the application.

**The Sequencer's Customization Blueprint**

The most important part of this definition is the list of properties that customize the widget. This list acts as an instruction manual, defining every setting the Sequencer must accept:

1.  **Audio Configuration:** It requires specifying which sound engine or plugin (`audioPlugin`) the Sequencer will control.
2.  **Visual Styling:** It controls all aspects of the widget’s appearance, including background colors, specific colors for different elements (like black keys, white keys, and selected steps), and the visual marker used to indicate the current playing position.
3.  **Structural Limits:** It sets parameters for behavior, such as the maximum allowed length of a sequence (`maxStepLen`).

By centralizing the definition here, the application ensures that every instance of the Sequencer is correctly configured for both visual presentation and reliable audio control.

sha: 1032662e7325620ff3ab51faf4487a325dda52219ef21f811e9e0dc5ac27310d 
*/
import { getJsonComponent } from '../ui';

export const Sequencer = getJsonComponent<{
    audioPlugin: string;
    bgColor?: string;
    blackKeyColor?: string;
    whiteKeyColor?: string;
    selectedColor?: string;
    stepColor?: string;
    maxStepLen?: number;
    parameterSelection?: number;
    // TODO add the rest of params
}>('Sequencer');
