# PCB for zicBox

![encoder pcb](encoder-top.gcode.png "encoder pcb")

## gEDA pcb editor

gEDA editor, has builtin gcode export.

```sh
sudo apt-get install pcb
```

Export as gcode, uncheck all box (predrill, drill-mill, advanced-gcode)

Fix gcode to work for laser engraver: `nodejs gcode4laser.js encoder-top.gcode`

> To flip/mirror the layout: 
> - select all element
> - buffer > cut to buffer
> - buffer > mirror buffer (left/right) 

### Settings

- View > Enable visible grid
- Set grid size to 100 mil (this is the specing between pin)
