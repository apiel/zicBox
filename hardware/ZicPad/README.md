# 06 Hardware

## ZicPad

ZicPad is an advance hardware version of ZicBox with a big touch screen, 12 encoders and a 4x12 RGB keypad.

<img src="https://raw.githubusercontent.com/apiel/zicBox/main/hardware/ZicPad/zicpad.jpg" />

### Material

TBD.

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
- Laser cutted wood or acrylic

The rest of the enclosure is hand made out of wood.