# 07 Hardware

## ZicPlus

Zic+ is a version of ZicBox with a big touch screen (800x400), 6 encoders (1 push), a 10 keycap and a DAC.

### Material

- 5x encoders wihtout detent [PEC12R-4025F-N0024](https://eu.mouser.com/ProductDetail/Bourns/PEC12R-4025F-N0024?qs=Zq5ylnUbLm4HSBD7%2FFgU%2FA%3D%3D&countryCode=DE&currencyCode=EUR&_gl=1*1nd7s7x*_ga*Nzc0OTY5NDMwLjE2OTg1MDM2NzE.*_ga_15W4STQT4T*MTcwNTk0NTcwNi4xMi4wLjE3MDU5NDU3MDcuNTkuMC4w*_ga_1KQLCYKRX3*MTcwNTk0NTcwNi4yLjAuMTcwNTk0NTcwNy4wLjAuMA..) 1.13€ on mouser (but might get expensive with shipping, could use other encoder to reduce cost)
- 1x PCBs at JLCPCB 4€ inlucing shipping
- 1x Raspberry PI 3 a+
- ... tbd. rest to be defined

### PCB

<table>
    <tr>
        <td><img src="https://raw.githubusercontent.com/apiel/zicBox/main/hardware/ZicPlus/pcb1.png" /></td>
        <td><img src="https://raw.githubusercontent.com/apiel/zicBox/main/hardware/ZicPlus/pcb2.png" /></td>
    </tr>
</table>

The PCB has been created with [EasyEDA](https://easyeda.com/). You can load the copy of the PCB project using the json file [PCB_zicplus_2024-10-05.json](https://raw.githubusercontent.com/apiel/zicBox/main/hardware/ZicPlus/PCB_zicplus_2024-10-05.json).

To order the PCB, you need to send the Gerber file [Gerber_zicplus_2024-10-05.zip](https://github.com/apiel/zicBox/raw/main/hardware/ZicPlus/Gerber_zicplus_2024-10-05.zip) to https://jlcpcb.com/. With slow shipping (about 10 days), it cost around 4€. Just upload the file using the button `Add gerber file` and normally there should be nothing to change. You can eventually change the color from `green` to `black`, it should not impact the price. Finally `Save to cart` and proceed to the checkout.

The PCB assembly is quiet easy, it is just few encoder and pins headers to solder to the board.

### Case enclosure

Not done yet, upcoming...
