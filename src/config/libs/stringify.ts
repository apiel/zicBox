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
