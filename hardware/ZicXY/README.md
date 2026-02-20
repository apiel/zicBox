# 20 Zic XY

<img src='https://github.com/apiel/zicBox/blob/main/hardware/ZicXY/Zic_XY_black.png?raw=true' width='350'> <img src='https://github.com/apiel/zicBox/blob/main/hardware/ZicXY/Zic_XY_blue.png?raw=true' width='350'>

The **Zic XY** is a compact digital instrument with 8 versatile tracks. Each track can load any engine — drums, synths, or samples — all handled by a single multi-engine system.  

## Track Layout

| Track Type | Count | Description |
|------------|-------|-------------|
| Multi Engine Tracks | 8 | Each track can load any engine: drum, synth, or sample. Independent parameters for envelope, mixer, and FX routing. |

You can freely mix engines across tracks — e.g., Kick Engine on Track 1, Grain Engine on Track 4, SuperSaw Engine on Track 8.

## Engines

All engines are handled by the **multi-engine system**. Each engine comes with its own controls, but all tracks share the same interface for simplicity.

### Drum Engines
- **Kick Engine:** Sub-heavy bass, punchy or distorted.
- **Clap Engine:** Synthetic handclaps, layered noise bursts.
- **Percussion Engine:** Mid/high hits, congas, toms, abstract clicks.
- **Metallic Engine:** Bells, zaps, robotic clanks.
- **ER Engine:** Sample-based drums with vintage digital feel.
- **FM Engine:** Zappy, glitchy electronic hits.
- ...

### Synth Engines
- **FM Engine:** Classic FM tones from bells to basses.
- **Wavetable Engine:** Morphing digital timbres.
- **Additive Engine:** Clean harmonic tones, evolving pads.
- **SuperSaw Engine:** Layered detuned waves for lush leads and basses.
- **Bass Engine:** Focused low-end for melodic or sub-bass.
- **String Engine:** Physical-model plucked or bowed tones.
- ...

### Sample Engines
- **Mono Engine:** One-shot or loop playback.
- **Grain Engine:** Granular textures and atmospheric drones.
- **AM Engine:** Metallic and tremolo effects from sample modulation.
- **Stretch Engine:** Time-stretching loops and soundscapes.

## Build & Installation

### ⚠️ WARNING

**Zic XY is in an early hardware stage.**  
The PCB **may change**, and the firmware will **not be backward compatible**. GPIO routing, pin assignments, or other design choices can change at any time.  

### Zic XY - Bill of Materials (BOM)

| Image | Qty | Description | Price |
|-------|-----|-------------|-------|
| <img src='https://github.com/apiel/zicBox/blob/main/hardware/rpi0.png?raw=true' width='100'> | 1 | Raspberry Pi Zero 2W | 23.00€ at [Amazon](https://www.amazon.de/-/en/Raspberry-Pi-Zero-2-W-multi-coloured/dp/B09KLVX4RT) |
| <img src='https://github.com/apiel/zicBox/blob/main/hardware/2inScreen8pin.png?raw=true' width='100'> | 1 | 2" SPI Display 8-pin ⚠️ For the display, I cannot find the exact one I used in my order history. Please check the PCB to verify whether the pinout matches. It needs to be a 2-inch, 320×240 display with an ST7789 controller, 8 pins. | Option 1 seems to be the correct one (95% confident): [Option 1](https://de.aliexpress.com/item/1005009314410563.html), [Option 2](https://de.aliexpress.com/item/1005006388926254.html), [Option 3](https://de.aliexpress.com/item/1005007883575666.html) |
| <img src='https://github.com/apiel/zicBox/blob/main/hardware/encoderPEC12R.png?raw=true' width='100'> | 4 | Encoder PEC12R-4025F-N0024<br/>No detents preferred for smoother control. Matches footprint, no push button. | 1.04€ per piece at [Mouser](https://eu.mouser.com/ProductDetail/Bourns/PEC12R-4025F-N0024?qs=Zq5ylnUbLm4HSBD7%2FFgU%2FA%3D%3D&countryCode=DE&currencyCode=EUR&_gl=1*1nd7s7x*_ga*Nzc0OTY5NDMwLjE2OTg1MDM2NzE.*_ga_15W4STQT4T*MTcwNTk0NTcwNi4xMi4wLjE3MDU5NDU3MDcuNTkuMC4w*_ga_1KQLCYKRX3*MTcwNTk0NTcwNi4yLjAuMTcwNTk0NTcwNy4wLjAuMA..) |
| <img src='https://github.com/apiel/zicBox/blob/main/hardware/dac.png?raw=true' width='100'> | 1 | PCM5102A DAC | 3.56€ at [Aliexpress](https://www.aliexpress.us/item/3256809374544552.html) |
| <img src='https://github.com/apiel/zicBox/blob/main/hardware/keysRedragon.png?raw=true' width='100'> | 13 | REDRAGON SMD RGB MX Low Profile 5.5 Switch 3Pin<br/>(or Kailh Low Profile Choc switch) | 4.17€ for 30 pcs at [Aliexpress](https://www.aliexpress.com/item/1005005895774028.html) |
| <img src='https://github.com/apiel/zicBox/blob/main/hardware/keycap.png?raw=true' width='100'> | 13 | Keycaps for MX Switch Mechanical Keyboard | 4.66€ for 20 pcs at [Aliexpress](https://www.aliexpress.com/item/1005008052800345.html) |
|  | 1 | Resistor 660Ω |  |
| <img src='https://github.com/apiel/zicBox/blob/main/hardware/20x2female.png?raw=true' width='100'> | 1 | 2x20 Pin Header female 90° | 3.70€ for 10 pcs at [Aliexpress](https://www.aliexpress.com/item/1005001340091287.html) |
| <img src='https://github.com/apiel/zicBox/blob/main/hardware/20x2male.png?raw=true' width='100'> | 1 | 2x20 Pin Header male 90° | 2.60€ for 10 pcs at [Aliexpress](https://www.aliexpress.com/item/32848774255.html) |
| <img src='https://github.com/apiel/zicBox/blob/main/hardware/male90.png?raw=true' width='100'> <img src='https://github.com/apiel/zicBox/blob/main/hardware/female90.png?raw=true' width='100'> | - | Optional 90° male/female pin headers (recommended for 3D enclosure) | [Male](https://de.aliexpress.com/item/1005008783967126.html) / [Female](https://de.aliexpress.com/item/1005008210914833.html) |
| <img src='https://github.com/apiel/zicBox/blob/main/hardware/slideswitch.png?raw=true' width='100'> | 1 | Optional Slide Switch Micro |  |
| <img src='https://github.com/apiel/zicBox/blob/main/hardware/usbc.png?raw=true' width='100'> | 1 | Optional USB Type-C 2Pin connector | 2€ at [Aliexpress](https://de.aliexpress.com/item/1005004403587119.html) |

### PCB

<img src='https://github.com/apiel/zicBox/blob/main/hardware/ZicXY/pcbXY.png?raw=true'>

The **Zic XY PCB** was designed in **EasyEDA**:  
[Open in EasyEDA](https://easyeda.com/editor#id=7b9b222dd34a41edba30aa5052ab8823) — this is a public shared project.

#### How to get the PCB
1. Open the EasyEDA project link above.
2. Click **Fabrication > PCB Fabrication File**.
3. Download the **Gerber files** (ZIP) needed for manufacturing.

#### Ordering PCB on JLCPCB
1. Go to [JLCPCB](https://jlcpcb.com/).
2. Click **Quote Now** or **Instant Quote**.
3. Upload the Gerber ZIP file from EasyEDA.
4. Select quantity, color, thickness, and other options.
5. Add to cart and place the order.

> Reminder: Double-check the channel before ordering — the PCB may change in upcoming versions, which can break firmware compatibility.

## OS Installation Guide

This guide explains how to install the **custom Zic XY OS** on your hardware.

### 1️⃣ Download the OS

Download the latest release here:  
[Zic OS XY Release](https://github.com/apiel/zicBox/releases/tag/zicOsXY)  

The file to download is: `zicOsXY.zip`

### 2️⃣ Prepare the SD Card

1. Uncompress the downloaded zip file. Ins../../../hardware/ZicXY/assemblyide you will find the `.img` file.  
2. Burn the image to an SD card.

#### Linux Example: Using Raspberry Pi Imager
Install if needed: `sudo apt install rpi-imager`  
Run the image## Assembly

[Read instructions](Assembly.md)r: `rpi-imager`  
- Select **"Use custom"** and choose the `.img` file from the zip.  
- Select your SD card as the target.  
- Write the image.

> On other OS (Windows/macOS), tools like [Balena Etcher](https://www.balena.io/etcher/) work similarly.

### 3️⃣ Resize the SD Card (Optional but Recommended)

The OS image is small to save space, so the full SD card capacity is **not used**.  
Resizing is recommended if you want to store samples or extra files.

#### Option 1: Using the included script
Navigate to the repo folder: `cd <path-to-unzipped-repo>/os`  
Run as root: `sudo ./sd.sh`  

- Must be run as **root**.  
- Script was first designed for me during my testing — use with caution, I don't know if work for everyone.

#### Option 2: Using standard tools
- **Linux:** `gparted` to resize the main partition.  
- **macOS:** `Disk Utility`.  
- **Windows:** `MiniTool Partition Wizard` or similar.

### 4️⃣ Optional: Configure WiFi

If you want WiFi, create the file `/etc/wpa_supplicant.conf` on the SD card root partition:

```sh
network={
    ssid="your_ssid"
    psk="your_password"
}
```

**SSH is installed by default**. You can connect to your Zic XY over the network using: `ssh root@zic.local`

Password: `password`


## 3d model and hardware discussion

Here is a discussion thread about Zic XY hardware: https://github.com/apiel/zicBox/discussions/12

The **3D model** for building the enclosure is included in the discussion.  
I personally print it on my own printer (Bambu Lab A1 Mini), but it can also be ordered from services like JLCPCB.  

If you have your own 3D model or improvements, I’d be **very happy** if you share it.  

> [!TIP]
> If you notice any missing information or think something could be clearer, I would be **very happy** if you raise a **PR** to improve this documentation!

## Assembly

These instructions assume you already have the PCB, everything from this [bill of materials](https://github.com/apiel/zicBox/wiki/20-Zic-XY), and that you have followed the [instructions for installing the OS on you SD card](https://github.com/apiel/zicBox/wiki/20-Zic-XY).

This assembly was made using rev.3 of the Zic XY PCB, with some silly custom silkscreen art.  Please check the [Zic XY Wiki](https://github.com/apiel/zicBox/wiki/20-Zic-XY), or the discord if you are using a different version of the board.

| <img src="https://raw.githubusercontent.com/apiel/zicBox/refs/heads/main/hardware/ZicXY/images/image6.png" width="60%"> <br> *PCB Front* |  <img src="https://raw.githubusercontent.com/apiel/zicBox/refs/heads/main/hardware/ZicXY/images/image16.png" width="60%"> <br> *PCB Back* |
| :---: | :---: |

NOTE: The author is not very skilled at soldering, so apologies in advance for strange and ugly joints.

### **Tools**

* **Soldering iron** (of course)  
* **Flush cutters** (highly recommended).    
There are places where it will be important that the pins don't stick out through the other side of the board.  
<img src="https://raw.githubusercontent.com/apiel/zicBox/refs/heads/main/hardware/ZicXY/images/image2.png" width="40%">  <img src="https://raw.githubusercontent.com/apiel/zicBox/refs/heads/main/hardware/ZicXY/images/image21.png" width="40%">    

### **■ Solder the 5V jumper (if you're not adding your own battery power)**

On the front of the board, solder the 2 pads below.  

*NOTE: If you're adding battery power, you shouldn't do this.*  
<img src="https://raw.githubusercontent.com/apiel/zicBox/refs/heads/main/hardware/ZicXY/images/image17.png" width="60%"> 

### **■ Solder jumpers on the DAC**

On the back of the DAC, Solder jumpers as pictured in the yellow square below   

| <img src="https://raw.githubusercontent.com/apiel/zicBox/refs/heads/main/hardware/ZicXY/images/image22.png" width="70%">  | 1-\>L<br>2-\>L<br>3-\>H<br>4-\>L  |
| :---- | :---: |

*More info here about the function of these jumpers here:*  
[https://github.com/apiel/zicBox/wiki/05-Hardware\#audio-dac](https://github.com/apiel/zicBox/wiki/05-Hardware#audio-dac)

### **■ Fit the 4 encoders into the PCB**

It is recommended that you leave them unsoldered for now.    
<img src="https://raw.githubusercontent.com/apiel/zicBox/refs/heads/main/hardware/ZicXY/images/image29.png" width="70%"> 

### **■ Fit the left 10 keyswitches, leaving the 3 bottom right keys empty for now**

It is recommended that you leave them unsoldered until the rest of the assembly is complete.  The keyswitches in particular may be fine to never solder, if you are satisfied that they are fit securely.  
<img src="https://raw.githubusercontent.com/apiel/zicBox/refs/heads/main/hardware/ZicXY/images/image10.png" width="70%"> 

**NOTE**  
You might notice that there are 3 pin-holes in the PCB for each keyswitch, but your switches may only have 2 pins.  That's OK. Just insert your switch as is. The 3rd hole is present for compatibility with other kinds of switches.

### **■ Solder 660ohm resistor into the 2 holes labeled R660, onto the *back* of the PCB**

Do your best to have the resistor legs / solder joints not stick up too much.  The screen will be placed flat against the PCB on top of where these legs come out. 

| <img src="https://raw.githubusercontent.com/apiel/zicBox/refs/heads/main/hardware/ZicXY/images/image3.png" width="100%">  | <span style="font-size: 400%">➧</span> |<img src="https://raw.githubusercontent.com/apiel/zicBox/refs/heads/main/hardware/ZicXY/images/image19.png" width="70%">  |
| :---: | :---: | :---: |

<img src="https://raw.githubusercontent.com/apiel/zicBox/refs/heads/main/hardware/ZicXY/images/image8.png" width="70%"> 
<br>
<i>This is not great. The solder joints / resistor legs are poking out too much, and will interfere with screen sitting flat against the PCB.</i>

### **■ If your screen has pre-soldered header pins, carefully remove the black plastic base from the pins**

| <img src="https://raw.githubusercontent.com/apiel/zicBox/refs/heads/main/hardware/ZicXY/images/image12.png" width="100%">  | <img src="https://raw.githubusercontent.com/apiel/zicBox/refs/heads/main/hardware/ZicXY/images/image4.png" width="100%">  *Carefully remove this black plastic from the pins, so the screen can sit flush against the PCB* |
| :---- | :---- |

If you skip this step, the screen will not sit flush against the PCB, and is only held up by its header pins on the right side (see the gap pictured below).  You may be able to add in some kind spacer, but if you're planning on using the 3D printed case it may no longer fit.  
<img src="https://raw.githubusercontent.com/apiel/zicBox/refs/heads/main/hardware/ZicXY/images/image32.png" width="70%"> 

### **■ Solder screen to front**
../../../hardware/ZicXY/assembly
| <img src="https://raw.githubusercontent.com/apiel/zicBox/refs/heads/main/hardware/ZicXY/images/image24.jpg" width="100%">  *Front of board with the screen* | <img src="https://raw.githubusercontent.com/apiel/zicBox/refs/heads/main/hardware/ZicXY/images/image20.png" width="70%">  <br>*Back, showing pin holes.  The author chose here to leave the black plastic piece on.* |
| :---: | :---: |

### **■ Solder 90° 2x20 female pin header to *back* of board, with holes facing up**

This will be where the RPi will be plugged in.  It should be flush to the board.  
<img src="https://raw.githubusercontent.com/apiel/zicBox/refs/heads/main/hardware/ZicXY/images/image28.png" width="30%"> <img src="https://raw.githubusercontent.com/apiel/zicBox/refs/heads/main/hardware/ZicXY/images/image1.png" width="40%"> 
<p align="center">The header should sit flush to the board</p>

**NOTE:**
|<img src="https://raw.githubusercontent.com/apiel/zicBox/refs/heads/main/hardware/ZicXY/images/image18.png" width="60%"> <br> It is recommended that you clip the legs that come out on the front of the board so the last 3 key switches can sit flush to the PCB. | <img src="https://raw.githubusercontent.com/apiel/zicBox/refs/heads/main/hardware/ZicXY/images/image30.png" width="60%"><br>*Here, the unclipped pins are preventing the switch from sitting flush. Not the end of the world, but not ideal.*  |
| :---- | :---- |

### **■ Solder the 2x20 male pin headers to the top of the RPi, with the long parts of the legs pointing away from the ports**

<img src="https://raw.githubusercontent.com/apiel/zicBox/refs/heads/main/hardware/ZicXY/images/image23.png" width="40%"> 

### **■ Plug the RPi into the 90° female header**

<img src="https://raw.githubusercontent.com/apiel/zicBox/refs/heads/main/hardware/ZicXY/images/image5.png" width="50%"> 

### **■ Solder a USB C socket to the back of the board here** 

This is technically optional, but you'll need to solder an additional jumper somewhere here if you decide to skip.  
<img src="https://raw.githubusercontent.com/apiel/zicBox/refs/heads/main/hardware/ZicXY/images/image33.png" width="60%"> 

### **■ Be sure to clip this encoder's pins, and left tab so they don't poke out on the back of the board**

This is to allow the header in the next step to sit flush against the PCB.  This is an instance where the flush cutters are helpful.  
<br>
<p align="center"><img src="https://raw.githubusercontent.com/apiel/zicBox/refs/heads/main/hardware/ZicXY/images/image27.png" width="60%"> 
<br><br>
<img src="https://raw.githubusercontent.com/apiel/zicBox/refs/heads/main/hardware/ZicXY/images/image11.png" width="60%">   
<br>
*Clip these so they don't stick out*
<br>
<span style="font-size: 500%">🠿</span> 
<br>
<img src="https://raw.githubusercontent.com/apiel/zicBox/refs/heads/main/hardware/ZicXY/images/image25.png" width="60%"> 
</p>

### **■ Solder a  90° 1x6 female pin header here on the back of the board for the DAC**

<img src="https://raw.githubusercontent.com/apiel/zicBox/refs/heads/main/hardware/ZicXY/images/image31.png" width="60%"> 

*If you don't have a 1x6 female header ../../../hardware/ZicXY/assemblyhandy, You can cut a longer header to size.* 

### **■ Solder a 1x6 male pin header to the *top* of the DAC.**

<img src="https://raw.githubusercontent.com/apiel/zicBox/refs/heads/main/hardware/ZicXY/images/image9.png" width="60%"> 

### **■ Plug DAC into the female slot from previous step**

<img src="https://raw.githubusercontent.com/apiel/zicBox/refs/heads/main/hardware/ZicXY/images/image14.png" width="60%"> 

### **■ (optional) Solder slide switch here**

This is only recommended if you are adding your own battery power.  
You will need to bend the pins 90 degrees to make the switch through its slot in the 3D printed case if using.

<img src="https://raw.githubusercontent.com/apiel/zicBox/refs/heads/main/hardware/ZicXY/images/image7.png" width="60%">  
<img src="https://raw.githubusercontent.com/apiel/zicBox/refs/heads/main/hardware/ZicXY/images/image15.png" width="60%">  
<br> *The switch's opening on the 3D printed case*

### **■ If you don't solder a slide switch, solder a jumper between these 2 holes**

!<img src="https://raw.githubusercontent.com/apiel/zicBox/refs/heads/main/hardware/ZicXY/images/image34.png" width="45%"> <span style="font-size: 300%">➧</span> <img src="https://raw.githubusercontent.com/apiel/zicBox/refs/heads/main/hardware/ZicXY/images/image26.png" width="40%"> 

### **■ Attach the last 3 keys, and solder the encoders in place**

<img src="https://raw.githubusercontent.com/apiel/zicBox/refs/heads/main/hardware/ZicXY/images/image13.png" width="60%"> 
The keys should pretty much work without being soldered, but the encoders will likely not work well until they have been soldered.

### **■ Done\! (Hopefully)**

**Troubleshooting:**  
Not booting up?

* Have you soldered all the necessary jumpers?  
* Does your RPI have a valid SD card?   
  (Even plugged into, LEDs might not light and it will look totally dead if not)

Keys/encoders not working?

* Check for broken traces on your board (this happened to the author)

No Audio?

* Double check that you've [soldered the jumpers correctly on the DAC](#bookmark=id.mpi7b1ljqzok)
