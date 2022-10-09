## Introduction to selected z88dk Libraries

Summary of the file system management libraries.

```
FatFs --> diskio_sd    - use to access the CSIO SD Card interface on SC126, SC130, and SC131 directly.
      \-> diskio_hbios - use to access any logical disk interface supported by RomWBW HBIOS (use the logical drive number).
      \-> diskio integrated in z88dk - 82C55 IDE drivers for RC2014 IDE Hard Drive Module and YAZ180 integrated PPIDE.
```

### diskio_hbios
FatFs is a generic FAT/exFAT filesystem module for small embedded systems. The FatFs module is written in compliance with ANSI C (C89) and completely separated from the disk I/O layer. This is the disk I/O layer for hbios, using hbios calls as implemented in ROMWBW.

### diskio_sd
FatFs is a generic FAT/exFAT filesystem module for small embedded systems. The FatFs module is written in compliance with ANSI C (C89) and completely separated from the disk I/O layer. This is the disk I/O layer for the SC126, SC130, and SC131, using the SD interface as implemented in z88dk for the Z180 CSIO.

### FatFs
FatFs is a generic FAT/exFAT filesystem module for small embedded systems. The FatFs module is written in compliance with ANSI C (C89) and completely separated from the disk I/O layer. Therefore it is independent of the platform. It can be incorporated into small microcontrollers with limited resource, such as 8051, PIC, AVR, ARM, Z80, RX and etc.

### FreeRTOS
Developed in partnership with the world’s leading chip companies over a 15 year period, FreeRTOS is a market-leading real–time operating system (RTOS) for microcontrollers and small microprocessors. Distributed freely under the MIT open source license, FreeRTOS is built with an emphasis on reliability, accessibility, and ease of use.

### ft80x
This library is build for FT801 EVE display, using I2C. Hardware tested is Riverdi RVT4.3" platform. Please do not use this library on any other devices, that could cause unpredictable damage to an unknown device.

### i2c_lcd
This library is build for I2C_LCD12864 from Seeed Studio - Sparking. Please do not use this library on any other devices, that could cause unpredictable damage to an unknown device.

### 3D
3D provides functions that allow you to simply and efficiently manage 3D Vectors and Vector Transformations. Including vector and matrix multiplication, identity matrices, and homogeneous coordinate transformations.

### ReGIS
ReGIS interprets commands that allow you to simply and efficiently control a video monitor screen and draw pictures on the screen with lines, curves, and circles. Also, ReGIS provides commands to include text characters in pictures. The ReGIS graphics language is designed for conciseness and easy transport of code from the host to the ReGIS device.

### th02
This is a multi-functional sensor that gives you temperature and relative humidity information at the same time. It utilizes a TH02 sensor that can meet measurement needs of general purposes. It provides reliable readings when environment humidity condition in between 0-80% RH, and temperature condition in between 0-70°C, covering needs in most home and daily applications that don't contain extreme conditions.

### time
The implementation of `time.h` aspires to conform with ISO/IEC 9899 (C90). However, due to limitations of the target processor and the nature of its development environment, a practical implementation must of necessity deviate from the standard.

## Installation

The `z88dk-lib` function is used to install for the desired target and desired library. e.g. for the time library on the yaz180 machine.

```
cd ~/z88dk-libraries
z88dk-lib +yaz180 time
```

Some further examples of `z88dk-lib` usage.

+ libraries list help
```
z88dk-lib
```
+ list 3rd party libraries already installed for the zx target
```
z88dk-lib +zx
```
+ remove the `libname1` `libname2` ... libraries from the zx target, -f for no nagging about deleting files.
```
z88dk-lib +zx -r -f libname1 libname2 ...
```

## Preparation
The libraries can be compiled using the following command lines in Linux, with the `+target` modified to be relevant to your machine, from the relevant library sub-directory. e.g. for any `library`.

```
cd ~/library/source
zcc +target -clib=new -x -O2 --opt-code-speed=add32,sub32,sub16,inlineints --math32 @library.lst -o ../library
mv ../library.lib ../target/lib/newlib/sccz80
zcc +target -clib=sdcc_ix -x -SO3 --max-allocs-per-node400000 --math32 @library.lst -o ../library
mv ../library.lib ../target/lib/newlib/sdcc_ix
zcc +target -clib=sdcc_iy -x -SO3 --max-allocs-per-node400000 --math32 @library.lst -o ../library
mv ../library.lib ../target/lib/newlib/sdcc_iy
```
The resulting `library.lib` file should be moved to `~/library/target/lib/newlib/sdcc_ix` or `~/library/target/lib/newlib/sdcc_iy` respectively, as noted above.

## Usage
Once installed, the libraries can be linked against on the compile line by adding `-llib/target/library` and the include file can be found with `#include <lib/target/library.h>`.
