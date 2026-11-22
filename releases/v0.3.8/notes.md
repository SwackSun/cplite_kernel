## czlite v0.3.8 for Cardputer Zero Standard V5

- The built-in console opens fff with Fn locked for navigation. Opening an application, editor or shell releases Fn. Returning to fff restores the lock. A single Fn tap still unlocks the keyboard manually.
- The preinstalled Wi-Fi utility and CPZ Store use the normal typing layer. CPZ and opkg share the same default package configuration and installation records.
- The public image includes component license notices under `/usr/share/licenses/czlite/`. The kernel and video utilities use public build paths and identifiers. The fff and keyboard module bytes match the versions tested on the device.

Download `cardputerzero-v0.3.8-sdcard.img.xz` and `SHA256SUMS`. Run `sha256sum --ignore-missing -c SHA256SUMS` in the download directory. Select the compressed image as a custom OS in Raspberry Pi Imager. Writing the image erases the selected microSD card.

The compressed image SHA-256 is `7d3371227e5643950890f43ffdef162ca4e1e6f29656cbaf5da31ed2d0cbe359`. The uncompressed image SHA-256 is `f0d711d1ca1121b2b2d645cb81479713afd37266f9a6fce30f0cecadbd9ae506`. The archive, partitions, filesystems, native boot checksums and keyboard components were checked offline. The final public card image has not undergone a separate physical boot test.

The tag contains the kernel and board driver source. The `czlite-v0.3.8-sources.tar.gz` asset includes userspace package sources and patches, board recipes, configuration files, the Buildroot snapshot and license notices. Individual components retain their respective terms. [Getting started](https://firmiss.com/page/911646860714704896) · [Firmiss firmware mirror](https://firmiss.com/page/911603036453863424)
