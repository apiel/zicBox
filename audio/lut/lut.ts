import * as fs from 'fs';
import * as path from 'path';

function saveHeader(filename: string, arrayName: string, data: number[], SIZE = data.length) {
    const content =
        `// Generated LUT - Do not edit manually\n` +
        `#pragma once\n\n` +
        `const int ${arrayName}_SIZE = ${SIZE};\n` +
        `const float ${arrayName}[${arrayName}_SIZE] = {\n    ` +
        data
            .map((v, i) => `${v.toFixed(7)}f${(i + 1) % 8 === 0 ? ',\n    ' : ', '}`)
            .join('')
            .replace(/,\s*$/, '') +
        `\n};`;

    fs.writeFileSync(path.join(__dirname, filename), content);
    console.log(`Saved ${filename}`);
}

[256, 512, 8192].forEach((SIZE) => {
    // 1. SINE Table [0, 2*PI]
    const sine = Array.from({ length: SIZE }, (_, i) => Math.sin((i / SIZE) * 2 * Math.PI));
    saveHeader(`sineTable${SIZE}.h`, 'SINE_TABLE', sine);

    // 2. TANH Table [-5.0, 5.0]
    const tanh = Array.from({ length: SIZE }, (_, i) => {
        const x = (i / (SIZE - 1)) * 10 - 5;
        return Math.tanh(x);
    });
    saveHeader(`tanhTable${SIZE}.h`, 'TANH_TABLE', tanh);

    // 3. EXPONENTIAL Table [0.0, 1.0] -> [1.0, 0.0]
    // Useful for super fast envelope curves without calling expf()
    const exp = Array.from({ length: SIZE }, (_, i) => {
        const x = i / (SIZE - 1);
        return Math.exp(-5 * x); // Adjust -5 for 'curviness'
    });
    saveHeader(`expTable${SIZE}.h`, 'EXP_TABLE', exp);

    // 4. INVERSE Table [1.0, SIZE]
    // Useful because Division is slow. Instead of x / y, do x * inv[y]
    const inv = Array.from({ length: SIZE }, (_, i) => 1.0 / (i + 1));
    saveHeader(`inverseTable${SIZE}.h`, 'INV_TABLE', inv);
});
