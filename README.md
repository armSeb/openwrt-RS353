This repository provides OpenWRT support for the followig devices:

 - Bintec RS230 - partial support - only in the old_2021 branch 
 - Bintec RS353 - full support

This repository contains two branchs:

 - old_2021: Old version based on the OpenWrt git master
 - openwrt-22.03: WIP based on the OpenWrt git 22.03 branch

For instance, if you wish to build the openwrt-22.03 for the RS353 target

    git clone https://github.com/armSeb/openwrt-RS353.git
    git checkout openwrt-22.03
    make menuconfig
  
  Then, select the right platform

For RS230 (for now only available in the old_2021 branch):

> Target System - Lantiq
> Subtarget - XWAY 
> Target Profile - Bintec RS230

For RS353: 

> Target System - Lantiq
> Subtarget - XRX200
> Target Profile - Bintec RS353

Then, select packages and type make.

You will obtain a directly flashable image that you can install using the original firmware web interface. This image is also flashable with bootmon via tftp and the OpenWRT sysupgrade.
