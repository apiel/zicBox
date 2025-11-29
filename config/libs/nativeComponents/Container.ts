
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
