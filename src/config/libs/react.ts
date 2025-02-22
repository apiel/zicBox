import { View } from "./nativeComponents/View";

export function createElement(component: Function, props?: Object, ...children: any[]) {
    // const element = component(props);
    const element = component.apply({ children }, [props]);
    // console.log({ component, props, children, element });
    console.log({ component, props, children: JSON.stringify(children), element: JSON.stringify(element) });
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
