#!/usr/bin/env bash
# SPDX-License-Identifier: MIT
set -euo pipefail
cd -- "$(dirname -- "$0")"
root=$(pwd -P)
out="$root/build"
export ARCH=arm64 CROSS_COMPILE="${CROSS_COMPILE-aarch64-linux-gnu-}"
export KBUILD_BUILD_USER=czlite KBUILD_BUILD_HOST=build
export LOCALVERSION=
jobs="${JOBS:-$(nproc)}"
mkdir -p "$out"
if [[ ! -f "$out/.config" ]]; then cp czlite.config "$out/.config"; fi
kernel=(make -C "$root/linux" O="$out" -j"$jobs")
"${kernel[@]}" olddefconfig
"${kernel[@]}" Image modules broadcom/bcm2710-rpi-cm0.dtb \
    overlays/{dwc2,vc4-kms-v3d,imx219,gpio-ir,gpio-ir-tx}.dtbo
modules="$root/bsp/modules"
for dir in CardputerZero tca8418-1.0 es8389-1.0 bq27220-1.0 imu-1.0/{bmi270,magnetometer}; do
    "${kernel[@]}" M="$modules/$dir" MO="$out/bsp/$dir" \
        CONFIG_CARDPUTERO_V0_5=y CONFIG_BMC150_MAGN=m CONFIG_BMC150_MAGN_BMI270=m \
        CONFIG_BMI270=m CONFIG_BMI270_I2C=m KCFLAGS=-DCONFIG_CARDPUTERO_V0_5=1 modules
done
mkdir -p "$out/overlays"
for overlay in CardputerZero/cardputerzero-v5-overlay bq27220-1.0/bq27220_v5 imu-1.0/bmi270_bmm150_overlay; do
    cpp -E -nostdinc -undef -x assembler-with-cpp -DCONFIG_CARDPUTERO_V0_5=1 "$modules/$overlay.dts" |
        "$out/scripts/dtc/dtc" -@ -I dts -O dtb -o "$out/overlays/${overlay##*/}.dtbo"
done
