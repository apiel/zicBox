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

const fileName = process.argv[2];
// remove outline file files
fs.unlinkSync(fileName.replace("-top.gcode", "-outline.gcode"));

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
// Change speed
content = content.replaceAll("F50.000000", "F2000");

fs.writeFileSync(outputFile, content);

console.log("Done", outputFile);
