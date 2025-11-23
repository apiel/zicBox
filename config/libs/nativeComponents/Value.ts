/** Description:
This code defines the blueprint for a highly configurable user interface element named `Value`. This element is specifically designed to display a single, crucial piece of data, often related to an audio system or real-time process.

### How the Code Works

The `Value` element is created using a utility function (`getJsonComponent`). This function acts as a factory, taking a name ('Value') and a massive list of configuration options. This approach standardizes how display widgets are built across the application, ensuring consistency and ease of maintenance.

### The Component's Purpose

The resulting `Value` component serves as a display widget—like a digital meter or a numeric readout. It requires specific connection details, notably the `audioPlugin` and the target `param`, to know which real-time data it should be monitoring and displaying (for example, displaying the current volume level or an equalization setting).

### Customization and Appearance

The extensive list of properties within the blueprint allows developers to control almost every visual and behavioral aspect of the display without writing complex front-end code:

1.  **Data Linkage:** It defines required parameters for connecting to the system data (`audioPlugin`, `param`).
2.  **Visual Layout:** Settings control the dimensions, alignment, and sizing of text and graphical bars (e.g., `barHeight`, `valueSize`, `verticalAlignCenter`).
3.  **Aesthetics:** Developers can set specific colors for every part of the widget—the background (`bgColor`), the descriptive label (`labelColor`), the value being displayed (`valueColor`), and any graphical meter bars (`barColor`).
4.  **Display Options:** Properties allow hiding certain aspects (like the unit of measurement or the label) or specifying the numerical precision (`floatPrecision`) of the displayed data.

In summary, this code creates a flexible, reusable widget used for metering and presenting specific system parameters in a controlled, customized manner.

sha: fe2f6ceaf1b84208eb4e69dabe1937e313c41e2c2d94259fdd1e48fd04a626ea 
*/
import { getJsonComponent } from '../ui';

export const Value = getJsonComponent<{
    audioPlugin: string;
    param: string;
    encoderId?: number;
    label?: string;
    floatPrecision?: number;
    useNumberValue?: boolean;
    barHeight?: number;
    barBgHeight?: number;
    verticalAlignCenter?: boolean;
    hideLabel?: boolean;
    hideUnit?: boolean;
    hideValue?: boolean;
    alignLeft?: boolean;
    showLabelOverValue?: number;
    labelOverValueX?: number;
    valueSize?: number;
    labelSize?: number;
    unitSize?: number;
    fontValue?: string;
    fontLabel?: string;
    valueHeight?: number;
    bgColor?: string;
    labelColor?: string;
    valueColor?: string;
    barColor?: string;
    unitColor?: string;
}>('Value');
