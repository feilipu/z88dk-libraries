## 3D Vector and Matrix Manipulation
------------

3D provides functions that allow you to simply and efficiently manage 3D Vectors and Matrices.

Compiled with sccz80 version 19569-078eaec31-20220528, and using zsdcc version 4.2.0 [r13131](https://sourceforge.net/p/sdcc/code/13131/log/?path=/trunk/sdcc).

## Installation

The `z88dk-lib` function is used to install for the desired target. e.g.

```bash
cd ..
z88dk-lib +target 3d
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
    2.use `z88dk-lib` to import this library into z88k.
    3.open the demo `demo_3d.c` to test your success.
    4.compile the demonstration using the incantation(s) below.
    5.profit.

### ZSDCC

For CP/M<br>
`zcc +cpm -clib=sdcc_iy -SO3 --max-allocs-per-node100000 -v -m --list -llib/cpm/regis -llib/cpm/3d --math32 demo_3d.c -o 3d -create-app`

For RC2014<br>
`zcc +rc2014 -subtype=cpm -SO3 --max-allocs-per-node100000 -v -m --list -llib/rc2014/regis -llib/rc2014/3d --math32 demo_3d.c -o 3d -create-app`

### SCCZ80

For CP/M<br>
`zcc +cpm -clib=new -O2 --opt-code-speed=all -v -m --list -llib/rc2014/3d demo_3d.c -o 3d -create-app`

For RC2014<br>
`zcc +rc2014 -subtype=cpm -clib=new -O2 --opt-code-speed=all -v -m --list -llib/rc2014/regis -llib/rc2014/3d --math32 demo_3d.c -o 3d -create-app`
`zcc +rc2014 -subtype=cpm -clib=new -v -m -O2 --opt-code-speed=all --list -llib/rc2014/regis -llib/rc2014/3df16 --math16 demo_3d.c -o 3df16 -create-app`

For YAZ180<br>
`zcc +yaz180 -subtype=cpm -clib=new -O2 --opt-code-speed=all -v -m --list -llib/yaz180/regis -llib/yaz180/3d --math32 demo_3d.c -o 3d -create-app`
`zcc +yaz180 -subtype=cpm -clib=new -v -m -O2 --opt-code-speed=all --list -llib/yaz180/regis -llib/yaz180/3df16 --math16 demo_3d.c -o 3df16 -create-app`

## Preparation

The library can be compiled using the following command lines in Linux, with the `+target` (eg. `+rc2014`) modified to be relevant to your machine.

```sh
zcc +rc2014 --math32 -x -O2  -clib=new --opt-code-speed=all @3d.lst -o ../3d
zcc +rc2014 --math32 -x -SO3 -clib=sdcc_ix --max-allocs-per-node400000 @3d.lst -o ../3d
zcc +rc2014 --math32 -x -SO3 -clib=sdcc_iy --max-allocs-per-node400000 @3d.lst -o ../3d

zcc +rc2014 --math16 -x -O2  -clib=new --opt-code-speed=all @3d.lst -o ../3df16
```

```sh
zcc +yaz180 --math32 -x -O2  -clib=new --opt-code-speed=all @3d.lst -o ../3d
zcc +yaz180 --math32 -x -SO3 -clib=sdcc_ix --max-allocs-per-node400000 @3d.lst -o ../3d
zcc +yaz180 --math32 -x -SO3 -clib=sdcc_iy --max-allocs-per-node400000 @3d.lst -o ../3d

zcc +yaz180 --math16 -x -O2  -clib=new --opt-code-speed=all @3d.lst -o ../3df16
```

```sh
zcc +cpm --math32 -x -O2 -clib=new --opt-code-speed=all @3d.lst -o ../3d
zcc +cpm --math32 -x -SO3 -clib=sdcc_ix --max-allocs-per-node400000 @3d.lst -o ../3d
zcc +cpm --math32 -x -SO3 -clib=sdcc_iy --max-allocs-per-node400000 @3d.lst -o ../3d

zcc +cpm --math16 -x -O2 -clib=new --opt-code-speed=all @3d.lst -o ../3df16
```

The resulting `3d.lib` files should be moved to `~/target/lib/newlib/sccz80` or `~/target/lib/newlib/sdcc_ix` or `~/target/lib/newlib/sdcc_iy` respectively.


## Demonstration

```sh
#  ZSDCC compile from demo directory
>  zcc +cpm -clib=sdcc_iy -v -m -SO3 --max-allocs-per-node100000 --list -llib/cpm/regis -llib/cpm/3d --math32 demo_3d.c -o 3d -create-app
>  zcc +cpm -clib=sdcc_iy -v -m -SO3 --max-allocs-per-node100000 --list -llib/cpm/regis -llib/cpm/3d --am9511 demo_3d.c -o 3dapu -create-app

#  SCCZ80 compile from demo directory
>  zcc +cpm -clib=new -v -m -O2 --opt-code-speed=all --list -llib/cpm/regis -llib/cpm/3d --math32 demo_3d.c -o 3d -create-app
>  zcc +cpm -clib=new -v -m -O2 --opt-code-speed=all --list -llib/cpm/regis -llib/cpm/3d --am9511 demo_3d.c -o 3dapu -create-app

#  SCCZ80 compile from demo directory with math16 (16-bit floating point)
>  zcc +cpm -clib=new -v -m -O2 --opt-code-speed=all --list -llib/cpm/regis -llib/cpm/3df16 --math16 demo_3d.c -o 3df16 -create-app

#  display ReGIS output using XTerm & picocom
>  xterm +u8 -geometry 132x50 -ti 340 -tn 340 -e picocom -b 115200 -p 2 -f h /dev/ttyUSB0 --send-cmd "sx -vv"
```

Which should produce the below result.

<div>
<table style="border: 2px solid #cccccc;">
<tbody>
<tr>
<td style="border: 1px solid #cccccc; padding: 6px;"><a href="https://github.com/feilipu/z88dk-libraries/blob/master/3d/doc/z80gears.png" target="_blank"><img src="https://github.com/feilipu/z88dk-libraries/blob/master/3d/doc/z80gears.png"/></a></td>
</tr>
<tr>
<th style="border: 1px solid #cccccc; padding: 6px;"><centre>RC2014 ReGIS - GLXGEARS<center></th>
</tr>
</tbody>
</table>
</div>



## Credits

[GLX Gears for Arduino](https://github.com/heroineworshiper/3d_arduino/blob/main/3d_arduino.ino)

[3D homogeneous coordinate definition](https://en.wikipedia.org/wiki/Homogeneous_coordinates)

[3D Clipping in Homogeneous Coordinates](https://chaosinmotion.com/2016/05/22/3d-clipping-in-homogeneous-coordinates/)

[Project 3D coords onto 2D screen](https://stackoverflow.com/questions/724219/how-to-convert-a-3d-point-into-2d-perspective-projection)

[Transformation matrix](https://www.tutorialspoint.com/computer_graphics/3d_transformation.htm)


## License

This library is licensed under [The MIT License](http://opensource.org/licenses/mit-license.php). Check the LICENSE file for more information.

## Contributing

Contributing to this software is warmly welcomed. You can do this by [forking](https://help.github.com/articles/fork-a-repo), committing modifications and then [pull requests](https://help.github.com/articles/using-pull-requests).
