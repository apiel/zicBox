

export function createElement(component: Function, props?: Object, ...children: any[]) {
    const element = component(props);
    return [element, children].flat(Infinity);
}

export function Fragment() {
    return [{component: 'Fragment'}];
}