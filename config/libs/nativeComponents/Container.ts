/** Description:
This TypeScript code defines a standardized organizational structure called a "Container," which serves as a crucial building block in a larger technical system, similar to how folders organize files on a computer.

### The Blueprint

Before a Container can be built, the code defines a precise blueprint specifying the required information:
1.  **Name:** A simple label used to identify the Container.
2.  **Bounds:** Four numbers that precisely define the Container's size and position within the larger system (like coordinates on a screen).
3.  **Resize Type (Optional):** A control setting that dictates how the Container should behave when the system tries to stretch or shrink it.

### How the Container Works

The main function acts as a specialized manager. Its core purpose is to take the provided setup information and organize all the other elements (referred to as "children") that are placed inside it.

When the Container is created, it performs essential cleanup and organizational tasks:

1.  **Collection:** It gathers every element currently nested within it.
2.  **Cleaning:** It automatically ignores or discards any empty or incomplete elements in the list.
3.  **Flattening:** This is the most important step. If you have elements nested many layers deep (like items inside Group A, which is inside Group B), the code pulls everything out to create one simple, single-level sequence of components.

The final output is a clearly structured package that identifies itself as a 'Container,' retains its original position and name, and includes the consistently organized list of all the components it manages. This ensures that the entire system maintains a tidy and easy-to-process structure.

sha: 2292a540c67ff0cf0bc75dbc9bef01bc774a2c1b9e0945e0e7f7ba7f452dab14 
*/

/** Description:
This TypeScript code defines a standardized building block called a "Container" for use within a larger technical system, likely a graphics engine, scene graph, or component framework.

### The Blueprint

First, the code establishes a blueprint that dictates the exact information required to build one of these elements. This includes a descriptive text label (`name`) and a specific location defined by a pair of numbers (like coordinates on a grid or map, known as `position`).

### How the Container Works

The main function acts as a specialized constructor. Its job is to take the provided name and position, and most importantly, organize any elements that are nested inside of it (referred to as "children").

When executed, the function performs three key steps:

1.  It collects all the elements that have been placed inside this Container.
2.  It cleans up this collected list by removing any empty spots or undefined entries.
3.  It flattens the list. If you had elements nested multiple layers deep (like a box inside a box), the code ensures everything is pulled out and placed into one simple, single-level sequence.

Finally, the function outputs a clearly structured object. This object formally identifies itself as a 'Container', retains its unique name and position, and includes the clean, flattened list of components it is responsible for managing. This process ensures that all elements in the system maintain a consistent, easy-to-read structure.

sha: 5acceebedc84cfa436a8e6b356aef4f7e87767dac3399a51b357a31e71fbc1d8 
*/
export interface Props {
    name: string;
    bounds: [Number, Number, Number, Number];
    resizeType?: number;
}

export function Container(this: any, { name, bounds, resizeType }: Props) {
    const children: [] = this.children;
    return [
        {
            __type: 'Container',
            name,
            bounds,
            resizeType,
            components: (children || []).filter((child) => child).flat(Infinity),
        },
    ];
}
