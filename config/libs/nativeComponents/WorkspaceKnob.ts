/** Description:
This section of code defines a crucial reusable component named `WorkspaceKnob`. Think of this component as a virtual dial or control found in complex software, particularly those related to audio editing or sound mixing.

The purpose of this code is not to build the component visually, but to establish a *definition* for it. It uses a specialized function to retrieve the entire design and visual layout of the knob from an external configuration file—acting like fetching a detailed blueprint rather than drawing the component manually.

The `WorkspaceKnob` is standardized, meaning it is guaranteed to perform a specific function: controlling an `audioPlugin` setting. However, it is also highly customizable. The definition specifies that users can optionally change almost every aspect of its appearance, including background color, label color, font style, and font size for all text elements.

In summary, this file creates a standardized, flexible control that links the software’s visual design (defined externally) with its required functionality (controlling specific audio settings).

sha: 762b42cee39542599f4cdf483595e73cf9eee251c82e41353b65e1bf99013e5b 
*/
import { getJsonComponent } from '../ui';

export const WorkspaceKnob = getJsonComponent<{
    bgColor?: string;
    color?: string;
    labelColor?: string;
    badgeColor?: string;
    fontSize?: number;
    font?: string;
    labelFontSize?: number;
    labelFont?: string;
    audioPlugin: string;
}>('WorkspaceKnob');
