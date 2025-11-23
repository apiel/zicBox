/** Description:
This code acts as a standardized factory or blueprint for creating structured visual components or "Views" within a larger application framework.

### 1. The Blueprint (Input Requirements)

The system first defines exactly what information is needed to build this specific view. This blueprint requires:

*   **A Name:** A mandatory identifier or title for the component being built.
*   **A "No Previous" Flag (Optional):** An optional signal indicating that the surrounding application should ignore any previous state or history when displaying this component.

### 2. The Builder Function (Core Logic)

The central function takes the provided inputs (name, optional flag) and combines them with any elements that might be nested *inside* this View.

The core process involves:

1.  **Gathering Sub-Elements:** It checks if the View contains any nested components (called "children").
2.  **Cleaning and Organizing:** It tidies up the list of nested components, ensuring that any empty or temporary containers are removed. Crucially, it flattens everything: if components are deeply layered inside one another, it brings them all up to the top level into one simple, clean list.
3.  **Conditional Setting:** It intelligently includes the optional "No Previous" setting in the final structure only if it was explicitly provided in the input.

### 3. The Output (Standardized Configuration)

The function returns a single, neatly packaged configuration object. This package contains the View's name, the flattened list of usable sub-elements, and any special flags (like "No Previous"), making it ready for the application framework to render and display.

sha: 6b45d37288e016e7020dba492f5386f1bfeaab18aa76888342d8e8db28069a26 
*/
import { ZicObj } from '@/libs/core';

export interface Props extends ZicObj {
    name: string;
    noPrevious?: boolean;
}

export function View(this: any, { name, noPrevious }: Props) {
    const children: [] = this.children;
    return [
        {
            name,
            ...noPrevious && { noPrevious: true },
            components: (children || []).filter((child) => child).flat(Infinity),
        },
    ];
}
