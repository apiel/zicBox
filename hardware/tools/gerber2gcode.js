// https://ncviewer.com/ to preview gcode result
// https://marlinfw.org/docs/gcode/ gcode doc

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

out('M3 ; Constant Power Laser On');

// Set MM mode to set speed rate, then it might switch to INCH if gerber file require it
out('G21 ; mm-mode');
out(`G0 F1000 ; set move speed rate to 1000 mm/min`);
out(`G1 F${speed} ; set line speed rate to ${speed} mm/min`);
// out(`G3 F${speed} ; set arc speed rate to ${speed} mm/min`);

const Zsafe = 'Z10';
const ZlaserOn = 'Z0';
const ZlaserWork = 'S1000';
// const Zsafe = '';
// const ZlaserOn = '';

let ratioX = 0.000001;
let ratioY = 0.000001;
let origin; // use first coordinate as origin

out('\nG92 X0 Y0 ; set current head position to X0 Y0');

const lines = gerber.split('\n');

for (let counter = 0; counter < passes; counter++) {
    if (counter) {
        out(`\n\n; Pass ${counter + 1}\n`);
    }

    for (const line of lines) {
        if (line === '%MOIN*%') {
            out('G20 ; inch-mode');
        } else if (line === '%MOMM*%') {
            out('G21 ; mm-mode');
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
            // if (!origin) {
            //     out(`\nG92 X${x * ratioX} Y${y * ratioY} ; set current head position to first coordinate`);
            //     origin = {x, y};
            // }
            out(`\nG0 ${Zsafe}`);
            out(`G0 X${x * ratioX} Y${y * ratioY}`);
            out(`G0 ${ZlaserOn}`);
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
                out(`G3 X${x * ratioX} Y${y * ratioY} I${i * ratioX} J${j * ratioY} ${ZlaserWork}`);
            } else {
                out(`G1 X${x * ratioX} Y${y * ratioY} ${ZlaserWork}`);
            }
        }
    }
    out(`\nG0 ${Zsafe}`);
}

out(`\nG0 ${Zsafe}`);
// out(`G0 X${origin.x * ratioX} Y${origin.y * ratioY} ; go back to origin`);
// out(`G0 X${(origin.x - 0.1) * ratioX} Y${(origin.y - 0.1) * ratioY}`);
// out(`G0 X${origin.x * ratioX} Y${origin.y * ratioY}`);
out(`M5 ; Laser Off`);

outRelease();
