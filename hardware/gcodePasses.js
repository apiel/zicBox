#!/usr/bin/env node

// preview gcode result https://ncviewer.com/

// User to update speed and increase the number of passes

const fs = require("fs");

console.log(process.argv);

if (process.argv.length < 5) {
  console.log(`Usage: node gcode.js <gcode file> <speed> <passes>`);
  process.exit(1);
}

const fileName = process.argv[2];
const speed = Number(process.argv[3]);
const passes = Number(process.argv[4]);

let outputFile = fileName.replace(".gcode", `_${speed}x${passes}.gcode`);
if (process.argv[5]) {
  outputFile = process.argv[5];
}

if (fileName === outputFile) {
  console.log(`file output is the same, are you sure? Y/n`);
  // TODO yes no input
  process.exit(1);
}

if (isNaN(speed)) {
  console.log(`Speed must be a number`);
  process.exit(1);
}

if (isNaN(passes)) {
  console.log(`Passes must be a number`);
  process.exit(1);
}

let content = fs.readFileSync(fileName, "utf8");
// we want to keep laser head at the current position and not go to home
content = content.replaceAll("G90", "");

const match = /G1 F[0-9]+ X/.exec(content);

if (match === null) {
  console.log(
    "Coult not find current speed. Gcode might miss some information..."
  );
  process.exit(1);
}

console.log(`Current speed ${match[0].slice(4, -2)} replaced by ${speed}`);
content = content.replaceAll(`F${match[0].slice(4, -2)}`, `F${speed}`);

const lines = content.split("\n");
let chunks = [];
fs.writeFileSync(outputFile, `; Speed: ${speed} Passes: ${passes}`);
for (const line of lines) {
  if (line.startsWith(`G1 F${speed} X`)) {
    chunks.push(line);
  } else {
    if (chunks.length > 0) {
      if (line.startsWith("; pingpong")) {
        for (let i = 0; i < chunks.length - 2; i++) {
          fs.writeFileSync(
            outputFile,
            `${chunks[i]}\n${chunks[i + 1]}\n`.repeat(passes) + `;---\n`,
            { flag: "a" }
          );
        }
        fs.writeFileSync(outputFile, `${chunks[chunks.length - 1]}\n`, {
          flag: "a",
        });
      } else {
        fs.writeFileSync(
          outputFile,
          `${chunks.join("\n")}\n;---\n`.repeat(passes),
          { flag: "a" }
        );
      }
      chunks = [];
    }
    fs.writeFileSync(outputFile, line + "\n", { flag: "a" });
  }
}

console.log("Done", outputFile);
