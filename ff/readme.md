## Introduction to the Z80 FatFS functions

FatFs is a generic FAT/exFAT filesystem module for small embedded systems. The FatFs module is written in compliance with ANSI C (C89) and completely separated from the disk I/O layer. Therefore it is independent of the platform. It can be incorporated into small microcontrollers with limited resource, such as 8051, PIC, AVR, ARM, Z80, RX and etc.

Current source version status is: 0.15, November 11, 2022.<br>

Compiled with sccz80 version 22168-da9bcc8cc0-20240130, and using zsdcc version 4.4.0 [r14648](https://sourceforge.net/p/sdcc/code/14648/log/?path=/trunk/sdcc).

#### Features

<ul>
 <li>DOS/Windows compatible FAT/exFAT filesystem.</li>
 <li>Platform independent. Easy to port.</li>
 <li>Very small footprint for program code and work area.</li>
 <li>Various configuration options to support for:
  <ul>
   <li>Multiple volumes (physical drives and partitions).</li>
   <li>Multiple ANSI/OEM code pages including DBCS.</li>
   <li>Long file name in ANSI/OEM or Unicode.</li>
   <li>exFAT filesystem.</li>
   <li>RTOS environment.</li>
   <li>Fixed or variable sector size.</li>
   <li>Read-only, optional API, I/O buffer and etc...</li>
  </ul>
 </li>
</ul>

## Installation

The `z88dk-lib` function is used to install for the desired target. e.g.

```bash
cd ..
z88dk-lib +target -f ff
```

Some further examples of `z88dk-lib` usage.
list help
```bash
z88dk-lib
```
list 3rd party libraries already installed for the zx target
```bash
z88dk-lib +zx
```
remove the `libname1` `libname2` ... libraries from the zx target, -f for no nagging about deleting files.
```bash
z88dk-lib +zx -r -f libname1 libname2 ...
```

## Usage

Once installed, the FatFs library can be linked against on the compile line by adding `-llib/target/ff` and the include file can be found with `#include <lib/target/ff.h>`.

A simple usage example, for the `+yaz180`, `+scz180`, or `+rc2014` targets.

```c
/*----------------------------------------------------------------------*/
/* Foolproof FatFs sample project for Z80              (C)ChaN, 2014    */
/*----------------------------------------------------------------------*/
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "ffconf.h"            /* The ffconf.h file from library compilation */

#if __RC2014
#include <lib/rc2014/ff.h>     /* Declarations of FatFs API */

#elif __SCZ180
#include <lib/scz180/ff.h>     /* Declarations of FatFs API */

#elif __YAZ180
#include <lib/yaz180/ff.h>     /* Declarations of FatFs API */

#endif

// zcc +yaz180 -subtype=app -clib=sdcc_iy -v --list -m -SO3 -llib/yaz180/ff --max-allocs-per-node200000 ff_main.c -o ff_main -create-app

// zcc +scz180 -subtype=app -clib=sdcc_iy -v --list -m -SO3 -llib/scz180/ff --max-allocs-per-node200000 ff_main.c -o ff_main -create-app

// zcc +rc2014 -subtype=app -clib=sdcc_iy -v --list -m -SO3 -llib/rc2014/ff --max-allocs-per-node200000 ff_main.c -o ff_main -create-app


static FATFS FatFs;        /* FatFs work area needed for each volume */
static FIL Fil;            /* File object needed for each open file */

void main (void)
{
    UINT bw;

    f_mount(&FatFs, "0:", 0);                       /* Give a work area to the default drive */
//  f_mount(&FatFs, "3:", 0);                       /* Give a work area to the HBIOS SD0: drive */

    if (f_open(&Fil, "newfile.txt", FA_WRITE | FA_CREATE_ALWAYS) == FR_OK)
    {    /* Create a file */

        f_write(&Fil, "It works!\r\n", 11, &bw);    /* Write data to the file */

        f_close(&Fil);                              /* Close the file */

        printf("It works!\r\n");
    }
}

```

## Preparation

Please check that your diskio layer is working correctly, using the example program in `examples/diskio_check.c`. This will destroy the contents of the drive, so back up any data prior to using it.

Then configure the library to suit your requirements by adjusting the `source/ffconf.h` file to provide the functions you need. You will then use this `ffconf.h` file to provide the options you included. This is a current limitation of z88dk, whereby it can only provide one third party library header file.

The ff library can be compiled from the `ff/source` directory using the following command lines in Linux, with the `+target` modified to be relevant to your machine.

```
zcc +rc2014 -clib=new -x -O2 --opt-code-speed=all --math32 @ff.lst -o ../ff
zcc +rc2014 -clib=sdcc_ix -x -SO3 --max-allocs-per-node400000 @ff.lst -o ../ff
zcc +rc2014 -clib=sdcc_iy -x -SO3 --max-allocs-per-node400000 @ff.lst -o ../ff

zcc +rc2014 -clib=new -m8085 -x -O2 --opt-code-speed=all -D__DISABLE_BUILTIN --math32 @ff.lst -o ../ff_85
```
```
zcc +yaz180 -clib=new -x -O2 --opt-code-speed=all --math32 @ff.lst -o ../ff
zcc +yaz180 -clib=sdcc_ix -x -SO3 --max-allocs-per-node400000 @ff.lst -o ../ff
zcc +yaz180 -clib=sdcc_iy -x -SO3 --max-allocs-per-node400000 @ff.lst -o ../ff
```
For any supported target.
```
zcc +target -clib=new -x -O2 --opt-code-speed=all --math32 @ff.lst -o ../ff
zcc +target -clib=sdcc_ix -x -SO3 --max-allocs-per-node400000 @ff.lst -o ../ff
zcc +target -clib=sdcc_iy -x -SO3 --max-allocs-per-node400000 @ff.lst -o ../ff
```
The resulting `ff.lib` file should be moved to `~/target/lib/newlib/sccz80` or `~/target/lib/newlib/sdcc_ix` or `~/target/lib/newlib/sdcc_iy` respectively.

## Documentation

ChaN's documentation is copied verbatim here, for immediate reference.
Please use his [FatFs website](http://elm-chan.org/fsw/ff/00index_e.html) as the original point of reference.

## Licence

Copyright (C) 2022, ChaN, all right reserved.

FatFs has being developed as a personal project of the author, ChaN. It is free from the code anyone else wrote at current release.

Following code block shows a copy of the FatFs license document that heading the source files.

```c
/*----------------------------------------------------------------------------/
/  FatFs - Generic FAT Filesystem Module  Rx.xx                               /
/-----------------------------------------------------------------------------/
/
/ Copyright (C) 20xx, ChaN, all right reserved.
/
/ FatFs module is an open source software. Redistribution and use of FatFs in
/ source and binary forms, with or without modification, are permitted provided
/ that the following condition is met:
/
/ 1. Redistributions of source code must retain the above copyright notice,
/    this condition and the following disclaimer.
/
/ This software is provided by the copyright holder and contributors "AS IS"
/ and any warranties related to this software are DISCLAIMED.
/ The copyright owner or contributors be NOT LIABLE for any damages caused
/ by use of this software.
/----------------------------------------------------------------------------*/
```

Therefore FatFs license is one of the BSD-style licenses, but there is a significant feature. FatFs is mainly intended for embedded systems. In order to extend the usability for commercial products, the redistributions of FatFs in binary form, such as embedded code, binary library and any forms without source code, do not need to include about FatFs in the documentations. This is equivalent to the 1-clause BSD license. Of course FatFs is compatible with the most of open source software licenses include GNU GPL. When you redistribute the FatFs source code with changes or create a fork, the license can also be changed to GNU GPL, BSD-style license or any open source software license that not conflict with FatFs license.
