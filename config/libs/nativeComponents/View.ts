/** Description:
This TypeScript code serves as a structural definition and a processing unit for an application framework, likely responsible for managing how screens or panels (Views) are built and organized.

### 1. Defining the Structure (Props)

The first part of the code defines the "Props," which are the required settings or attributes for a specific View. Think of this as the configuration checklist for creating a screen:

*   **Name:** A required text label used to identify this specific View.
*   **No Previous:** An optional setting. If checked, it tells the system that the user cannot navigate back to a prior screen from this point.

### 2. The View Function (Processing Logic)

The main function (`View`) acts as a smart organizer. Its job is to take the defined settings and the elements nested inside it, and package them into a standardized format the rest of the application can understand.

#### How it works:

1.  **Collection:** It gathers all the elements that have been placed inside this View (referred to as "children") and organizes them into a single, clean list.
2.  **Categorization:** It inspects these collected elements to see if any are specialized structural components called "Containers." Containers are high-level groupings, similar to dedicated sections or layout managers.
3.  **Decision Making:** The function decides how to present the output based on what it finds:
    *   **If Containers Exist:** The View prioritizes the larger structure. It organizes the output around the specialized "Containers."
    *   **If No Containers Exist:** It assumes all nested elements are basic "Components" (individual buttons, fields, etc.) and groups the entire list under that generic category.

The final output is a clearly defined object containing the View's name, any navigational restrictions (`noPrevious`), and a structured hierarchy of its internal elements, ready to be rendered by the application.

sha: 8bfbc371f7c242720cc9a107dbd96a297fc579424b30711cc26d0df9d78f3ef0 
*/
import { ZicObj } from '@/libs/core';

export interface Props extends ZicObj {
    name: string;
    noPrevious?: boolean;
}

export function View(this: any, { name, noPrevious }: Props) {
    const children = (this.children || []).filter(Boolean).flat(Infinity);

    // If has container, only take containers and exclude components
    const containers = children.filter((child: any) => child?.__type === 'Container');

    return [
        {
            name,
            ...(noPrevious && { noPrevious: true }),
            ...(containers.length > 0 ? { containers } : { components: children }),
        },
    ];
}
