

export function createElement(component: Function, props?: Object, ...children: any[]) {
    const element = component(props);
    // filter child for && condition component
    return [element, children.filter(child => child)].flat(Infinity);
}

export function Fragment() {
    return [];
    // return [{component: 'Fragment'}];
}