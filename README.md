This repository provides OpenWrt support for the followig devices:

Bintec RS230 (partial support)
Bintec RS353 (full support)

This repository contains two branchs:

- old_2021: Old version based on the OpenWrt git master
- openwrt-22.03: WIP based on the OpenWrt git 22.03 branch

To get the right openWRT version, do the following steps:

git clone https://github.com/armSeb/openwrt-RS353.git
git checkout old_2021


To Build:

make menuconfig

Then, select the right platform

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

