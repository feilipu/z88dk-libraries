## Introduction to the FreeRTOS functions


#### Features


## Preparation

The FreeRTOS library can be compiled using the following command lines in Linux, with the `+target` modified to be relevant to your machine.

`zcc +yaz180 -clib=new -x -SO3 --math32 @freertos.lst -o ../freertos`
`zcc +yaz180 -clib=sdcc_ix -x -SO3 --math32 --max-allocs-per-node400000 @freertos.lst -o ../freertos`
`zcc +yaz180 -clib=sdcc_iy -x -SO3 --math32 --max-allocs-per-node400000 @freertos.lst -o ../freertos`

`zcc +target -clib=new -x -SO3 --math32 @freertos.lst -o ../freertos`
`zcc +target -clib=sdcc_ix -x -SO3 --math32 --max-allocs-per-node400000 @freertos.lst -o ../freertos`
`zcc +target -clib=sdcc_iy -x -SO3 --math32 --max-allocs-per-node400000 @freertos.lst -o ../freertos`

The resulting `freertos.lib` file should be moved to `~/target/lib/newlib/sccz80` or `~/target/lib/newlib/sdcc_ix` or `~/target/lib/newlib/sdcc_iy` respectively.

The `z88dk-lib` function is used to install for the desired target. e.g.

```bash
cd ..
z88dk-lib +target -f freertos
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

Once installed, the FreeRTOS library can be linked against on the compile line by adding `-llib/target/freertos` and the include file can be found with `#include <lib/target/freertos.h>`.

A simple usage example, for the `+yaz180` target.


## Documentation


## Licence

