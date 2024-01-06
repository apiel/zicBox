// use https://ncviewer.com/ to preview gcode result
//

if (process.argv.length < 3) {
    console.log('Usage: node gerber2gcode.js <gerber-file> [speed] [passes]');
    process.exit(1);
}

const file = process.argv[2];
const gerber = require('fs').readFileSync(file, 'utf8');

const speed = process.argv[3] || 100;
const passes = process.argv[4] || 1;

let out = console.log;
let outRelease = () => {};

if (process.env.OUTPUT !== 'console') {
    const outputFilepath = file.replace(/\.[^/.]+$/, '') + `_F${speed}_x${passes}.gcode`;
    const outputFile = require('fs').createWriteStream(outputFilepath);
    out = (line) => outputFile.write(line + '\n');
    outRelease = () => {
        outputFile.end();
    };
}

// %MOIN*% -> G20
// %MOMM*% -> G21
// %FSLAX36Y36*% -> format X 3.6 Y 3.6

out('M3; Constant Power Laser On');

// set speed
out(`F${speed}`);

const Zsafe = 'Z10';
const ZlaserOn = 'Z0';
// const Zsafe = '';
// const ZlaserOn = '';

let ratioX = 0.000001;
let ratioY = 0.000001;

const lines = gerber.split('\n');

for (let counter = 0; counter < passes; counter++) {
    if (counter) {
        out(`; Pass ${counter + 1}`);
    }

    for (const line of lines) {
        if (line === '%MOIN*%') {
            out('G20');
        } else if (line === '%MOMM*%') {
            out('G21');
        } else if (line.startsWith('%FSLAX')) {
            // %FSLAX36Y36*%
            const [x, y] = line.substring(6, line.length - 2).split('Y');
            const xDecimal = Number(x[1]);
            ratioX = Number((0.1 ** xDecimal).toFixed(xDecimal)); // ** = Math.pow = exponentiation operator
            const yDecimal = Number(y[1]);
            ratioY = Number((0.1 ** yDecimal).toFixed(yDecimal));
            // out({ ratioX, ratioY });
        } else if (line.endsWith('D02*')) {
            // X4975000Y2200000D02*
            const [x, y] = line.substring(1, line.length - 4).split('Y');
            out(`\nG0 X${x * ratioX} Y${y * ratioY} ${Zsafe}`);
        } else if (line.endsWith('D01*')) {
            // X4975000Y475000D01*
            // or
            // X5175000Y250000I225615J-842D01*
            const l = line.replace('G03', '');
            const [x, restY = ''] = l.substring(1, l.length - 4).split('Y');
            const [y, restI = ''] = restY.split('I');
            const [i, j = ''] = restI.split('J');
            // out({ line, x, y, i, j });
            if (i !== '' && j !== '') {
                out(`G3 X${x * ratioX} Y${y * ratioY} I${i * ratioX} J${j * ratioY} ${ZlaserOn}`);
            } else {
                out(`G1 X${x * ratioX} Y${y * ratioY} ${ZlaserOn}`);
            }
        }
    }
}

outRelease();
