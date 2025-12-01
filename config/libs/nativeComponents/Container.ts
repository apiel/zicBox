import { VisibilityContext } from './component';

/** Description:
This TypeScript code defines a fundamental building block, typically used in graphical rendering systems or specialized user interface frameworks, called a "Container."

### 1. The Blueprint (The `Props` Definition)

The first part of the code establishes the required blueprint for creating this Container. Every Container must have the following mandatory properties:

*   **Name:** A unique label or identifier.
*   **Position:** A pair of mandatory coordinates (like X and Y) to determine where the Container is located in the environment.
*   **Height:** An optional setting defining the size of the Container.

### 2. The Factory Function (`Container`)

The main function is responsible for assembling this component. It takes the defined properties (name, position, height) and uses them to create a structured object.

**How it Works:**

1.  **Gathering Inputs:** It accepts the defined properties.
2.  **Collecting Contents:** Crucially, it gathers any elements placed *inside* the Container (referred to as `children`).
3.  **Standardizing Output:** It generates a standardized object tagged internally as `'Container'`. This object packages the properties (name, position, height) alongside the contents (`children`).
4.  **Cleaning Up Nested Items:** The function ensures that the collected contents are neatly organized, removing any empty or unusable elements and handling any deeply nested items by bringing them all up to a single, clean list.

### Summary

In essence, this code creates a robust, standardized wrapper used to define and organize interactive elements within a larger system. It ensures that every container adheres to strict rules regarding its definition (name and position) and efficiently manages everything placed inside it.

sha: ec248a119196ddb04b3389e9e2d90acd5c67a4b86fcaf6e64e94bb3b39cf387f 
*/
export interface Props {
    name: string;
    position: [Number, Number];
    height?: string;
    visibilityContext?: VisibilityContext[];
}

export function Container(this: any, { name, position, height, visibilityContext }: Props) {
    const children: [] = this.children;
    return [
        {
            __type: 'Container',
            name,
            position,
            ...(height && { height }),
            ...(visibilityContext && { visibilityContext }),
            components: (children || []).filter((child) => child).flat(Infinity),
        },
    ];
}
