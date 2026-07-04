/** Description:
This TypeScript code provides the essential building blocks for creating structured user interfaces, acting much like an assembly line for application components. It is fundamental to how modern application frameworks organize and render their visual elements.

The primary utility is the `createElement` function. Imagine you are building a structured document. This function is responsible for assembling each part (component). It takes three inputs: the blueprint (which type of component to build, like a "Header" or "Button"), the specific features (like "color" or "text size"), and any smaller items that go inside that structure (the nested content).

When this function runs, it calls the component's underlying instructions, generating the final structural piece. Crucially, the function ensures that the resulting component and all its nested content are organized into a single, clean list. It automatically removes any parts that were conditionally included but ended up being empty (like a feature that wasn't needed), guaranteeing a clean and optimized structure ready for rendering.

The second utility is `Fragment`. In user interface design, sometimes you need to group several elements together logically, but you don't want that group to create an extra, invisible box or container in the final display. The `Fragment` serves as a lightweight, invisible wrapper that helps organize the structure without adding unnecessary visual overhead.

sha: 13a749cf13492fd6aecdeac876d9dff982c06758d7e935579c13df9f85df1ec0 
*/
import { View } from "./nativeComponents/View";

export function createElement(component: Function, props?: Object, ...children: any[]) {
    // const element = component(props);
    const element = component.apply({ children }, [props]);
    // console.log({ component, props, children, element });
    // console.log({ component, props, children: JSON.stringify(children), element: JSON.stringify(element) });
    // filter child for && condition component
    // return [element, children.filter((child) => child)].flat(Infinity);
    return [element, component === View ? [] : children.filter((child) => child)].flat(Infinity);
    // return element;
    // return [element];
}

export function Fragment() {
    return [];
    // return [{component: 'Fragment'}];
}
