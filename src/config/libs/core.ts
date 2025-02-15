declare var IS_RPI: boolean;
declare var zic: (key: string, value: string) => void;

export function _zic(key: string, value: string) {
    zic(key, value);
}


  /**
   * Convert a value to a JSON string. Since Lua doesn't have builtin JSON support, 
   * this function is a custom function to serialize values to JSON strings.
   *
   * @param value The value to convert
   * @returns A JSON string representing the value
   * @throws {Error} If the value is of an unsupported type
   */
export function jsonStringify(value: any): string {
    // Handle primitive types
    if (typeof value === "string") {
      return `"${value}"`; // Wrap strings in double quotes
    } else if (typeof value === "number" || typeof value === "boolean") {
      return String(value); // Convert numbers and booleans to strings
    } else if (value === null) {
      return "null"; // Handle null
    } else if (Array.isArray(value)) {
      // Handle arrays
      const elements = value.map((item) => jsonStringify(item)).join(",");
      return `[${elements}]`;
    } else if (typeof value === "object") {
      // Handle objects
      const properties = Object.keys(value)
        .map((key) => `"${key}":${jsonStringify(value[key])}`)
        .join(",");
      return `{${properties}}`;
    } else {
      throw new Error(`Unsupported type: ${typeof value}`);
    }
  }

/**
 * Get the build the plateform
 * @returns string The plateform
 */
export function buildPlateform() {
    return IS_RPI ? 'arm' : 'x86';
}

export type ZicObj = Record<string, string | number | boolean | undefined>;
export type ZicValue = ZicObj | ZicValue[];

/**
 * Apply zic values by iterating over them
 * @param values table Zic values
 */
export function applyZic(values: ZicValue[]) {
    for (const obj of (values as any[]).flat(Infinity)) {
        for (const key in obj) {
            zic(key.toUpperCase(), `${obj[key]}`);
            // console.log(`${key}: ${obj[key]}`);
        }
    }
}
// applyZic([{ VIEW: "hello" }, [{HELLO: 'world'}]]);

export function dump(obj: any, log = true) {
    let buffer = '';
    const color = (str: string, c = 35) => `\x1b[${c}m` + str + '\x1b[0m';
    if (typeof obj === 'string' || typeof obj === 'number' || typeof obj === 'boolean') {
        buffer += `${obj}`;
    } else if (Array.isArray(obj)) {
        buffer += color('[');
        for (const item of obj) {
            buffer += dump(item, false);
            buffer += color(',');
        }
        buffer += color(']');
    } else if (typeof obj === 'object') {
        buffer += color('{');
        for (const key in obj) {
            buffer += color(`${key}: `, 34);
            buffer += dump(obj[key], false);
            buffer += color(',');
        }
        buffer += color('}');
    }
    if (log) {
        console.log(buffer);
    }
    return buffer;
}
// dump('test');
// dump(123);
// dump(true);
// dump([1, 2, 3]);
// dump({ a: 1, b: 'test' });
// dump([]);
// dump({});
// dump({ hello: [1, 2, 3], world: { a: 'test' } });
