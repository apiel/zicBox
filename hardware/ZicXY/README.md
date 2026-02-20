# 20 Zic XY

<img src='https://github.com/apiel/zicBox/blob/main/hardware/ZicXY/Zic_XY_black.png?raw=true' width='350'> <img src='https://github.com/apiel/zicBox/blob/main/hardware/ZicXY/Zic_XY_blue.png?raw=true' width='350'>

The **Zic XY** is a compact digital instrument with 8 versatile tracks. Each track can load any engine — drums, synths, or samples — all handled by a single multi-engine system.  

## 🔸 Track Layout

| Track Type | Count | Description |
|------------|-------|-------------|
| Multi Engine Tracks | 8 | Each track can load any engine: drum, synth, or sample. Independent parameters for envelope, mixer, and FX routing. |

You can freely mix engines across tracks — e.g., Kick Engine on Track 1, Grain Engine on Track 4, SuperSaw Engine on Track 8.

## 🥁 Engines

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

## 🔧 Build & Installation

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

## Assembly

[Read instructions](Assembly)

## OS Installation Guide

This guide explains how to install the **custom Zic XY OS** on your hardware.

### 1️⃣ Download the OS

Download the latest release here:  
[Zic OS XY Release](https://github.com/apiel/zicBox/releases/tag/zicOsXY)  

The file to download is: `zicOsXY.zip`

### 2️⃣ Prepare the SD Card

1. Uncompress the downloaded zip file. Inside you will find the `.img` file.  
2. Burn the image to an SD card.

#### Linux Example: Using Raspberry Pi Imager
Install if needed: `sudo apt install rpi-imager`  
Run the imager: `rpi-imager`  
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

