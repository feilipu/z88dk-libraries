# I2C_LCD_Library
The library for I2C_LCD.

Visit all support files and product documents [click here](https://github.com/SparkingStudio/I2C_LCD) please.

Compiled using zsdcc version 4.0.0 [r11535](https://sourceforge.net/p/sdcc/code/11535/log/?path=/trunk/sdcc).

## Installation

The `z88dk-lib` function is used to install for the desired target. e.g.

```bash
cd ..
z88dk-lib +target i2c_lcd
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
`zcc +yaz180 -subtype=app -v -m -SO3 --list --math32_z180  -llib/yaz180/i2c_lcd --c-code-in-asm --max-allocs-per-node400000 @test.lst -o test -create-app`

### SCCZ80
`zcc +yaz180 -subtype=app -clib=new -v -m -SO3 --list --math32_z180 -llib/yaz180/i2c_lcd @test.lst -o test -create-app`

## Preparation

The library can be compiled using the following command lines in Linux, with the `+target` modified to be relevant to your machine.

`zcc +yaz180 --math32 -x -SO3 -clib=new @i2c_lcd.lst -o ../i2c_lcd`

`zcc +yaz180 --math32 -x -SO3 -clib=sdcc_ix --max-allocs-per-node400000 @i2c_lcd.lst -o ../i2c_lcd`

`zcc +yaz180 --math32 -x -SO3 -clib=sdcc_iy --max-allocs-per-node400000 @i2c_lcd.lst -o ../i2c_lcd`

The resulting `i2c_lcd.lib` file should be moved to `~/target/lib/newlib/sccz80` or `~/target/lib/newlib/sdcc_ix` or `~/target/lib/newlib/sdcc_iy` respectively.

## Licence

This library is free software; you can redistribute it and/or modify it under the terms of the GNU Lesser General Public License as published by the Free Software Foundation; either version 2.1 of the License, or (at your option) any later version.

This library is build for I2C_LCD12864. Please do not use this library on any other devices, that could cause unpredictable damage to the unknown device. See the GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License along with this library; if not, write to the Free Software Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
