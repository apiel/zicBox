## Kicad pcb editor

The PCB was created with [kicad](https://www.kicad.org/). 

```sh
sudo apt-get install kicad
```

#### Export as SVG

- Select only `F.Cu`
- Set print mode to `Black and white`
- Set SVG Page Size to `Board area only`

![alt text](encoder-F_Cu.svg)

> Might want to optimize the svg: `npx svgo encoder-F_Cu.svg -o encoder-F_Cu.optimized.svg`

## SVG to Gcode

To convert SVG to Gcode, use https://cam.openbuilds.com/


