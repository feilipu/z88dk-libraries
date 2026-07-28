## Introduction to selected z88dk Libraries

Summary of the file system management libraries.

```
FatFs --> diskio_sd    - use to access the CSIO SD Card interface on SC126, SC130, and SC131 directly.
      \-> diskio_hbios - use to access any logical disk interface supported by RomWBW HBIOS (use the logical drive number).
      \-> diskio integrated in z88dk - 82C55 IDE drivers for RC2014 IDE Hard Drive Module and YAZ180 integrated PPIDE.
```

**`f_*` is always FatFs** (ChaN API on block devices). It is **not** an alias for unprefixed `open` / `read` / `write`. On hardware CP/M applications (`+rc2014` / `+yaz180` / `+scz180` with **`-subtype=cpm`**), those unprefixed calls use **BDOS FCB**; FatFs remains a parallel stack on the raw media. Link `ff` (+ the matching diskio where needed) and usually **`time`** (for `get_fattime` write timestamps). Dual-stack policy and recipes: [z88dk wiki — Newlib File I/O and FatFs](https://github.com/z88dk/z88dk/wiki/Newlib_File_IO_and_FatFs).

| Target | diskio | Typical install |
|--------|--------|-----------------|
| **rc2014** | In-tree IDE (z88dk) | `z88dk-lib +rc2014 ff time` |
| **yaz180** | In-tree PPIDE (z88dk) | `z88dk-lib +yaz180 ff time` |
| **scz180** | Package **`diskio_sd`** | `z88dk-lib +scz180 diskio_sd ff time` |
| **hbios** | Package **`diskio_hbios`** | `z88dk-lib +hbios diskio_hbios ff time` |

Firmware / bare ROM builds (e.g. CP/M-IDE, YABIOS) often use `ff` or a read-only `ff_ro` without `-subtype=cpm`. CP/M **applications** that want host FCB files use `-subtype=cpm` and optionally the full `ff` packages above.

### diskio_hbios
FatFs is a generic FAT/exFAT filesystem module for small embedded systems. The FatFs module is written in compliance with ANSI C (C89) and completely separated from the disk I/O layer. This is the disk I/O layer for hbios, using hbios calls as implemented in ROMWBW. There is no CP/M FCB dual-stack on `+hbios` — FatFs is the disk API.

### diskio_sd
FatFs is a generic FAT/exFAT filesystem module for small embedded systems. The FatFs module is written in compliance with ANSI C (C89) and completely separated from the disk I/O layer. This is the disk I/O layer for the SC126, SC130, and SC131, using the SD interface as implemented in z88dk for the Z180 CSIO. Pair with `ff` (and `time`) for `+scz180`; under `-subtype=cpm` this is the FatFs half of the dual stack.

### FatFs
FatFs is a generic FAT/exFAT filesystem module for small embedded systems. The FatFs module is written in compliance with ANSI C (C89) and completely separated from the disk I/O layer. Therefore it is independent of the platform. It can be incorporated into small microcontrollers with limited resource, such as 8051, PIC, AVR, ARM, Z80, RX and etc. Always use the ChaN `f_*` API; install per target with `z88dk-lib` as in the table above.

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

NOTE WELL: z88dk third-party libraries install under **`lib/clibs/{sccz80,sdcc_ix,sdcc_iy}/lib/<target>/`** with headers under `include/_DEVELOPMENT/{proto,common}/lib/<target>/` (see z88dk #2814 / #2815).

The `z88dk-lib` tool installs packages for the desired target. Examples:

```bash
cd ~/z88dk-libraries
z88dk-lib +yaz180 time
z88dk-lib +rc2014 ff time
z88dk-lib +scz180 diskio_sd ff time
z88dk-lib +hbios diskio_hbios ff time
```

Further `z88dk-lib` usage:

```bash
z88dk-lib                    # list help
z88dk-lib +rc2014            # list installed third-party libs for target
z88dk-lib +rc2014 -r -f ff   # remove package(s); -f skips prompts
```

## Preparation

Compiled with sccz80 version 25141-6b30e0885e-20260716, and using zsdcc version 4.5.0 [r15242](https://sourceforge.net/p/sdcc/code/15242/log/?path=/trunk/sdcc). Libraries rebuilt 2026-07-28.


To **rebuild** a package from source, from that package’s `source` directory:

```bash
export PATH=…/z88dk/bin:$PATH ZCCCFG=…/z88dk/lib/config
# sccz80
zcc +<target> -clib=new -x -O2 --opt-code-speed=all --math32 @library.lst -o ../library
mkdir -p ../<target>/lib/newlib/sccz80
mv ../library.lib ../<target>/lib/newlib/sccz80/library.lib
# sdcc_ix / sdcc_iy
zcc +<target> -clib=sdcc_ix -x -SO3 --max-allocs-per-node400000 --math32 @library.lst -o ../library
mkdir -p ../<target>/lib/newlib/sdcc_ix
mv ../library.lib ../<target>/lib/newlib/sdcc_ix/library.lib
zcc +<target> -clib=sdcc_iy -x -SO3 --max-allocs-per-node400000 --math32 @library.lst -o ../library
mkdir -p ../<target>/lib/newlib/sdcc_iy
mv ../library.lib ../<target>/lib/newlib/sdcc_iy/library.lib
z88dk-lib +<target> library
```

**FatFs:** also build `ff_ro` (and rc2014 `ff_85` / `ff_85_ro`) into the **same** `lib/newlib/<clib>/` dirs as `ff.lib`. `z88dk-lib +<target> ff` only installs `ff.lib`; copy `ff_ro.lib` / `ff_85*.lib` into `lib/clibs/…/lib/<target>/` by hand. See `ff/readme.md`.

**Bulk rebuild + install:** `./rebuild-all.sh` in this repo root (requires `PATH`/`ZCCCFG` pointing at your z88dk). Max two concurrent `zcc` jobs.

1. Build package products under `<pkg>/<target>/lib/newlib/…`
2. **Install** with `z88dk-lib +<target> <pkg>` (remove-then-install, non-interactive)
3. **Manual copy** of extras `z88dk-lib` does not install: `ff_ro.lib`, `ff_85.lib`, `ff_85_ro.lib` into the same dirs as `ff.lib`, resolved from `ZCCCFG` as  
   `$ZCCCFG/../clibs/{sccz80,sdcc_ix,sdcc_iy}/lib/<target>/`

`./rebuild-all.sh` uses SDCC `--max-allocs-per-node400000` (same as package Preparation sections) and writes each product into `<pkg>/<target>/lib/newlib/{sccz80,sdcc_ix,sdcc_iy}/` before install.

## Usage

Once installed, link with `-llib/<target>/<library>` and include `#include <lib/<target>/<library>.h>`.

Example dual-stack CP/M application (FCB `open` + FatFs `f_*` on scz180):

```bash
z88dk-lib +scz180 diskio_sd ff time
zcc +scz180 -subtype=cpm -clib=new app.c \
  -llib/scz180/ff -llib/scz180/diskio_sd -llib/scz180/time -o app -m
```

For rc2014/yaz180, diskio is in the target library — only `ff` and `time` need installing. See the [z88dk Newlib File I/O and FatFs](https://github.com/z88dk/z88dk/wiki/Newlib_File_IO_and_FatFs) page for full recipes.