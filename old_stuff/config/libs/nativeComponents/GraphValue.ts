/** Description:
This code block defines a crucial user interface component called "GraphValue."

### Purpose and Mechanism

The primary purpose of the GraphValue component is likely to display a dynamic reading, such as an audio signal level, or to allow users to graphically manipulate a specific setting within an application, particularly in systems dealing with audio processing or parameter control.

Instead of writing all the component logic from scratch, the code utilizes a specialized helper function to quickly load a pre-configured, standardized definition (often stored as a configuration file). This approach ensures that all components named "GraphValue" across the application maintain consistent structure and behavior.

### Component Configuration (Properties)

This definition specifies all the configurable options—the "properties"—that developers can use when placing a GraphValue element in the user interface:

**1. Visual Styling:**
These properties control the look of the component:
*   `fillColor`, `outlineColor`, and `bgColor`: Determine the colors of the displayed value, the border, and the component's background.
*   `outline` and `filled`: Simple on/off switches to control if the component has a visible border and whether its internal graph area is fully colored.

**2. Functional Linkage (Required Settings):**
These are the most important settings, linking the visual component to the application's core functionality:
*   `audioPlugin`: Specifies which specific audio module or external device the component is intended to control.
*   `param`: Designates the exact parameter within that audio plugin that the GraphValue is visualizing or adjusting.

**3. Behavioral Controls:**
*   `inverted`: A flag that might reverse the visual display or the parameter mapping.
*   `extendEncoderIdArea`: A numerical setting that fine-tunes the size or behavior of any associated physical input control (like a rotary knob) used to interact with the graph.

sha: c2de163aca994963ce5c9f303acce3604a291d3f9592d2f68f46d5ede6cd4b42 
*/
import { getJsonComponent } from '../ui';

export const GraphValue = getJsonComponent<{
    outline?: boolean;
    fillColor?: string;
    filled?: boolean;
    outlineColor?: string;
    bgColor?: string;
    audioPlugin: string;
    param: string;
    inverted?: boolean;
    extendEncoderIdArea?: number;
}>('GraphValue');
