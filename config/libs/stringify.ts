/** Description:
These tools are specialized utilities designed to convert complex data structures (like configuration files or logging information) into a human-readable text format, similar to structured log output or JSON. They solve common issues where standard conversion methods produce either a single, unreadable line or excessive, deeply nested indentation.

The core mechanism of both functions is "recursion," meaning the tool systematically inspects every piece of data inside the structure, calling itself to format sub-sections until the entire structure has been processed.

### 1. Stringify with Limited Indentation

This function focuses on controlling the visual appearance (indentation). It ensures that the output is neatly spaced, but only up to a specified level of depth (by default, 4 levels deep). If data is nested beyond that limit, the tool stops indenting, presenting the deeper content compactly on fewer lines. This prevents logs and reports from becoming unnecessarily wide and cluttered when dealing with very complex, multi-layered data.

### 2. Stringify with Compacted Keys

This function provides "smart" formatting based on the *name* of the data section. If the tool encounters a data block associated with a specific key (such as 'plugins' or 'components'), it immediately compacts the entire content of that block onto a single line. This is useful for summarizing large, verbose sections of data that are not critical for immediate review, making the overall structure much cleaner and easier to navigate. It also includes a backup mechanism to compact any data found deeper than 5 levels.

sha: 43aefc90ab6e3b1fa5407f644155cd7a6a82bbe1580daf3b16092df2d82eae55 
*/
export function stringifyWithLimitedIndentation(
    obj: unknown,
    levels: number = 4,
    space: number = 2
): string {
    // Helper function to recursively format the object
    const format = (obj: unknown, level: number): string => {
        if (level > levels) {
            // Compact formatting for levels deeper than `levels`
            return JSON.stringify(obj);
        }

        if (Array.isArray(obj)) {
            // Format arrays
            const items = obj.map((item) => format(item, level + 1));
            return `[\n${' '.repeat(level * space)}${items.join(
                ',\n' + ' '.repeat(level * space)
            )}\n${' '.repeat((level - 1) * space)}]`;
        } else if (typeof obj === 'object' && obj !== null) {
            // Format objects
            const keys = Object.keys(obj);
            const items = keys.map(
                (key) =>
                    `${' '.repeat(level * space)}"${key}": ${format(
                        (obj as Record<string, unknown>)[key],
                        level + 1
                    )}`
            );
            return `{\n${items.join(',\n')}\n${' '.repeat((level - 1) * space)}}`;
        } else {
            // Format primitives (strings, numbers, booleans, null)
            return JSON.stringify(obj);
        }
    };

    return format(obj, 1);
}

// export function stringifyWithCompactedKeys(
//     obj: unknown,
//     compactKeys: string[] = ['plugins', 'components', 'zonesEncoders'],
//     space: number = 2
// ): string {
//     // Helper function to recursively format the object
//     const format = (obj: unknown, level: number, parentKey?: string): string => {
//         if (Array.isArray(obj)) {
//             // Format arrays
//             const items = obj.map((item) => format(item, level + 1, parentKey));
//             return `[\n${' '.repeat(level * space)}${items.join(
//                 ',\n' + ' '.repeat(level * space)
//             )}\n${' '.repeat((level - 1) * space)}]`;
//         } else if ((parentKey && compactKeys.includes(parentKey)) || level > 5) {
//             return JSON.stringify(obj);
//         } else if (typeof obj === 'object' && obj !== null) {
//             // Format objects
//             const keys = Object.keys(obj);
//             const items = keys.map(
//                 (k) =>
//                     `${' '.repeat(level * space)}"${k}": ${format(
//                         (obj as Record<string, unknown>)[k],
//                         level + 1,
//                         k // Pass the key to check if it should be compacted
//                     )}`
//             );
//             return `{\n${items.join(',\n')}\n${' '.repeat((level - 1) * space)}}`;
//         } else {
//             // Format primitives (strings, numbers, booleans, null)
//             return JSON.stringify(obj);
//         }
//     };

//     return format(obj, 1);
// }

export function stringifyWithCompactedKeys(
    obj: unknown,
    compactKeys: string[] = ['plugins', 'components', 'zonesEncoders'],
    space: number = 2
): string {
    // Helper function to recursively format the object
    const format = (obj: unknown, level: number, compact: number = 0): string => {
        if (compact >= 2 || level > 5) {
            return JSON.stringify(obj);
        }
        if (Array.isArray(obj)) {
            // Format arrays
            const items = obj.map((item) => format(item, level + 1, compact > 0 ? compact + 1 : 0));
            return `[\n${' '.repeat(level * space)}${items.join(
                ',\n' + ' '.repeat(level * space)
            )}\n${' '.repeat((level - 1) * space)}]`;
        } else if (typeof obj === 'object' && obj !== null) {
            // Format objects
            const keys = Object.keys(obj);
            const items = keys.map(
                (k) =>
                    `${' '.repeat(level * space)}"${k}": ${format(
                        (obj as Record<string, unknown>)[k],
                        level + 1,
                        compactKeys.includes(k) ? 1 : 0,
                    )}`
            );
            return `{\n${items.join(',\n')}\n${' '.repeat((level - 1) * space)}}`;
        } else {
            // Format primitives (strings, numbers, booleans, null)
            return JSON.stringify(obj);
        }
    };

    return format(obj, 1);
}

