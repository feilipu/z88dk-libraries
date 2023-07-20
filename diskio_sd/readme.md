## Introduction to the Z80 FatFS diskio_sd functions

FatFs is a generic FAT/exFAT filesystem module for small embedded systems. The FatFs module is written in compliance with ANSI C (C89) and completely separated from the disk I/O layer. Therefore it is independent of the platform. It can be incorporated into small microcontrollers with limited resource, such as 8051, PIC, AVR, ARM, Z80, RX and etc.

Compiled with sccz80 version 21481-8e126c50b2-20230720, and using zsdcc version 4.3.0 [r14210](https://sourceforge.net/p/sdcc/code/14210/log/?path=/trunk/sdcc).

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
z88dk-lib +target -f diskio_sd
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

Once installed, the FatFs library can be linked against on the compile line by adding `-llib/target/diskio_sd` and the include file can be found with `#include <lib/target/diskio_sd.h>`.

A simple usage example, for the `+scz180` target.

```c
/*----------------------------------------------------------------------*/
/* Foolproof FatFs sample project for Z80              (C)ChaN, 2014    */
/*----------------------------------------------------------------------*/
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#if __SCZ180
#include <lib/scz180/ff.h>     /* Declarations of FatFs API */
#include <lib/scz180/diskio_sd.h> 
#endif

// zcc +scz180 -subtype=app -clib=sdcc_iy -v --list -m -SO3 --opt-code-size --max-allocs-per-node200000 -llib/scz180/diskio_sd -llib/scz180/ff ff_main.c -o ff_main -create-app

static FATFS FatFs;		/* FatFs work area needed for each volume */
static FIL Fil;			/* File object needed for each open file */

void main (void)
{
	UINT bw;

	f_mount(&FatFs, "0:", 0);	        /* Give a work area to the SCZ180 default SD drive 0/1 */
//  f_mount(&FatFs, "2:", 0);	        /* Give a work area to the HBIOS SD0 */

	if (f_open(&Fil, "newfile.txt", FA_WRITE | FA_CREATE_ALWAYS) == FR_OK)
	{	/* Create a file */

		f_write(&Fil, "It works!\r\n", 11, &bw);	/* Write data to the file */

		f_close(&Fil);								/* Close the file */

		printf("It works!\r\n");
	}
}

```

## Preparation

The diskio_sd library can be compiled using the following command lines in Linux, with the `+target` modified to be relevant to your machine.

```
zcc +scz180 -clib=new -x -O2 --opt-code-speed=all --math32 @diskio_sd.lst -o ../diskio_sd
zcc +scz180 -clib=sdcc_ix -x -SO3 --max-allocs-per-node400000 --math32 @diskio_sd.lst -o ../diskio_sd
zcc +scz180 -clib=sdcc_iy -x -SO3 --max-allocs-per-node400000 --math32 @diskio_sd.lst -o ../diskio_sd
```
```
zcc +target -clib=new -x -02 --opt-code-speed=all --math32 @diskio_sd.lst -o ../diskio_sd
zcc +target -clib=sdcc_ix -x -SO3 --max-allocs-per-node400000 --math32 @diskio_sd.lst -o ../diskio_sd
zcc +target -clib=sdcc_iy -x -SO3 --max-allocs-per-node400000 --math32 @diskio_sd.lst -o ../diskio_sd
```
The resulting `diskio_sd.lib` file should be moved to `~/scz180/lib/newlib/sccz80` or `~/scz180/lib/newlib/sdcc_ix` or `~/scz180/lib/newlib/sdcc_iy` respectively.

## Documentation

ChaN's documentation here [FatFs website](http://elm-chan.org/fsw/ff/00index_e.html), as the point of reference.

## Licence

Copyright (C) 2019, ChaN, all right reserved.

FatFs module is an open source software. Redistribution and use of FatFs in source and binary forms, with or without modification, are permitted provided that the following condition is met:

1. Redistributions of source code must retain the above copyright notice, this condition and the following disclaimer.

This software is provided by the copyright holder and contributors "AS IS" and any warranties related to this software are DISCLAIMED. The copyright owner or contributors be NOT LIABLE for any damages caused by use of this software.
