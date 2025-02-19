import { applyZic, jsonStringify, ZicValue } from './core';
import { ComponentProps } from './nativeComponents/component';

export type Bounds = string | string[] | number[];

// TODO to be deprecated
export function getPosition(pos: Bounds) {
    return Array.isArray(pos) ? pos.join(' ') : pos;
}

export function getBounds(pos: Bounds) {
    return Array.isArray(pos) ? pos : pos.split(' ');
}

export function getJsonComponent<P>(componentName: string) {
    return (props: ComponentProps<P>) => {
        const COMPONENT = jsonStringify({
            ...props,
            componentName,
            bounds: getBounds(props.bounds),
        });
        return [{ COMPONENT }];
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
    applyZic([{ WINDOW_POSITION: `${x} ${y}` }]);
}

/**
 * Set the screen size
 * @param width number The width of the screen
 * @param height number The height of the screen
 */
export function setScreenSize(width: number, height: number) {
    applyZic([{ SCREEN: `${width} ${height}` }]);
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
    return `#${r.toString(16).padStart(2, '0')}${g.toString(16).padStart(2, '0')}${b.toString(16).padStart(2, '0')}`;
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
    return `#${r.toString(16).padStart(2, '0')}${g.toString(16).padStart(2, '0')}${b.toString(16).padStart(2, '0')}${(Math.round(a * 255)).toString(16).padStart(2, '0')}`;
}