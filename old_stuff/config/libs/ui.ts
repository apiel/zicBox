/** Description:
This comprehensive TypeScript file acts as a foundational toolkit for building graphical user interfaces (GUIs), particularly in environments that rely on a configuration system referred to internally as "Zic."

The primary goal of this utility is to bridge the gap between high-level application definitions and the underlying command structure required to display and manage components.

### How the Code Works

This toolkit operates across three main areas:

1.  **Component Management and Build Setup:** The system tracks every custom UI element used in the application. It provides a function to register these elements and, crucially, generates a special configuration file (a "Makefile"). This automated generation ensures that all custom parts are properly identified and included when the application is compiled or deployed. It handles coordinates and dimensions (known as "bounds") by ensuring they are consistently formatted for the system.

2.  **View and Command Execution:** The toolkit allows developers to define different screens or containers, referred to as "Views." Functions are provided to create these views and apply specific configuration commands ("Zic values") instantly. This includes setting global parameters like the application's window position and screen resolution.

3.  **Color Utilities:** A set of helper functions simplifies working with colors. Developers can define colors using standard Red, Green, and Blue (RGB) values, or include transparency (RGBA). The utilities also allow programmatically adjusting the brightness of any color by making it lighter or darker.

In summary, this file provides the essential structure for defining layout, registering custom components, and manipulating colors, acting as the primary interface for configuring the look and function of the resulting application.

sha: f2d3f13390cced95fc6912cf4ce4113a93f718f9ccbdbbe46a1ecbbabdb8d6e1 
*/
import { writeFileSync } from 'fs';
import { applyZic, ZicValue } from './core';
import { ComponentProps } from './nativeComponents/component';

const componentPluginList = new Set();

export type Bounds = string | string[] | number[];

// TODO to be deprecated
export function getPosition(pos: Bounds) {
    return Array.isArray(pos) ? pos.join(' ') : pos;
}

export function getBounds(pos: Bounds) {
    return Array.isArray(pos) ? pos : pos.split(' ');
}

export function generateComponentMakefile(dest: string) {
    const content = `customBuild:
	make ${[...componentPluginList].map(p => `${p}Component`).join(' ')}
`;

    writeFileSync(dest, content);
}

export function getJsonComponent<P>(componentName: string) {
    return function (this: any, props: ComponentProps<P>) {
        componentPluginList.add(componentName);
        const component = {
            ...props,
            componentName,
            bounds: getBounds(props.bounds),
        };
        // const children: [] = this.children;
        // const flatChildren = children.filter((child) => child).flat(Infinity);
        // console.log({ flatChildren });
        return [component];
    };
}

/**
 * Add a zone encoder
 * @param position string | string[] - The position of the component.
 */
export function addZoneEncoder(bounds: Bounds) {
    applyZic([{ ADD_ZONE_ENCODER: getPosition(bounds) }]);
}

/**
 * Create a view
 *
 * @param name string - The name of the view to create.
 * @param values table - An array of Zic values to apply to the view.
 * @returns table
 */
export function getView(name: string, values: ZicValue[] = []) {
    return [{ VIEW: name }, values];
}

/**
 * Create a view and apply Zic values to it.
 *
 * @param name string - The name of the view to create.
 * @param values table - An array of Zic values to apply to the view.
 */
export function view(name: string, values: ZicValue[] = []) {
    applyZic(getView(name, values));
}

/**
 * Set the window position
 *
 * @param x number The x position
 * @param y number The y position
 */
export function setWindowPosition(x: number, y: number) {
    // applyZic([{ WINDOW_POSITION: `${x} ${y}` }]);
    return { windowPosition: { x, y } };
}

/**
 * Set the screen size
 * @param width number The width of the screen
 * @param height number The height of the screen
 */
export function setScreenSize(width: number, height: number) {
    // applyZic([{ SCREEN: `${width} ${height}` }]);
    return { screenSize: { width, height } };
}

/**
 * Returns a string representing a color in hexadecimal notation.
 *
 * @param r number The red component of the color, between 0 and 255.
 * @param g number The green component of the color, between 0 and 255.
 * @param b number The blue component of the color, between 0 and 255.
 *
 * @returns string A string in the format `#RRGGBB`, where `RR`, `GG`, and `BB` are the red, green, and blue components of the color, respectively, in hexadecimal notation.
 */
export function rgb(r: number, g: number, b: number) {
    return `#${r.toString(16).padStart(2, '0')}${g.toString(16).padStart(2, '0')}${b
        .toString(16)
        .padStart(2, '0')}`;
}

export function darken(color: string, amount: number) {
    // Ensure the amount is between 0 and 1
    amount = Math.min(1, Math.max(0, amount));

    // Extract the red, green, and blue components from the hex color
    const r = parseInt(color.slice(1, 3), 16);
    const g = parseInt(color.slice(3, 5), 16);
    const b = parseInt(color.slice(5, 7), 16);

    // Calculate the darker color by reducing each component
    const darkenComponent = (component: number) => Math.round(component * (1 - amount));

    const newR = darkenComponent(r);
    const newG = darkenComponent(g);
    const newB = darkenComponent(b);

    // Convert back to hex and return the color
    return rgb(newR, newG, newB);
}

export function lighten(color: string, amount: number) {
    // Ensure the amount is between 0 and 1
    amount = Math.min(1, Math.max(0, amount));

    // Extract the red, green, and blue components from the hex color
    const r = parseInt(color.slice(1, 3), 16);
    const g = parseInt(color.slice(3, 5), 16);
    const b = parseInt(color.slice(5, 7), 16);

    // Calculate the lighter color by increasing each component
    const lightenComponent = (component: number) =>
        Math.round(component + (255 - component) * (1 - amount));

    const newR = lightenComponent(r);
    const newG = lightenComponent(g);
    const newB = lightenComponent(b);

    // Convert back to hex and return the color
    return rgb(newR, newG, newB);
}

/**
 * Returns a string representing a color with an alpha channel in hexadecimal notation.
 *
 * @param r number The red component of the color, between 0 and 255.
 * @param g number The green component of the color, between 0 and 255.
 * @param b number The blue component of the color, between 0 and 255.
 * @param a number The alpha component of the color, between 0 and 1.
 *
 * @returns string A string in the format `#RRGGBBAA`, where `RR`, `GG`, `BB`, and `AA` are the red, green, blue, and alpha components of the color, respectively, in hexadecimal notation.
 */
export function rgba(r: number, g: number, b: number, a: number) {
    return `#${r.toString(16).padStart(2, '0')}${g.toString(16).padStart(2, '0')}${b
        .toString(16)
        .padStart(2, '0')}${Math.round(a * 255)
        .toString(16)
        .padStart(2, '0')}`;
}
