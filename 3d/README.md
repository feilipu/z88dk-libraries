## 3D Vector and Matrix Manipulation
------------

3D provides functions that allow you to simply and efficiently manage 3D Vectors and Matrices.

Compiled with sccz80 version 22635-40749b9e71-20240612, and using zsdcc version 4.4.0 [r14648](https://sourceforge.net/p/sdcc/code/14648/log/?path=/trunk/sdcc).

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

_Note:_ the 16-bit `math16` library `3df16.lib` will need to be copied manually into the correct z88dk directory as `z88dk-lib` tool can only handle one library file per tree.


## Usage
    1.git clone this repository.
    2.use `z88dk-lib` to import this library into z88k.
    3.open the demo `demo_3d.c` to test your success.
    4.compile the demonstration program using the incantation(s) below.
    5.profit.

### Vector functions

```C
/* Produce a unit vector */
void unit_v(vector_t * vect);

/* Scale a vector by m, but don't touch w dimension */
void scale_v(vector_t * vect, float scale);

/* Produce a dot product between vectors */
float dot_v(vector_t * vect1, vector_t * vect2);

/* Vector Matrix Multiplication */
void mult_v(vector_t * vect,matrix_t * multiplier);
```

### Matrix 3D (Homogeneous Coordinate) functions

```C
/* Produce an identity matrix */
void identity_m(matrix_t * matrix);

/* Produce a transformation (translation) */
void translate_m(matrix_t * matrix, float x, float y, float z);

/* Produce a transformation (scale) */
void scale_m(matrix_t * matrix, float x, float y, float z);

/* Produce a transformation (shear) */
void shear_m(matrix_t * matrix, float x, float y, float z);

/* Rotation in x dimension */
void rotx_m(matrix_t * matrix, float angle);

/* Rotation in y dimension */
void roty_m(matrix_t * matrix, float angle);

/* Rotation in z dimension */
void rotz_m(matrix_t * matrix, float angle);

/* Set up a projection matrix - OpenGL */
void projection_opengl_m(matrix_t * matrix, float fov, float aspect_ratio, float near, float far);

/* Set up a projection matrix - W3Woody */
void projection_w3woody_m(matrix_t * matrix, float fov, float aspect_ratio, float near, float far);

/* Matrix Multiplication */
void mult_m(matrix_t * multiplicand, matrix_t * multiplier);
```

## Preparation

The library can be compiled using the following command lines in Linux, with the `+target` (eg. `+rc2014`) modified to be relevant to your machine.

```sh
>  zcc +rc2014 -x -clib=new -O2 --opt-code-speed=all --math32 @3d.lst -o ../3d
>  zcc +rc2014 -x -clib=sdcc_ix -SO3 --max-allocs-per-node400000 --math32 @3d.lst -o ../3d
>  zcc +rc2014 -x -clib=sdcc_iy -SO3 --max-allocs-per-node400000 --math32 @3d.lst -o ../3d

>  zcc +rc2014 -x -clib=new -O2 --opt-code-speed=all --math16 @3d.lst -o ../3df16
```

```sh
>  zcc +yaz180 -x -clib=new -O2 --opt-code-speed=all --math32 @3d.lst -o ../3d
>  zcc +yaz180 -x -clib=sdcc_ix -SO3 --max-allocs-per-node400000 --math32 @3d.lst -o ../3d
>  zcc +yaz180 -x -clib=sdcc_iy -SO3 --max-allocs-per-node400000 --math32 @3d.lst -o ../3d

>  zcc +yaz180 -x -O2 -clib=new --opt-code-speed=all --math16 @3d.lst -o ../3df16
```

```sh
>  zcc +cpm -x -clib=new -O2 --opt-code-speed=all --math32 @3d.lst -o ../3d
>  zcc +cpm -x -clib=sdcc_ix -SO3 --max-allocs-per-node400000 --math32 @3d.lst -o ../3d
>  zcc +cpm -x -clib=sdcc_iy -SO3 --max-allocs-per-node400000 --math32 @3d.lst -o ../3d

>  zcc +cpm -x -clib=new -O2 --opt-code-speed=all --math16 @3d.lst -o ../3df16

>  zcc +cpm -x -clib=8085 -O2 --opt-code-speed=all --math-am9511 @3d.lst -o ../3d_8085
```

The resulting `3d.lib` or `3df16.lib` files should be moved to `~/target/lib/newlib/sccz80` or `~/target/lib/newlib/sdcc_ix` or `~/target/lib/newlib/sdcc_iy` respectively.

## Demonstration

### CP/M

```sh
#  ZSDCC compile from demo directory
>  zcc +cpm -clib=sdcc_iy -v -m --list -SO3 --max-allocs-per-node100000 -llib/cpm/regis -llib/cpm/3d --math32 demo_3d.c -o 3d -create-app
>  zcc +cpm -clib=sdcc_iy -v -m --list -SO3 --max-allocs-per-node100000 -llib/cpm/regis -llib/cpm/3d --am9511 demo_3d.c -o 3dapu -create-app

#  SCCZ80 compile from demo directory
>  zcc +cpm -clib=new -v -m --list -O2 --opt-code-speed=all -llib/cpm/regis -llib/cpm/3d --math32 demo_3d.c -o 3d -create-app
>  zcc +cpm -clib=new -v -m --list -O2 --opt-code-speed=all -llib/cpm/regis -llib/cpm/3d --am9511 demo_3d.c -o 3dapu -create-app

#  SCCZ80 compile from demo directory with math16 (16-bit floating point)
>  zcc +cpm -clib=new -v -m --list -O2 --opt-code-speed=all -llib/cpm/regis -llib/cpm/3df16 --math16 demo_3d.c -o 3df16 -create-app

#  SCCZ80 compile from demo directory for 8085 with Am9511 APU
>  zcc +cpm -clib=8085 -v -m --list -O2 --opt-code-speed=all -DAMALLOC -lregis_8085 -l3d_8085 --math-am9511 demo_3d.c -o 3d-8085 -create-app

#  display ReGIS output using XTerm & picocom
>  xterm +u8 -geometry 132x50 -ti 340 -tn 340 -T "ReGIS" -e picocom -b 115200 -p 2 -f h /dev/ttyUSB0 --send-cmd "sx -vv"
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

### RC2014 CP/M

```sh
#  For SDCC
>  zcc +rc2014 -subtype=cpm -v -m --list -SO3 --max-allocs-per-node100000 -llib/rc2014/regis -llib/rc2014/3d --math32 demo_3d.c -o 3d -create-app
>  zcc +rc2014 -subtype=cpm -v -m --list -SO3 --max-allocs-per-node100000 -llib/rc2014/regis -llib/rc2014/3d --am9511 demo_3d.c -o 3dapu -create-app

#  For SCCZ80
>  zcc +rc2014 -subtype=cpm -clib=new -v -m --list -O2 --opt-code-speed=all -llib/rc2014/regis -llib/rc2014/3d --math32 demo_3d.c -o 3d -create-app
>  zcc +rc2014 -subtype=cpm -clib=new -v -m --list -O2 --opt-code-speed=all -llib/rc2014/regis -llib/rc2014/3d --am9511 demo_3d.c -o 3dapu -create-app
>  zcc +rc2014 -subtype=cpm -clib=new -v -m --list -O2 --opt-code-speed=all -llib/rc2014/regis -llib/rc2014/3df16 --math16 demo_3d.c -o 3df16 -create-app
```

### YAZ180 CP/M

```sh
#  For SDCC
>  zcc +yaz180 -subtype=cpm -v -m --list -SO3 --max-allocs-per-node100000 -llib/yaz180/regis -llib/yaz180/3d --math32 demo_3d.c -o 3d -create-app
>  zcc +yaz180 -subtype=cpm -v -m --list -SO3 --max-allocs-per-node100000 -llib/yaz180/regis -llib/yaz180/3d --am9511 demo_3d.c -o 3dapu -create-app

#  For SCCZ80
>  zcc +yaz180 -subtype=cpm -clib=new -v -m --list -O2 --opt-code-speed=all -llib/yaz180/regis -llib/yaz180/3d --math32 demo_3d.c -o 3d -create-app
>  zcc +yaz180 -subtype=cpm -clib=new -v -m --list -O2 --opt-code-speed=all -llib/yaz180/regis -llib/yaz180/3d --am9511 demo_3d.c -o 3dapu -create-app
>  zcc +yaz180 -subtype=cpm -clib=new -v -m --list -O2 --opt-code-speed=all -llib/yaz180/regis -llib/yaz180/3df16 --math16 demo_3d.c -o 3df16 -create-app
```

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
