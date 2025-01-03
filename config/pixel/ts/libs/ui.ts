import { applyZic, ZicValue } from './core';

export type Position = string | string[];

export function getPosition(pos: Position) {
    return Array.isArray(pos) ? pos.join(' ') : pos;
}

/**
 * Create a component
 *
 * @param name string - The name of the component to create.
 * @param position string | string[] - The position of the component.
 * @param values table - An array of Zic values to apply to the component.
 * @returns table
 */
export function getComponent(name: string, position: Position, values: ZicValue[] = []) {
    return [{ COMPONENT: `${name} ${getPosition(position)}` }, ...values];
}

/**
 * Create a component
 *
 * @param name string - The name of the component to create.
 * @param position string | string[] - The position of the component.
 * @param values table - An array of Zic values to apply to the component.
 */
export function component(name: string, position: Position, values: ZicValue[] = []) {
    applyZic(getComponent(name, position, values));
}

/**
 * Add a zone encoder
 * @param position string | string[] - The position of the component.
 */
export function addZoneEncoder(position: Position) {
    applyZic([{ ADD_ZONE_ENCODER: getPosition(position) }]);
}

/**
 * Create a view
 *
 * @param name string - The name of the view to create.
 * @param values table - An array of Zic values to apply to the view.
 * @returns table
 */
export function getView(name: string, values: ZicValue[] = []) {
    return [{ VIEW: name }, ...values];
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
