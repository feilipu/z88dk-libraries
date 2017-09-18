## Introduction to selected z88dk Libraries
This file describes some some z88dk compatible libraries.


### z88dk-time
The implementation of `time.h` aspires to conform with ISO/IEC 9899 (C90). However, due to limitations of the target processor and the nature of its development environment, a practical implementation must of necessity deviate from the standard.

### z88dk-FatFS
TBC

### z88dk-FreeRTOS
TBC

## Preparation
The libraries can be compiled using the following command lines in Linux, with the `+target` modified to be relevant to your machine, from the relevant library sub-directory. e.g. for the time library.

```bash
cd time
zcc +target -lm -x -SO3 --opt-code-size -clib=sdcc_ix --max-allocs-per-node400000 @time.lst -o time
mv time.lib target/lib/newlib/sdcc_ix
zcc +target -lm -x -SO3 --opt-code-size -clib=sdcc_iy --max-allocs-per-node400000 @time.lst -o time
mv time.lib target/lib/newlib/sdcc_iy
```
The resulting `time.lib` file should be moved to `~/time/target/lib/newlib/sdcc_ix` or `~/time/target/lib/newlib/sdcc_iy` respectively, as noted above.

Then, the `z88dk-lib` function is used to install for the desired target. e.g. for the yaz180 machine.

```bash
cd ..
z88dk-lib +yaz180 time
```

Some further examples of `z88dk-lib` usage.

+ libraries list help
```bash
z88dk-lib
```
+ list 3rd party libraries already installed for the zx target
```bash
z88dk-lib +zx
```
+ remove the `libname1` `libname2` ... libraries from the zx target, -f for no nagging about deleting files.
```bash
z88dk-lib +zx -r -f libname1 libname2 ...
```

## Usage
Once installed, the libraries can be linked against on the compile line by adding `-llib/target/library` and the include file can be found with `#include <lib/target/library.h>`.
