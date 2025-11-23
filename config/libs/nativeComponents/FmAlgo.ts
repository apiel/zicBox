/** Description:
This code snippet serves as a foundational blueprint for defining a reusable element within a software application, specifically one likely related to audio processing or interface design.

### What This Code Does

This code defines a constant object named `FmAlgo`. Think of `FmAlgo` as a specialized "building block" or widget that can be placed multiple times throughout an application.

The code uses a specialized function (`getJsonComponent`) which acts like a factory. This factory’s job is to load the actual structure and behavior of the `FmAlgo` component from a standardized configuration source (likely a file that stores definitions in a structured data format).

### The Component Blueprint

Crucially, the code specifies a strict set of rules, or a "contract," defining exactly what properties this component must and can accept when it is used:

1.  **Styling Options (Optional):** Developers can optionally customize the appearance using settings like `bgColor` (background color), `textColor`, and `borderColor`.
2.  **Identifiers (Optional):** It can accept unique tracking numbers or codes, such as an `encoderId` or a `dataId`.
3.  **Core Functionality (Required):** It requires critical settings necessary for its operation, specifically `audioPlugin` and `param`. These are essential instructions that must be provided for the component to function correctly, likely dictating how audio data is processed.

In summary, this definition ensures that every time the application uses the `FmAlgo` building block, it adheres to the same configuration standards, guaranteeing consistency and proper functionality, especially concerning its audio processing capabilities.

sha: 16e432b3497c8cad485b78f508eced38078fe3d32b4810d96f2e327ea951710e 
*/
import { getJsonComponent } from '../ui';

export const FmAlgo = getJsonComponent<{
    bgColor?: string;
    textColor?: string;
    borderColor?: string;
    encoderId?: number;
    audioPlugin: string;
    param: string;
    dataId?: string;
}>('FmAlgo');