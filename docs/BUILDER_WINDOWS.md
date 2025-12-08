# 31 WSL Development Environment

This guide covers setting up a Windows + WSL2 development environment for ZicBox, enabling:
- Local x86 testing with audio via WSLg
- ARM64 cross-compilation for Raspberry Pi Zero 2W
- Fast iteration without deploying to hardware

## Prerequisites

- Windows 10/11 with WSL2 (Ubuntu 22.04+ recommended)
- WSLg enabled (automatic in recent Windows 11 / WSL updates)
- Node.js 18+ (for TypeScript config generation)

## WSL2 Setup

### 1. Install Required Packages

```bash
# Core build tools
sudo apt update
sudo apt install -y build-essential git

# Audio libraries (x86)
sudo apt install -y libasound2-dev libsndfile1-dev

# Graphics (for local SFML UI)
sudo apt install -y libsfml-dev

# Audio utilities (for WSLg audio routing)
sudo apt install -y pulseaudio-utils alsa-utils libasound2-plugins
```

### 2. Configure ALSA → PulseAudio Bridge (WSLg Audio)

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

## Building & Running Locally (x86)

### Build Configuration

```bash
npm install           # Install TypeScript dependencies
npm run build:pixel   # Generate config.json from TypeScript
```

### Build x86 Binary + Plugins

```bash
make                  # Builds ./build/x86/zic and plugins
```

### Run Locally

```bash
./build/x86/zic       # Launches SFML window with audio
```

**Expected warnings (safe to ignore in WSL):**
- `can't open /dev/gpiomem` – No GPIO in WSL
- `Failed to open wpa_supplicant.conf` – No WiFi config needed
- Missing clip files – Normal for fresh start

## ARM64 Cross-Compilation (for Raspberry Pi)

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

## Plugin Development Workflow

### Fast Iteration Cycle

1. **Edit plugin** (e.g., `plugins/audio/MultiEngine/FmEngine.h`)
2. **Test locally** (x86 build, no audio hardware needed for parameter testing)
3. **Cross-compile** for ARM64
4. **Deploy** single `.so` file to Pi
5. **Restart** zicBox service on Pi

### Build Single Plugin (x86)

```bash
g++ -DPLUGIN_INCLUDE=\"SynthMultiEngine.h\" \
  -shared -fPIC -O3 \
  -I. -Ilibs \
  plugins/audio/SynthMultiEngine.cpp \
  -o build/x86/libs/audio/libzic_SynthMultiEngine.so \
  -lsndfile
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

## Project Structure Reference

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
