### Case

> This is a prototype doc to keep note on how I made my journey to make the enclosure

For prototype enclosure, I am using TWOTREES TTS-55 laser engraver and the ultimate goal is to make a front panel out of PCB. Concerning the rest of the enclosure, it is still not define but for the moment I will keep it out of wood.

Prototype are first made out of cardboard and then out of 4mm plywood or acrylic board.

Since the end goal (at least for the front panel) is to make it out of PCB board, the design is done using [easyeda](https://easyeda.com/). Once design is done, the project is exported in gerber files. To be able to engrave the panel, it need to be converted to gcode code. For this, extract the outline `.GKO` file from the gerber zip file. Then use the nodejs script to convert the gerber file to gcode:

```sh
# For plywood
node gerber2gcode.js outline.GKO 350 10

# For cardboard
node gerber2gcode.js outline.GKO 400 3

# For acrylic board black 1mm
node gerber2gcode.js outline.GKO 350 5
```

Following are the number of passes and speed base on my on test:

- 4 mm plywood at 350 speed with 8 passes
- cardboard at 400 speed with 3 passes
- acrylic black 1mm 350 speed with 4 passes

> those setting can really vary depending on many thing, so you will first have to experiment...

Some hint:

- https://ncviewer.com/ to preview the gcode result
- https://xyzbots.com/gcode-optimizer/ to optimized gocde output, however so far it didn't made a huge difference. Also when this is done, it should be done for a single pass mode (so if you need multiple passes, you would have to generate it manually)
