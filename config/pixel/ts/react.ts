

export function createElement(component: Function, props?: Object, ...children: any[]) {
    // console.log(typeof component, typeof props, typeof children);
    const element = component(props);
    if (!element) {
        return;
    }
    if (typeof element.component === 'function') {
        return createElement(element.component, element.props, ...children);
    }

    return [...element, ...children.flat()];
}

export function Fragment() {
    return {
        component: 'Fragment',
    };
}