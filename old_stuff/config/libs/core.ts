/** Description:
This code acts as a utility layer focused on preparing data for transmission, detecting the operating environment, and providing custom debugging tools. It is designed for use in specialized environments, particularly those interacting with systems that may lack standard capabilities, such as environments relying on Lua scripting.

The core functionality revolves around data translation. A key component, `jsonStringify`, is a custom function that converts complex data structures (like lists and grouped settings) into a standardized, simple text format (JSON string). This is crucial because the environment it runs in might not have a built-in tool for this task.

For system interaction, the code uses a mechanism called `zic`. Functions like `_zic` and `applyZic` are used to process configuration settings (represented as key-value pairs) and dispatch them through this specific interface to the underlying application or system, allowing it to apply changes or configurations.

Furthermore, the code includes environment intelligence via `buildPlateform`, which determines whether the system is running on a high-efficiency chip (like 'arm' architecture, often found in devices like Raspberry Pi) or a standard desktop chip ('x86').

Lastly, a specialized debugging tool called `dump` is provided. This function helps developers by taking any complex piece of data and displaying it in the console in a human-readable format, often using color coding to distinguish different elements.

sha: 6b48d18d5d09c381c39572d7e8cc789a4ec5ea32252cc5943db8fd82a0de9d58 
*/
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
  // console.log(values);
  // console.log(JSON.stringify(values, null, 2));
    for (const obj of (values as any[]).flat(Infinity)) {
        for (const key in obj) {
            // zic(key.toUpperCase(), `${obj[key]}`);
            // zic(key.toUpperCase(), jsonStringify(obj[key]));
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
