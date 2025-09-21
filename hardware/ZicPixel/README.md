# 04 Hardware

## Zic Pixel

<img src='https://github.com/apiel/zicBox/blob/main/hardware/ZicPixel/pixel_12btn.png?raw=true' height='350'>

<table>
<tr>
    <th>Image</th>
    <th>Qty</th>
    <th>Description</th>
    <th>Price</th>
<tr>
<tr>
    <td><img src='https://github.com/apiel/zicBox/blob/main/hardware/rpi0.png?raw=true' width='100'></td>
    <td>1</td>
    <td>Raspberry Pi zero 2w</td>
    <td>23.00€ for 1 piece at <a href="https://www.amazon.de/-/en/Raspberry-Pi-Zero-2-W-multi-coloured/dp/B09KLVX4RT">Amazon</a></td>
</tr>
<tr>
    <td><img src='https://github.com/apiel/zicBox/blob/main/hardware/encoderPEC12R.png?raw=true' width='100'></td>
    <td>4</td>
    <td>Encoder PEC12R-4025F-N0024</td>
    <td>1.04€ per piece at <a href="https://eu.mouser.com/ProductDetail/Bourns/PEC12R-4025F-N0024?qs=Zq5ylnUbLm4HSBD7%2FFgU%2FA%3D%3D&countryCode=DE&currencyCode=EUR&_gl=1*1nd7s7x*_ga*Nzc0OTY5NDMwLjE2OTg1MDM2NzE.*_ga_15W4STQT4T*MTcwNTk0NTcwNi4xMi4wLjE3MDU5NDU3MDcuNTkuMC4w*_ga_1KQLCYKRX3*MTcwNTk0NTcwNi4yLjAuMTcwNTk0NTcwNy4wLjAuMA..">mouser</a></td>
</tr>
<tr>
    <td><img src='https://github.com/apiel/zicBox/blob/main/hardware/keysRedragon.png?raw=true' width='100'></td>
    <td>12</td>
    <td>REDRAGON SMD RGB MX Low Profile 5.5 Switch 3Pin</td>
    <td>4.17€ for 30 piece at <a href="https://www.aliexpress.com/item/1005005895774028.html">Aliexpress</a></td>
</tr>
<tr>
    <td><img src='https://github.com/apiel/zicBox/blob/main/hardware/keycap.png?raw=true' width='100'></td>
    <td>12</td>
    <td>Key Caps for Mx Switch Mechanical Keyboard</td>
    <td>4.66€ for 20 piece at <a href="https://www.aliexpress.com/item/1005008052800345.html">Aliexpress</a></td>
</tr>
<tr>
    <td><img src='https://github.com/apiel/zicBox/blob/main/hardware/1.9screen.png?raw=true' width='100'></td>
    <td>1</td>
    <td>1.9 inch Screen Color Display SPI 170x320 ST7789</td>
    <td>3.50€ for 1 piece at <a href="https://www.aliexpress.com/item/1005007321299056.html">Aliexpress</a></td>
</tr>
<tr>
    <td><img src='https://github.com/apiel/zicBox/blob/main/hardware/20x2female.png?raw=true' width='100'></td>
    <td>1</td>
    <td>2x20 Pin Header female</td>
    <td>3.70€ for 10 piece at <a href="https://www.aliexpress.com/item/1005001340091287.html">Aliexpress</a></td>
</tr>
<tr>
    <td><img src='https://github.com/apiel/zicBox/blob/main/hardware/20x2male.png?raw=true' width='100'></td>
    <td>1</td>
    <td>2x20 Pin Header male</td>
    <td>2.60€ for 10 piece at <a href="https://www.aliexpress.com/item/32848774255.html">Aliexpress</a></td>
</tr>
<tr>
    <td><img src='https://github.com/apiel/zicBox/blob/main/hardware/dac.png?raw=true' width='100'></td>
    <td>1</td>
    <td>PCM5102A DAC</td>
    <td>2.73€ for 1 piece at <a href="https://www.aliexpress.com/item/1005006198619536.html">Aliexpress</a></td>
</tr>
</table>

### PCB

The PCB has been designed using EasyEDA. To access the project, use the following link:

https://easyeda.com/editor#id=5cd71bb61a6c49ae8e3dccd67d79cbfc (wip)

To get files to send to Jlcpcb, click on "Generate PCB Fabrication File (Gerber)"

If you need to modify the PCB, clone the project in your own account.

**Ordering the PCB**
To manufacture the PCB, you need to generate and submit the Gerber files to a PCB fabrication service.

You can order the PCB from JLCPCB by following these steps:

1. Visit JLCPCB.
2. Click on the "Add Gerber File" button and upload the provided Gerber file (.zip).
3. The default settings should work fine, and no changes are necessary.
4. If desired, you can change the PCB color from green to black—this typically does not affect the price.
5. Click "Save to Cart", then proceed to checkout.
   
💰 **Estimated Cost**: ~4€ (with standard shipping, approximately 10 days delivery time).

<table>
<tr>
    <td><img src='https://github.com/apiel/zicBox/blob/main/hardware/ZicPixel/pcb.png?raw=true' width='400'></td>
    <td><img src='https://github.com/apiel/zicBox/blob/main/hardware/ZicPixel/pcb2d.png?raw=true' width='400'></td>
</tr>
</table>

### Assembly

Since PCB is constantly evolving, this tutorial might be outdated.

<img src='https://github.com/apiel/zicBox/blob/main/hardware/ZicPixel/assembly/pcb_00.png?raw=true' width='700'>

Place the pin header of the display onto the board, noting its unusual orientation. The pins must be soldered **only from the top side**, ensuring that they do not protrude through to the underside. This is important because additional components will be mounted on the opposite side, and the space must remain free of obstructions.

<img src='https://github.com/apiel/zicBox/blob/main/hardware/ZicPixel/assembly/pcb_01.png?raw=true' width='700'>

On the back side, you should see the final result, the pins do not come through the board.

<img src='https://github.com/apiel/zicBox/blob/main/hardware/ZicPixel/assembly/pcb_02.png?raw=true'>

Now place the DAC pin header in the same way as you did for the display pin header, and solder it.

<img src='https://github.com/apiel/zicBox/blob/main/hardware/ZicPixel/assembly/pcb_03.png?raw=true' width='700'>

Place the four encoders and solder them in position. Then place the Raspberry Pi female pin header and solder it as well.

Don’t forget to trim the pins step by step as you place and solder each component. This will help keep the board neat and ensure that everything fits together correctly.

<img src='https://github.com/apiel/zicBox/blob/main/hardware/ZicPixel/assembly/pcb_04.png?raw=true' width='700'>

Place the DAC onto the pins and solder it in place. Then solder the jumper on the DAC as [described at the top of this page](https://github.com/apiel/zicBox/wiki/04-Hardware#audio-dac).

<table>
<tr>
    <td><img src='https://github.com/apiel/zicBox/blob/main/hardware/ZicPixel/assembly/pcb_05.png?raw=true' width='400'></td>
    <td><img src='https://github.com/apiel/zicBox/blob/main/hardware/ZicPixel/assembly/pcb_05_bis.png?raw=true' width='400'></td>
</tr>
</table>

Place the display onto the pins and solder it in place. Make sure it is aligned straight and positioned as low as possible, otherwise it may interfere with the keyboard button.

<img src='https://github.com/apiel/zicBox/blob/main/hardware/ZicPixel/assembly/pcb_06.png?raw=true' width='700'>

Finally, solder the Raspberry Pi male pin header onto the Raspberry Pi.

<img src='https://github.com/apiel/zicBox/blob/main/hardware/ZicPixel/assembly/pcb_07.png?raw=true' width='700'>

Once soldered, connect the Raspberry Pi to the board.

<img src='https://github.com/apiel/zicBox/blob/main/hardware/ZicPixel/assembly/pcb_08.png?raw=true' width='700'>

---

*Another old prototype of the Pixel*:

<table>
<tr>
    <td>
<img src='https://github.com/apiel/zicBox/blob/main/hardware/ZicPixel/pixel.png?raw=true' height='220'>
    </td><td>
<img src='https://github.com/apiel/zicBox/blob/main/hardware/ZicPixel/pixel3A.png?raw=true' height='220'>
    </td><td>
<img src='https://github.com/apiel/zicBox/blob/main/hardware/ZicPixel/pixel3A_2.png?raw=true' height='220'>
    </td>
</tr>
</table>