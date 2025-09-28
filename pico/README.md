# 20 Zic Pico

Unlike other versions of zicBox that are based on Linux, **Zic Pico** is built on a microcontroller using the **RP2040**. The goal is to make zicBox as **portable as possible**, spanning from desktop computers to SBCs and MCU boards. This version is designed to be much **lighter and cheaper** to build.

The RP2040 offers several advantages making it versatile for multiple types of interfaces:
- ADC inputs: potentiometers can be connected directly without extra hardware.
- Capacitive touch sensing: supports touch controls without additional components.




| PCM5102 | RP2040 |
| ------- | ------ |
| SCK     | GND    |
| DIN     | GP9    |
| BCK     | GP10   |
| LRCK    | GP11   |
| GND     | GND    |
| VIN     | Vout   |

