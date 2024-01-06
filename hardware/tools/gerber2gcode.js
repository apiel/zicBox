if (process.argv.length < 3) {
    console.log('Usage: node gerber2gcode.js <gerber-file> [speed] [passes]');
    process.exit(1);
}

const file = process.argv[2];
const gerber = require('fs').readFileSync(file, 'utf8');

const speed = process.argv[3] || 100;
const passes = process.argv[4] || 1;

// %MOIN*% -> G20
// %MOMM*% -> G21
// %FSLAX36Y36*% -> format X 3.6 Y 3.6

console.log('M3; Constant Power Laser On');

// set speed
console.log(`F${speed}`);

const Zsafe = 'Z10';
const ZlaserOn = 'Z0';
// const Zsafe = '';
// const ZlaserOn = '';

let ratioX = 0.000001;
let ratioY = 0.000001;

const lines = gerber.split('\n');

for (let counter = 0; counter < passes; counter++) {
    if (counter) {
        console.log(`; Pass ${counter + 1}`);
    }

    for (const line of lines) {
        if (line === '%MOIN*%') {
            console.log('G20');
        } else if (line === '%MOMM*%') {
            console.log('G21');
        } else if (line.startsWith('%FSLAX')) {
            // %FSLAX36Y36*%
            const [x, y] = line.substring(6, line.length - 2).split('Y');
            const xDecimal = Number(x[1]);
            ratioX = Number((0.1 ** xDecimal).toFixed(xDecimal)); // ** = Math.pow = exponentiation operator
            const yDecimal = Number(y[1]);
            ratioY = Number((0.1 ** yDecimal).toFixed(yDecimal));
            // console.log({ ratioX, ratioY });
        } else if (line.endsWith('D02*')) {
            // X4975000Y2200000D02*
            const [x, y] = line.substring(1, line.length - 4).split('Y');
            console.log(`\nG0 X${x * ratioX} Y${y * ratioY} ${Zsafe}`);
        } else if (line.endsWith('D01*')) {
            // X4975000Y475000D01*
            // or
            // X5175000Y250000I225615J-842D01*
            const l = line.replace('G03', '');
            const [x, restY = ''] = l.substring(1, l.length - 4).split('Y');
            const [y, restI = ''] = restY.split('I');
            const [i, j = ''] = restI.split('J');
            // console.log({ line, x, y, i, j });
            if (i !== '' && j !== '') {
                console.log(
                    `G3 X${x * ratioX} Y${y * ratioY} I${i * ratioX} J${j * ratioY} ${ZlaserOn}`
                );
            } else {
                console.log(`G1 X${x * ratioX} Y${y * ratioY} ${ZlaserOn}`);
            }
        }
    }
}
