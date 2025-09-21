# 05 Hardware

<table>
    <tr>
        <td><a href="https://github.com/apiel/zicBox/wiki/10-Zic-Grid"><b>Zic Grid</b></a></td>
        <td><a href="https://github.com/apiel/zicBox/wiki/15-Zic-Pixel"><b>Zic Pixel</b></a></td>
    </tr>
    <tr>
        <td>
            <a href="https://github.com/apiel/zicBox/wiki/10-Zic-Grid">
                <img src="https://raw.githubusercontent.com/apiel/zicBox/main/hardware/ZicGrid/zicgrid.png" />
            </a>
        </td>
        <td>
            <a href="https://github.com/apiel/zicBox/wiki/15-Zic-Pixel">
                <img src="https://github.com/apiel/zicBox/blob/main/hardware/ZicPixel/pixel_12btn.png" width='480' />
            </a>
        </td>
    </tr>
</table>

> [!TIP]
> Before to order pcb, I would recommend you to join the [discord channel](https://discord.gg/65HTx7z9qg) to have a chat on the current status of the project and see which version might be the most suited for you. This project is in constant evolution and new PCB versions are regularly designed.

## PCB

PCBs are designed with [EasyEDA](https://easyeda.com/). You can load the copy of the projects using the easyeda json file available in the different folder of the builds.

To order the PCB, you need to send the Gerber file (gerber*.zip) to https://jlcpcb.com/. With slow shipping (about 10 days), it cost around 4€. Just upload the file using the button `Add gerber file` and normally there should be nothing to change. You can eventually change the color from `green` to `black`, it should not impact the price. Finally `Save to cart` and proceed to the checkout.

## Audio DAC

In my builds, I am using PCM5102 a cheap but good quality DAC that can be found on Amazon and Aliexpress. Depending where you buy it, you might need to solder some jumper on the DAC.

<img src="https://raw.githubusercontent.com/apiel/zicBox/main/hardware/pcm5102_00.png" />
<img src="https://raw.githubusercontent.com/apiel/zicBox/main/hardware/pcm5102_01.png" />
<img src="https://raw.githubusercontent.com/apiel/zicBox/main/hardware/pcm5102_02.png" />
