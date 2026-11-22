# czlite v0.3.8 for Cardputer Zero

This tree builds the AArch64 Linux 6.18.59-cm0-early kernel for the Cardputer Zero Standard v5 native-display firmware. The kernel and BSP sources include the CZLite patches for display handoff, HDMI hotplug, the IO expander, battery gauge, audio, power control and IMU initialization.

## Firmware and changes since v0.3.7

The v0.3.8 microSD image includes fff. Logging in on the built-in console locks Fn for arrow-key navigation. Launching an application, editor or shell releases Fn, and returning to fff locks it again. The TCA8418 module in this tag exposes the Fn lock used by fff and tmux. The opkg command and CPZ now share the package database and feed configuration.

Download the compressed image and its checksum, then select the image in Raspberry Pi Imager. Verify it with `sha256sum -c SHA256SUMS` before writing the card. [GitHub release](https://github.com/SwackSun/cplite_kernel/releases/tag/v0.3.8) · [Firmiss firmware](https://firmiss.com/page/911603036453863424) · [Getting started](https://firmiss.com/page/911646860714704896)

## Source and licenses

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

The kernel Image is written to build/arch/arm64/boot/. The Raspberry Pi device trees are under build/arch/arm64/boot/dts/, external board modules under build/bsp/, and M5 overlays under build/overlays/. Runtime peripheral firmware such as bmi270-init-data.fw is supplied by the v0.3.8 device filesystem.

czlite.config comes from the shipped configs.ko. The host-specific compiler description is omitted and Kconfig fills it from the selected compiler. The readable panel initialization commands are in panel/cardputerzero.splash. After editing them, run `python3 panel/generate.py` before rebuilding.
