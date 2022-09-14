# FT80x Library
The library for FT80x EVE display controllers.

Visit all support files and product documents click here.

Compiled with sccz80 version 19569-078eaec31-20220528, and using zsdcc version 4.2.0 [r13131](https://sourceforge.net/p/sdcc/code/13131/log/?path=/trunk/sdcc).

## Installation

The `z88dk-lib` function is used to install for the desired target. e.g.

```bash
cd ..
z88dk-lib +target ft80x
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
    1.git clone this repository.
    2.use `z88dk-lib` to import into z88k
	3.open a demo
	4.use incantation below
	5.profit

### ZSDCC
`zcc +yaz180 -subtype=app -SO3 --max-allocs-per-node400000 -v -m --list --math32 -llib/yaz180/ft80x @test.lst -o test -create-app`

### SCCZ80
`zcc +yaz180 -subtype=app -clib=new -O2 --opt-code-speed=inlineints -v -m --list --math32 -llib/yaz180/ft80x @test.lst -o test -create-app`

## Preparation

The library can be compiled using the following command lines in Linux, with the `+target` modified to be relevant to your machine.

```sh
zcc +yaz180 -clib=new -x -O2 --opt-code-speed=add32,sub32,sub16,inlineints --math32 @ft80x.lst -o ../ft80x
zcc +yaz180 -clib=sdcc_ix -x -SO3 --opt-code-speed --max-allocs-per-node400000 --math32 @ft80x.lst -o ../ft80x
zcc +yaz180 -clib=sdcc_iy -x -SO3 --opt-code-speed --max-allocs-per-node400000 --math32 @ft80x.lst -o ../ft80x
```

The resulting `ft80x.lib` file should be moved to `~/yaz180/lib/newlib/sccz80` or `~/yaz180/lib/newlib/sdcc_ix` or `~/yaz180/lib/newlib/sdcc_iy` respectively.

## Licence

This library is free software; you can redistribute it and/or modify it under the terms of the GNU Lesser General Public License as published by the Free Software Foundation; either version 2.1 of the License, or (at your option) any later version.

You should have received a copy of the GNU Lesser General Public License along with this library; if not, write to the Free Software Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
