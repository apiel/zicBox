## TODO

- TODO have a look to libgpiod

```sh
# Update GPIO backend to support Orange Pi with libgpiod

Please update the GPIO and build system to support Orange Pi while preserving the existing Raspberry Pi implementation.

## 1. `helpers/gpio.h`

Currently the file has:

- A default implementation using direct Raspberry Pi GPIO register access via `/dev/gpiomem` and `mmap()`.
- An optional `PIGPIO` implementation using `pigpio.h`.

Change this so that:

- Remove the `PIGPIO` backend completely.
- Keep the existing direct `/dev/gpiomem` GPIO implementation as the default.
- Add a new `libgpiod` implementation selected with `-DGPIO_GPIOD=1`.

Conceptually:

\`\`\`cpp
#ifdef GPIO_GPIOD
    // libgpiod implementation
#else
    // existing direct Raspberry Pi MMIO implementation
#endif
\`\`\`

The `libgpiod` backend should provide the same GPIO API currently used by the application (`initGpio`, `gpioSetMode`, `gpioWrite`, `gpioRead`, `gpioSetPullUp`, etc.) so callers do not need to change.

Use the libgpiod API version that is actually available in this project's Buildroot environment. Do not mix v1/v2 APIs.

Keep GPIO numbering/mapping in mind: Raspberry Pi BCM GPIO numbers may not correspond directly to Orange Pi GPIO numbers. Reuse any existing board-specific mapping if the repository already has one.

## 2. `make_common.mk`

Add an optional board/platform parameter without breaking existing builds.

I would like to be able to build an Orange Pi target with something like:

\`\`\`bash
make build_zic cc=arm64 platform=orange_pi
\`\`\`

When `platform=orange_pi` is specified:

- Add `-DGPIO_GPIOD=1`.
- Add the required `libgpiod` compiler/linker flags.
- Use the Buildroot sysroot/toolchain already configured for ARM64.

When `platform` is not specified, preserve the existing behavior.

In particular:

\`\`\`bash
make build_zic
\`\`\`

and:

\`\`\`bash
make build_zic cc=arm64
\`\`\`

must continue to use the existing direct Raspberry Pi GPIO implementation and must **not** define `GPIO_GPIOD`.

Keep CPU architecture (`TARGET_PLATFORM` / `cc=arm64`) separate from board selection (`platform=orange_pi`).

## 3. Buildroot

Check the existing Buildroot configuration and enable `libgpiod` if it is not already enabled.

Make sure the library and headers are available through the existing Buildroot staging/sysroot setup when cross-compiling.

Do not break the existing Raspberry Pi Buildroot image or release workflow.

## 4. Do not modify every subproject

`make_common.mk` is shared by all subprojects.

Prefer implementing the platform selection and GPIO dependency centrally in `make_common.mk` so individual subproject Makefiles do not need to be modified.

Only modify a subproject Makefile if it is genuinely necessary.

## 5. Validation

Verify as much as possible:

\`\`\`bash
make build_zic
make build_zic cc=arm64
make build_zic cc=arm64 platform=orange_pi
\`\`\`

Confirm that:

- Default builds use direct MMIO GPIO.
- Raspberry Pi ARM64 builds use direct MMIO GPIO.
- Orange Pi builds define `GPIO_GPIOD=1`.
- Orange Pi builds link against `libgpiod`.
- `pigpio.h` is no longer used.
- Existing Raspberry Pi build/release behavior remains unchanged.

If an Orange Pi-specific Buildroot toolchain or GPIO mapping does not currently exist, do not invent one. Implement the generic support that can be added cleanly and clearly report what remains to be configured.
```

## debug

```sh
gdb zicBox
```

then `run`


then `bt`.

# i2s expender

MCP23017

# bare metal or rtos

Try to build a baremetal synth https://github.com/rsta2/circle/tree/master/sample/29-miniorgan
                               https://github.com/rsta2/circle/blob/master/sample/34-sounddevices/oscillator.cpp
                               https://github.com/rsta2/minisynth

or to build something on freertos


| PCM5102 Board | WeActStudio PCM5100A Board                   |
| ------------- | -------------------------------------------- |
| VIN           | VCC                                          |
| GND           | GND                                          |
| LRCK          | WS                                           |
| DIN           | DIN                                          |
| BCK           | BCK                                          |
| SCK           | MC                                           |
| *(none)*      | SD (tie HIGH or check docs for enable logic) |
