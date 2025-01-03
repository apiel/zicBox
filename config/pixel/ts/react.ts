

export function createElement(component: Function, props?: Object, ...children: any[]) {
    // console.log(typeof component, typeof props, typeof children);
    const element = component(props);
    if (!element) {
        return;
    }
    if (typeof element.component === 'function') {
        return createElement(element.component, element.props, ...children);
    }
    console.log(element.component, element.props);
    if (element.props) {
        for (const prop in element.props) {
            console.log('-' + prop, element.props[prop]);
        }
    }

    console.log(`Children (${children.length}):`);
    for (let i = 0; i < children.length; i++) {
        console.log(children[i]);
    }

    return { ...element, children };
}

export function Fragment() {
    return {
        component: 'Fragment',
    };
}