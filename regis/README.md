## ReGIS - Remote Graphics Instruction Set
------------

ReGIS interprets commands that allow you to simply and efficiently control a video monitor screen and draw pictures on the screen with lines, curves, and circles. Also, ReGIS provides commands to include text characters in pictures. The ReGIS graphics language is designed for conciseness and easy transport of code from the host to the ReGIS device. The language consists of commands that are modified by options.

Compiled with sccz80 version 16894-223f580fd-20200818, and using zsdcc version 4.1.6 [r12419](https://sourceforge.net/p/sdcc/code/12419/log/?path=/trunk/sdcc).

## Installation

The `z88dk-lib` function is used to install for the desired target. e.g.

```bash
cd ..
z88dk-lib +target regis
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
	3.open the demo `regis_demo.c`
	4.use incantation below
	5.profit

### ZSDCC
`zcc +rc2014 -subtype=app -v -m -SO3 --list --math32  -llib/rc2014/regis --max-allocs-per-node100000 @test.lst -o test -create-app`

### SCCZ80
`zcc +rc2014 -subtype=app -clib=new -v -m -SO3 --list --math32 -llib/rc2014/regis @test.lst -o test -create-app`

## Preparation

The library can be compiled using the following command lines in Linux, with the `+target` modified to be relevant to your machine.

```
zcc +rc2014 --math32 -x -O2 -clib=new @regis.lst -o ../regis
zcc +rc2014 --math32 -x -SO3 -clib=sdcc_ix --max-allocs-per-node400000 @regis.lst -o ../regis
zcc +rc2014 --math32 -x -SO3 -clib=sdcc_iy --max-allocs-per-node400000 @regis.lst -o ../regis
```

The resulting `regis.lib` file should be moved to `~/target/lib/newlib/sccz80` or `~/target/lib/newlib/sdcc_ix` or `~/target/lib/newlib/sdcc_iy` respectively.

## License

This demo is licensed under [The MIT License](http://opensource.org/licenses/mit-license.php). Check LICENSE for more information.

Contributing to this software is warmly welcomed. You can do this basically by [forking](https://help.github.com/articles/fork-a-repo), committing modifications and then [pull requests](https://help.github.com/articles/using-pull-requests).

