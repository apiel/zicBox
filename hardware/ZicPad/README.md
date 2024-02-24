# 07 Hardware

## ZicPad

ZicPad is an advance hardware version of ZicBox with a big touch screen, 12 encoders and a 4x12 RGB keypad.

<img src="https://raw.githubusercontent.com/apiel/zicBox/main/hardware/ZicPad/zicpad.png" />

Note that this design doesn't have DAC and this might be problematic for some of us. The built-in DAC from RPi4 did improve a little bit but is far from being optimal. When I made this design, I was building it with using USB as audio interface in mind, in my case using my Elektron Digitone or my Behringer Xenyx 302 USB mixer... This is the point, many device today support audio in/out over USB and this is why I prefered to had more encoder instead of using a DAC. Having a DAC would not be so hard, but would require some small change on the PCB to move the encoder on a separate ICs instead to connect them directly to the RPi GPIO.

### Material

- 1x [Adafruit NeoTrellis M4 Express](https://learn.adafruit.com/adafruit-neotrellis-m4/overview) [37€ on mouser](https://www.mouser.at/ProductDetail/Adafruit/3938?qs=Zz7%252BYVVL6bG3C5a64%2FPLvA%3D%3D) 
- 1x [Adafruit NeoTrellis](https://learn.adafruit.com/adafruit-neotrellis/overview) [12€ on mouser](https://www.mouser.at/ProductDetail/Adafruit/3954?qs=byeeYqUIh0NVHgHNdfReRA%3D%3D)
- 3x [Silicone Elastomer 4x4 Button Keypad](https://www.adafruit.com/product/1611) [4.60€ on mouser](https://www.mouser.at/ProductDetail/Adafruit/1611?qs=GURawfaeGuDeTqOS7H3MnQ%3D%3D)
- 1x [2.8inch Capacitive Touch Display for Raspberry Pi, 480×640, DSI](https://www.waveshare.com/2.8inch-dsi-lcd.htm) 40€ on Geekbuying, 60€ on Amazon
- 12x encoders wihtout detent [PEC12R-4025F-N0024](https://eu.mouser.com/ProductDetail/Bourns/PEC12R-4025F-N0024?qs=Zq5ylnUbLm4HSBD7%2FFgU%2FA%3D%3D&countryCode=DE&currencyCode=EUR&_gl=1*1nd7s7x*_ga*Nzc0OTY5NDMwLjE2OTg1MDM2NzE.*_ga_15W4STQT4T*MTcwNTk0NTcwNi4xMi4wLjE3MDU5NDU3MDcuNTkuMC4w*_ga_1KQLCYKRX3*MTcwNTk0NTcwNi4yLjAuMTcwNTk0NTcwNy4wLjAuMA..) 1.13€ on mouser (but might get expensive with shipping, could use other encoder to reduce cost)
- 1x PCBs at JLCPCB 4€ inlucing shipping
- 1x Raspberry PI about 50€ depending on the model you get (note that RPi5 doesn't have jack output so the only way to get audio output will be using a USB sound card. At some point I might add a DAC but for this I would need to rework the encoder array to use an I2C IC to free the I2S GPIO)

To reduce cost, I would recommend to order everyhting at once on mouser and get free shipping.

About 190€ without enclosure.

For the enclosure, there is 2 options:
- PCB front panel, about 30€ at JLCPCB
- Laser cutted wood and/or acrylic

The rest of the enclosure is hand made out of wood.

### PCB

<table>
    <tr>
        <td><img src="https://raw.githubusercontent.com/apiel/zicBox/main/hardware/ZicPad/encoders/pcb.png" /></td>
        <td><img src="https://raw.githubusercontent.com/apiel/zicBox/main/hardware/ZicPad/encoders/pcb2d.png" /></td>
    </tr>
</table>

The PCB has been created with [EasyEDA](https://easyeda.com/). You can load the copy of the PCB project using the json file [PCB_zicbox_encoders_2024-01-11.json](https://raw.githubusercontent.com/apiel/zicBox/main/hardware/ZicPad/PCB_zicbox_encoders_2024-01-11.json).

To order the PCB, you need to send the Gerber file [Gerber_zicbox_encoders_2024-01-11.zip](https://github.com/apiel/zicBox/raw/main/hardware/ZicPad/Gerber_zicbox_encoders_2024-01-11.zip) to https://jlcpcb.com/. With slow shipping (about 10 days), it cost around 4€. Just upload the file using the button `Add gerber file` and normally there should be nothing to change. You can eventually change the color from `green` to `black`, it should not impact the price. Finally `Save to cart` and proceed to the checkout.

The PCB assembly is quiet easy, it is just few encoder and pins headers to solder to the board.

### Front panel

For the front panel, it is also possible to order it as a PCB. I haven't yet tried myself, as it is more expensive due to the size (around 30€). I am waiting for the graphic design before to order it. So far, I made the front panel using my laser engraver to cut wood and acrylic sheet. To do this, I was converting the gerber file of PCB outline to Gcode, using a small script. For more details about this process, see my [notes](https://github.com/apiel/zicBox/blob/main/hardware/ZicPad/case/NOTE.md) in the case folder.

The EasyEDA file  and the gerber file for the front panel are available [here](https://github.com/apiel/zicBox/tree/main/hardware/ZicPad/case/easyeda).

### Case enclosure

The rest of the enclosure has been fully hand made out of wood, using a saw, drill and glue. For this part, it will be up to your own creativity :p till someone come up with 3d model.

<table>
    <tr>
        <td><img src="https://raw.githubusercontent.com/apiel/zicBox/main/hardware/ZicPad/case/photo/build0.png" /></td>
        <td><img src="https://raw.githubusercontent.com/apiel/zicBox/main/hardware/ZicPad/case/photo/build1.png" /></td>
        <td><img src="https://raw.githubusercontent.com/apiel/zicBox/main/hardware/ZicPad/case/photo/build2.png" /></td>
    </tr>
</table>
