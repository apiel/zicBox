// https://ncviewer.com/ to preview gcode result
// https://marlinfw.org/docs/gcode/ gcode doc
//
// This should not be necessary, https://xyzbots.com/gcode-optimizer/ should be enough

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
    console.log(
        'Have you check if https://xyzbots.com/gcode-optimizer/ is not doing a better job??'
    );

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

let arcG = 'G2';

out('\nG92 X0 Y0 ; set current head position to X0 Y0');

const lines = gerber.split('\n');

let commandes = [];

for (const line of lines) {
    if (line.startsWith('G03*')) {
        arcG = 'G3';
    } else if (line.startsWith('G02*')) {
        arcG = 'G2';
    } else if (line === '%MOIN*%') {
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
    } else if (line.endsWith('D02*')) {
        const [x, y] = line.substring(1, line.length - 4).split('Y');
        commandes.push([{ cmd: 'G0', x: getX(x), y: getY(y) }]);
    } else if (line.endsWith('D01*')) {
        const l = line.replace('G03', '');
        const [x, restY = ''] = l.substring(1, l.length - 4).split('Y');
        const [y, restI = ''] = restY.split('I');
        const [i, j = ''] = restI.split('J');
        if (i !== '' && j !== '') {
            commandes[commandes.length - 1].push({
                cmd: arcG,
                x: getX(x),
                y: getY(y),
                i: getX(i),
                j: getY(j),
            });
        } else {
            commandes[commandes.length - 1].push({ cmd: 'G1', x: getX(x), y: getY(y) });
        }
    }
}

commandes = commandes.sort((a, b) => {
    return b[0].y - a[0].y;
});

commandes = commandes.sort((a, b) => {
    return b[0].x - a[0].x;
});

for (let counter = 0; counter < passes; counter++) {
    if (counter) {
        out(`\n\n; Pass ${counter + 1}\n`);
    }
    for (const chunk of commandes) {
        for (const command of chunk) {
            if (command.cmd === 'G0') {
                out(`\nG0 ${Zsafe}`);
                out(`G0 X${command.x} Y${command.y}`);
                out(`G0 ${ZlaserOn}`);
            } else if (command.cmd === 'G1') {
                out(`G1 X${command.x} Y${command.y} ${ZlaserWork}`);
            } else if (command.cmd === 'G2') {
                out(`G2 X${command.x} Y${command.y} I${command.i} J${command.j} ${ZlaserWork}`);
            } else if (command.cmd === 'G3') {
                out(`G3 X${command.x} Y${command.y} I${command.i} J${command.j} ${ZlaserWork}`);
            }
        }
    }
}

out(`\nG0 ${Zsafe}`);
out(`M5 ; Laser Off`);

outRelease();

function round4(x) {
    if (process.env.SKIP_ROUNDING) {
        return x;
    }
    return Math.round(x * 1000) / 1000;
}

function getX(x) {
    return round4(x * ratioX);
}

function getY(y) {
    return round4(y * ratioY);
    I;
}
