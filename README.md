# CZLite v0.3.7 kernel source

This tree builds the AArch64 Linux 6.18.59-cm0-early kernel for the Cardputer Zero Standard v5 native-display firmware. The kernel and BSP sources include the CZLite patches for display handoff, HDMI hotplug, the IO expander, battery gauge, audio, power control and IMU initialization.

| Source | Upstream revision |
| --- | --- |
| Raspberry Pi Linux | raspberrypi/linux · b3495ea401d709af0701a85babcba5e3d788a602 |
| M5Stack board drivers and overlays | m5stack/m5stack-linux-dtoverlays · 4f7aeee9fc103ebb03d22cac476bb191ecfb321e |

The 93-byte panel initialization data follows the [M5Stack BSP license notice](bsp/COPYING). See [COPYING](COPYING) for its source revision and license scope. The standalone CZLite build script, panel generator and this guide use the [MIT License](LICENSE-MIT).

## Build

On Linux, install make, GCC and binutils for AArch64, native GCC and cpp, flex, bison, bc, libssl development headers and libelf development headers. The release used GCC 15.3 and binutils 2.45.1. Set CROSS_COMPILE to your toolchain prefix or use the default aarch64-linux-gnu-.

```sh
JOBS=8 ./build.sh
```

The kernel Image is written to build/arch/arm64/boot/. The Raspberry Pi device trees are under build/arch/arm64/boot/dts/, external board modules under build/bsp/, and M5 overlays under build/overlays/. Runtime peripheral firmware such as bmi270-init-data.fw is supplied by the v0.3.7 device filesystem.

czlite.config comes from the shipped configs.ko. The host-specific compiler description is omitted and Kconfig fills it from the selected compiler. The readable panel initialization commands are in panel/cardputerzero.splash. After editing them, run `python3 panel/generate.py` before rebuilding.
