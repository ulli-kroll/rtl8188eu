rtl8188eu linux
======================

rtl8188fu linux driver for wireless bgn device

This is the same driver you can found in drivers/staging  
The purpose of this is, to get used and convert to driver

build/load/function tested with v4.19  

Building and install driver
---------------------------

for building type  
`make`  

for load the driver  
`sudo modprobe cfg80211`  
`sudo modprobe lib80211`  
`sudo insmod rtl8188eu.ko`  

The needed firmware blog is part of linux-firmware

If you need to crosscompile use  
`ARCH= CROSS_COMPILE= KSRC=`  
while calling `make` i.e.  

`make ARCH="arm" CROSS_COMPILE=armv5tel-softfloat-linux-gnueabi- KSRC=/home/linux-master modules`  

Please use prefix **rtl8188eu** if you want to mail me  
But please please don't, I have enough to do.  
TIA  

Hans Ulli Kroll <ulli.kroll@googlemail.com>
