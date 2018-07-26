# Ramdisk for ARTIK5 and ARTIK10
## Contents
1. [Introduction](#1-introduction)
2. [License](#2-license)
3. [Build guide](#3-build-guide)
4. [Update guide](#4-update-guide)

## 1. Introduction
This 'initrd-artik' repository is ramdisk source for artik5(artik520) and
artik10(artik1020). The initrd of artik will do recovery the eMMC partitions
from sdcard fusing image.

---
## 2. License
This 'initrd-artik' includes below source files:
+ busybox: GPLv2
+ e2fsprogs: GPLv2
+ pv: ARTISTIC 2.0

---
## 3. Build guide
### 3.1 Install u-boot-tools
The 'mkimage' will be required to generate 'uInitrd' image.
```
sudo apt-get install u-boot-tools
```

### 3.2 Build the initrd
```
./build.sh
```

The 'uInitrd' file will be generated into output/uInitrd

---
## 4. Update Guide
Copy the 'uInitrd' into your board.

```
scp output/uInitrd root@{YOUR_BOARD_IP}:/boot
```

+ On your board
```
sync
reboot
```
