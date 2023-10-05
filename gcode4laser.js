#!/usr/bin/env node

// preview gcode result https://ncviewer.com/
//
// use cam.openbuilds.com to convert SVG to gcode

const fs = require("fs");

console.log(process.argv);

if (process.argv.length < 1) {
  console.log(`Usage: node gcode4laser.js <gcode file> (<output file>)`);
  process.exit(1);
}

const speed = "F1000";

const fileName = process.argv[2];

try {
  // remove outline file files
  fs.unlinkSync(fileName.replace("-top.gcode", "-outline.gcode"));
} catch (e) { }

const outputFile = process.argv[3] ? process.argv[3] : fileName;

let content = fs.readFileSync(fileName, "utf8");

// remove comments
content = content.replace(/ *\([^)]*\) */g, "");

// find top position
const top = content.indexOf("G0 Z2.000000");
content = content.substring(top);

content = `
G21 ; mm-mode
G54; Work Coordinates
G21; mm-mode
M3; Constant Power Laser On

${content}`;

// Fix Z safe zone (laser off)
content = content.replaceAll("G0 Z2.000000", "G0 Z10");
// Fix Z working zone (laser on)
content = content.replaceAll("G1 Z-0.050000 F25.000000", "G0 Z0");


// Change speed (maybe we could simply remove it completely)
content = content.replaceAll("F50.000000", speed);
// Change speed and start laser
content = content.replaceAll(/G1 (X[0-9.]* Y[0-9.]*)/g, `G1 ${speed} $1 Z-0.1000 S1000`);
// other options less strict
// content = content.replaceAll(/G1 X/g, `G1 F1000 Z-0.1000 S1000 X`);

fs.writeFileSync(outputFile, content);

console.log("Done", outputFile);
