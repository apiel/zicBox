import { SerialPort, ReadlineParser } from "serialport";

const port = new SerialPort({
  path: "/dev/ttyACM0",
  baudRate: 115200,
  // baudRate: 921600,
});

function setButton(key: number, color: number) {
  const state: Uint8Array = new Uint8Array(3);
  state[0] = 0x25; // equivalent to '%'
  state[1] = key;
  state[2] = color;
  port.write(state);
}

port.on("open", () => {
  console.log("serial port was open");
  // 36, 37, 38, 25
  setButton(36, 10);
  setButton(37, 10);
  setButton(38, 10);
  setButton(25, 10);
});

const parser = port.pipe(new ReadlineParser());
parser.on("data", (data) => {
  console.log(`data: '${data}'`);
  if (data[0] === "$") {
    const key = data[1].charCodeAt(0);
    console.log("pressed: ", key);
    if (key === 7) {
      loop();
    }
  } else if (data[0] === "!") {
    console.log("released: ", data[1].charCodeAt(0));
  } else {
    console.log("unknown: ", data);
  }
});

const sleep = (ms: number) => new Promise((resolve) => setTimeout(resolve, ms));

const buttonCount = 12 * 4;
const colorCount = 16 * 10; // 16 colors of 10 brightness
// 255 is reserved for no color

async function loop() {
  const state: Uint8Array = new Uint8Array(3);

  state[0] = 0x23; // equivalent to '#' command to set color
  for (let i = 7; i < buttonCount; i++) {
    state[1] = i;
    for (let j = 0; j < colorCount; j++) {
      state[2] = j;
      port.write(state);
      // console.log({state});
      await sleep(50);
    }
    state[2] = 254; // button color off
    port.write(state);
    // console.log({state});
    await sleep(50);
  }
}
