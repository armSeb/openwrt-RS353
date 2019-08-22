This repository provides OpenWrt support for the followig devices:

Bintec RS230 (partial support)
Bintec RS353 (full support)


Build:

For RS230:

Target System (Lantiq)
Subtarget (XWAY)
Target Profile (Bintec RS230)

For RS353: 
Target System (Lantiq)
Subtarget (XRX200)
Target Profile (Bintec RS353)

Then, select packages and type make.

You will obtain a directly flashable image that you can install using the original firmware web interface. This image is also flashable with bootmon and sysupgrade.

