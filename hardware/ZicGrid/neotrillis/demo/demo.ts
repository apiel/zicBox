/** Description:
This program serves as a dedicated software controller for an external piece of hardware, likely a custom keypad, button matrix, or an LED lighting rig, using a standard serial connection (like a USB cable connecting to a microcontroller).

### Core Functionality

1.  **Establishing Connection:** The program first opens a communication line to the external device located on a specific virtual port (`/dev/ttyACM0`). It sets a high speed for data transfer (115,200 baud rate) to ensure fast and reliable communication. Once the connection is confirmed, it sends initial commands to light up a few specific buttons.

2.  **Sending Commands (Output):** The controller sends small, coded messages to the hardware. A dedicated function allows the program to specify which button to target and which color/brightness level it should display. These messages use internal codes (like the `%` character) to identify the "set button state" command.

3.  **Receiving Feedback (Input):** The program constantly listens for incoming data from the hardware. It uses a specialized reader that processes data line-by-line. It looks for specific symbols at the start of the message:
    *   If the data begins with a `$` sign, the program recognizes that a button has been pressed.
    *   If the data begins with a `!` sign, it knows a button has been released.
    *   Pressing a specific target key (key 7) triggers a major demonstration sequence.

4.  **The Light Show (`loop`):** When triggered, the program runs a dynamic, continuous sequence. It sends rapid commands (using a different internal code, `#`) to cycle through all available buttons and test all possible colors and brightness levels sequentially. This creates a sweeping or flashing light demonstration across the connected hardware, proving that all parts are functional.

sha: ff9c830fb79491c583a1cb8bc8943dc131bef4d9514328f34617130aff71a0a9 
*/
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
