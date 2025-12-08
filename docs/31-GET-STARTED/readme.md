# 31 Get Started on Ubuntu and WSL

To get started 🚀, it’s best to begin by exploring an **existing project** — for example, the **Pixel** build.  
This will help you understand how zicBox projects are structured and how the configuration system works.

This guide covers setting up a development environment on **Ubuntu** or ** Windows + WSL2** for ZicBox, enabling:
- Local x86 testing
- ARM64 cross-compilation for Raspberry Pi Zero 2W

## Prerequisites

- **Ubuntu** 
- or **Windows 10/11 with WSL2** (Ubuntu 22.04+)
  - WSLg enabled (automatic in recent Windows 11 / WSL updates)

### 1. Setup

Before building or running the application, install the required development packages:

```bash
# Core build tools
sudo apt update
sudo apt install -y build-essential git

# Audio libraries (x86)
sudo apt install -y libasound2-dev libsndfile1-dev

# Graphics (for local SFML UI)
sudo apt install -y libsfml-dev
```

On WSL, you need to install as well the following packages for audio routing:
```sh
# Audio utilities (for WSLg audio routing)
sudo apt install -y pulseaudio-utils alsa-utils libasound2-plugins
```

We use TypeScript to generate the config file.
Make sure you have **Node.js** installed on your system: 

```sh
sudo apt-get install npm
```

After cloning the repository for the first time, install the dependencies:

```sh
npm install
```


### 2. WSL ONLY: Configure ALSA → PulseAudio Bridge

> [!NOTE]  
> This is only for Windows WSL

WSLg provides PulseAudio at `/mnt/wslg/PulseServer`. Configure ALSA to route through it:

```bash
# Create ~/.asoundrc
cat > ~/.asoundrc << 'EOF'
pcm.!default { type pulse }
ctl.!default { type pulse }
EOF
```

Verify audio works:
```bash
pactl info  # Should show "Server Name: pulseaudio"
```

### 3. Create Missing Data Folders

```bash
cd /mnt/e/zicBox  # or your workspace path
mkdir -p data/audio/er1
mkdir -p data/workspaces/pixel/default
```

### 4. Build the Configuration

To generate the configuration files for the Pixel project, run:

```sh
npm run build:pixel
```

Or, if you want **hot reloading** while editing:

```sh
npm run dev:pixel
```

### 5. Run the Application

Once the configuration is built, launch the application (in another terminal if you’re using dev mode):

```sh
make
```

**Expected warnings (safe to ignore in WSL):**
- `can't open /dev/gpiomem` – No GPIO in WSL
- `Failed to open wpa_supplicant.conf` – No WiFi config needed
- Missing clip files – Normal for fresh start

The **Zic Pixel UI** should now be visible.  
You can interact with it using your **keyboard and mouse** to simulate buttons and encoders:

```
Q W E R
A S D F
Z X C V
```

Use your **mouse wheel** and scroll over an encoder value on the screen to simulate turning an encoder.

### 6. Edit and Reload

Try editing something to see how changes are reflected in real time.

Open the file:

```
config/pixel/ui/constants.ts
```

Change the color value of `ColorTrack1`, save the file, and if you’re running in **dev mode**, it should **automatically reload** — showing the updated color instantly.

## Project Structure Reference


Now that you’ve seen how things update, here’s what’s happening:

- The **TypeScript files** in `config/pixel/` are used to **generate JSON configuration files**.  
- These JSON files are then **loaded by zicBox** to configure the **UI** and **audio tracks**.

For example:  
- The audio setup is defined in `config/pixel/audio/index.ts`  
- The UI layout is defined in `config/pixel/ui/`  

The UI code looks similar to HTML, but it’s **not web-based** — there’s **no HTML, CSS, or DOM events**.  
It’s a purely static layout definition that zicBox uses to build the interface views and place components.

```
zicBox/
├── build/
│   ├── x86/              # Desktop build output
│   │   ├── zic           # Main executable
│   │   └── libs/audio/   # x86 plugin .so files
│   └── pixel_arm64/      # ARM64 build output
│       └── libs/audio/   # ARM64 plugin .so files
├── plugins/
│   └── audio/
│       ├── SynthMultiEngine.cpp/.h
│       └── MultiEngine/
│           ├── FmEngine.h
│           ├── AdditiveEngine.h
│           └── ...
├── config/
│   └── pixel/            # TypeScript config source
├── config.json           # Generated UI/audio config
└── data/
    ├── audio/            # Sample files, wavetables
    └── workspaces/       # Saved presets
```

## ARM64 Cross-Compilation (for Raspberry Pi)

> [!NOTE]  
> Still to be tested. Right now, I am building the project using buildroot zicOs, need to write doc about this as well.

### 1. Install ARM64 Cross-Compiler

```bash
sudo apt install -y g++-aarch64-linux-gnu
```

### 2. Add ARM64 Package Architecture

```bash
sudo dpkg --add-architecture arm64

# Add Ubuntu ports repository for ARM64 libs
sudo tee /etc/apt/sources.list.d/arm64-ports.list << 'EOF'
deb [arch=arm64] http://ports.ubuntu.com/ubuntu-ports noble main restricted universe multiverse
deb [arch=arm64] http://ports.ubuntu.com/ubuntu-ports noble-updates main restricted universe multiverse
EOF

sudo apt update
```

### 3. Install ARM64 Libraries

```bash
sudo apt install -y libsndfile1-dev:arm64 libasound2-dev:arm64
```

### 4. Build for ARM64

```bash
# Build a single plugin
aarch64-linux-gnu-g++ \
  -DPLUGIN_INCLUDE=\"SynthMultiEngine.h\" \
  -shared -fPIC -O3 \
  -I. -Ilibs \
  -L/usr/lib/aarch64-linux-gnu \
  plugins/audio/SynthMultiEngine.cpp \
  -o build/pixel_arm64/libs/audio/libzic_SynthMultiEngine.so \
  -lsndfile

# Or use makefile (requires zicOs toolchain for full build)
make buildPi
```

## Troubleshooting

### No Audio in WSL

1. Check PulseAudio server: `pactl info`
2. Verify `.asoundrc` exists with pulse config
3. Install `libasound2-plugins` for ALSA-Pulse bridge

### ALSA "Unknown PCM default" Error

The `.asoundrc` file is missing or incorrect. Recreate it:
```bash
echo 'pcm.!default { type pulse }' > ~/.asoundrc
echo 'ctl.!default { type pulse }' >> ~/.asoundrc
```

### ARM64 Library Not Found

Ensure ports.ubuntu.com is configured for arm64 architecture:
```bash
cat /etc/apt/sources.list.d/arm64-ports.list
```

### "undefined symbol" on Pi

Missing library in link step. Add required `-l` flags:
```bash
-lsndfile   # For sf_open, sf_read, etc.
-lasound    # For ALSA functions
```