# I2C_LCD_Library
The library for I2C_LCD.

Visit all support files and product documents [click here](https://github.com/SparkingStudio/I2C_LCD) please.


`zcc +yaz180 -subtype=app -v -m -SO3 --list --math32_z180 -llib/yaz180/i2c_lcd --c-code-in-asm --max-allocs-per-node200000 @i2ctest.lst -o i2ctest`

## Preparation

The library can be compiled using the following command lines in Linux, with the `+target` modified to be relevant to your machine.

`zcc +yaz180 --math32 -x -SO3 -clib=new i2c_lcd.c -o i2c_lcd`

`zcc +yaz180 --math32 -x -SO3 -clib=sdcc_ix --max-allocs-per-node400000 i2c_lcd.c -o i2c_lcd`

`zcc +yaz180 --math32 -x -SO3 -clib=sdcc_iy --max-allocs-per-node400000 i2c_lcd.c -o i2c_lcd`

The resulting `th02.lib` file should be moved to `~/target/lib/newlib/sccz80` or `~/target/lib/newlib/sdcc_ix` or `~/target/lib/newlib/sdcc_iy` respectively.

The `z88dk-lib` function is used to install for the desired target. e.g.

```bash
cd ..
z88dk-lib +target th02
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

