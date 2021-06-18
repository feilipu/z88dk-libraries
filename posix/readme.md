## Introduction to the posix wrappers for FatFS functions

__NOTE__ This library is work in progress and is not functioning.

FatFs is a generic FAT/exFAT filesystem module for small embedded systems. The FatFs module is written in compliance with ANSI C (C89) and completely separated from the disk I/O layer. These functions are posix wrappers to enable standard posix function calling for FatFs.

Compiled using zsdcc version 4.0.0 [r11556](https://sourceforge.net/p/sdcc/code/11556/log/?path=/trunk/sdcc).

#### Features

[Mike Gore](https://github.com/magore): _I created some [posix wrappers for fatfs](https://github.com/magore/hp85disk/tree/master/posix) that allow you to use many unix file operations. It provides the following functions (note that fdevopen sets up stdio and is not posix):_

- POSIX character I/O functions
    - isatty
    - fgetc
    - fputc
    - getchar
    - putc
    - putchar
    - ungetc

- POSIX string I/O functions
    - fgets
    - fputs
    - puts

- POSIX file position functions
    - feof
    - fgetpos
    - fseek
    - fsetpos
    - ftell
    - lseek
    - rewind

- POSIX file functions
    - close
    - fileno
    - fileno_to_stream  __NOT POSIX__
    - fopen
    - fread
    - ftruncate
    - fwrite
    - open
    - read
    - sync
    - syncfs
    - truncate
    - write
    - fclose

- POSIX file information functions
    - dump_stat - __NOT POSIX__
    - fstat
    - mctime    - __NOT POSIX__
    - stat

- POSIX file and directory manipulation
    - basename
    - baseext   - __NOT POSIX__
    - chmod
    - chdir
    - dirname
    - getcwd
    - mkdir
   - rmdir
    - unlink
    - utime

- POSIX - directory scanning functions
    - closedir
    - opendir
    - readdir

- POSIX error functions
    - clrerror
    - ferror
    - perror
    - strerror
    - strerror_r

- Device open functions
    - fdevopen  - __NOT POSIX__

- Character reading functions using (fast) buffers - __NOT POSIX__
    - buffer_read_open
    - buffer_read_close
    - buffer_ungetc
    - buffer_getc
    - buffer_gets

## Installation

The `z88dk-lib` function is used to install for the desired target. e.g.

```bash
cd ..
z88dk-lib +target -f posix
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

Once installed, the FatFs library can be linked against on the compile line by adding `-llib/target/posix` and the include file can be found with `#include <lib/target/posix.h>`.

A simple usage example, for the `+yaz180`, `+scz180`, or `+rc2014` targets.

```c
/*----------------------------------------------------------------------*/
/* Foolproof FatFs sample project for Z80              (C)ChaN, 2014    */
/*----------------------------------------------------------------------*/
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "ffconf.h"                 /* The ffconf.h file from library compilation */

#if __RC2014
#include <lib/rc2014/ff.h>          /* Declarations of FatFs API */
#include <lib/rc2014/posix.h>       /* Declarations of posix API */

#elif __SCZ180
#include <lib/scz180/ff.h>          /* Declarations of FatFs API */
#include <lib/scz180/posix.h>       /* Declarations of posix API */

#elif __YAZ180
#include <lib/yaz180/ff.h>          /* Declarations of FatFs API */
#include <lib/yaz180/posix.h>       /* Declarations of posix API */

#endif

// zcc +yaz180 -subtype=app -v --list -m -SO3 -clib=sdcc_iy -llib/yaz180/posix -llib/yaz180/ff --max-allocs-per-node200000 ff_main.c -o ff_main -create-app

// zcc +scz180 -subtype=hbios -v --list -m -SO3 -clib=sdcc_iy -llib/scz180/posix -llib/scz180/ff  -llib/scz180/diskio_hbios --max-allocs-per-node200000 ff_main.c -o ff_main -create-app

// zcc +rc2014 -subtype=app -v --list -m -SO3 -clib=sdcc_iy -llib/rc2014/posix -llib/rc2014/ff --max-allocs-per-node200000 ff_main.c -o ff_main -create-app


static FATFS FatFs;		/* FatFs work area needed for each volume */
static FIL Fil;			/* File object needed for each open file */

void main (void)
{
	UINT bw;

	f_mount(&FatFs, "0:", 0);	        /* Give a work area to the default drive */
//  f_mount(&FatFs, "3:", 0);	        /* Give a work area to the HBIOS SD0: drive */

	if (f_open(&Fil, "newfile.txt", FA_WRITE | FA_CREATE_ALWAYS) == FR_OK)
	{	/* Create a file */

		f_write(&Fil, "It works!\r\n", 11, &bw);	/* Write data to the file */

		f_close(&Fil);								/* Close the file */

		printf("It works!\r\n");
	}
}

```

## Preparation

The posix library can be compiled from the `posix/source` directory using the following command lines in Linux, with the `+target` modified to be relevant to your machine.

```
zcc +rc2014 -clib=new -x -O2 --math32 @posix.lst -o ../posix
zcc +rc2014 -clib=sdcc_ix -x -SO3 --math32 --max-allocs-per-node400000 @posix.lst -o ../posix
zcc +rc2014 -clib=sdcc_iy -x -SO3 --math32 --max-allocs-per-node400000 @posix.lst -o ../posix
```
```
zcc +yaz180 -clib=new -x -O2 --math32 @posix.lst -o ../posix
zcc +yaz180 -clib=sdcc_ix -x -SO3 --math32--max-allocs-per-node400000 @posix.lst -o ../posix
zcc +yaz180 -clib=sdcc_iy -x -SO3 --math32 --max-allocs-per-node400000 @posix.lst -o ../posix
```
For any supported target.
```
zcc +target -clib=new -x -O2 --math32 @posix.lst -o ../posix
zcc +target -clib=sdcc_ix -x -SO3 --math32 --max-allocs-per-node400000 @posix.lst -o ../posix
zcc +target -clib=sdcc_iy -x -SO3 --math32 --max-allocs-per-node400000 @posix.lst -o ../posix
```
The resulting `posix.lib` file should be moved to `~/target/lib/newlib/sccz80` or `~/target/lib/newlib/sdcc_ix` or `~/target/lib/newlib/sdcc_iy` respectively.

## Documentation

Mikes documentation is available [here](https://rawgit.com/magore/hp85disk/master/doxygen/html/index.html), for easy reference.

## Licence

Copyright (C) 2014-2017, Mike Gore, all right reserved.

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sub-license, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
