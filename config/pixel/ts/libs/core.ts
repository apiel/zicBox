/** @noSelfInFile **/

declare var IS_RPI: boolean;
declare var zic: (key: string, value: string) => void;

/**
 * Get the build the plateform
 * @returns string The plateform
 */
export function buildPlateform() {
    return IS_RPI ? 'arm' : 'x86';
}

export interface ZicValue {
    [key: string]: string | number | boolean;
}

/**
 * Apply zic values by iterating over them
 * @param values table Zic values
 */
export function applyZic(values: ZicValue[]) {
    for (const obj of values) {
        for (const key in obj) {
            const value = typeof obj[key] === 'boolean' ? (obj[key] ? 'true' : 'false') : obj[key];
            // zic(key, `${value}`);
            console.log(`${key}: ${value}`);
        }
    }
}


