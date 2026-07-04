/** Description:
This code defines a specialized component used within a user interface environment, specifically referred to as the **GraphEncoder**.

### Identity and Purpose

The primary function of this segment is to create and export the configuration blueprint for a graphical element, likely a widget or input control used for encoding or visualizing data (like a dial, slider, or simple graph).

It utilizes a factory function, `getJsonComponent`, which registers this element and defines all the allowable settings, ensuring the component is correctly recognized and rendered by the larger application framework.

### Component Configuration

The GraphEncoder component is highly customizable. The definition specifies a large set of properties, which dictate both its visual style and how it interacts with data:

1.  **Visual Styling:** These properties control the component's appearance, including whether it has an outline, the background and fill colors (`bgColor`, `fillColor`), and the colors used for displaying any text labels (`textColor1`, `textColor2`).
2.  **Data and Functionality:** These settings define the element’s behavior, such as identifying the specific data source (`dataId`), whether the data is presented as a list (`isArrayData`), and specifying an associated audio processing function (`audioPlugin`). Other options allow for visual adjustments, such as inverting the display or forcing values to render on top.

### The Role of TypeScript

The structure around the GraphEncoder ensures consistency. By listing all allowed settings, TypeScript acts as a strict validator. It guarantees that anyone using the GraphEncoder element can only provide settings that are correctly defined in this blueprint (for example, preventing developers from mistakenly trying to set a property that doesn't exist on the graph).

sha: 3c011912b8408ea7c09f910a3b017f80993b33b89a21a8a7d315798cff427aed 
*/
import { getJsonComponent } from '../ui';

export const GraphEncoder = getJsonComponent<{
    outline?: boolean;
    fillColor?: string;
    filled?: boolean;
    outlineColor?: string;
    textColor1?: string;
    textColor2?: string;
    bgColor?: string;
    audioPlugin: string;
    dataId: string;
    values?: string[];
    isArrayData?: boolean;
    renderValuesOnTop?: boolean;
    inverted?: boolean;
}>('GraphEncoder');
