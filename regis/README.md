## ReGIS - Remote Graphics Instruction Set
------------

ReGIS interprets commands that allow you to simply and efficiently control a video monitor screen and draw pictures on the screen with lines, curves, and circles using a serial interface (USART). Also, ReGIS provides commands to include scalable text characters in pictures. The ReGIS graphics language is designed for conciseness and easy transport of code from the host to the ReGIS device. The language consists of commands that are modified by options.

Read here for a full description on [how to enable ReGIS for Windows 10 and Linux desktop machines](https://feilipu.me/2022/09/28/regis-serial-graphics-for-arduino-rc2014/).

There is a [Programmer Reference Manual for VT330/VT340 Terminals](https://vt100.net/docs/vt3xx-gp/) describing how to use ReGIS, as well as the [VT125 Primer](https://github.com/feilipu/z88dk-libraries/blob/master/regis/doc/EK-VT125-GI-001_VT125_ReGIS_Primer_May82.pdf) document provided here.

Compiled with sccz80 version 22635-40749b9e71-20240612, and using zsdcc version 4.4.0 [r14648](https://sourceforge.net/p/sdcc/code/14648/log/?path=/trunk/sdcc).

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
    2.use `z88dk-lib` to import this library into z88k.
    3.open the demo `regis_demo.c` to test your success.
    4.compile the demonstration using the incantation(s) below.
    5.profit.

### ZSDCC

For CP/M<br>
`zcc +cpm -clib=sdcc_iy -SO3 --max-allocs-per-node100000 -v -m --list -llib/rc2014/regis regis_demo.c -o regis -create-app`

For RC2014<br>
`zcc +rc2014 -subtype=cpm -clib=sdcc_iy -SO3 --max-allocs-per-node100000 -v -m --list -llib/rc2014/regis regis_demo.c -o regis -create-app`

### SCCZ80

For CP/M<br>
`zcc +cpm -clib=new -O2 --opt-code-speed=inlineints -v -m --list -llib/rc2014/regis regis_demo.c -o regis -create-app`

For RC2014<br>
`zcc +rc2014 -subtype=cpm -clib=new -O2 --opt-code-speed=inlineints -v -m --list -llib/rc2014/regis regis_demo.c -o regis -create-app`

## Preparation

The library can be compiled using the following command lines in Linux, with the `+target` (eg. `+rc2014`) modified to be relevant to your machine.

```
zcc +rc2014  -clib=new  --math32 -x -O2--opt-code-speed=all @regis.lst -o ../regis
zcc +rc2014 -clib=sdcc_ix --math32 -x -SO3 --max-allocs-per-node400000 @regis.lst -o ../regis
zcc +rc2014 -clib=sdcc_iy --math32 -x -SO3 --max-allocs-per-node400000 @regis.lst -o ../regis
```

```
zcc +yaz180 -clib=new --math32 -x -O2 --opt-code-speed=all @regis.lst -o ../regis
zcc +yaz180 -clib=sdcc_ix --math32 -x -SO3 --max-allocs-per-node400000 @regis.lst -o ../regis
zcc +yaz180 -clib=sdcc_iy --math32 -x -SO3 --max-allocs-per-node400000 @regis.lst -o ../regis
```

```
zcc +cpm -clib=new --math32 -x -O2 --opt-code-speed=all @regis.lst -o ../regis
zcc +cpm -clib=sdcc_ix --math32 -x -SO3 --max-allocs-per-node400000 @regis.lst -o ../regis
zcc +cpm -clib=sdcc_iy --math32 -x -SO3 --max-allocs-per-node400000 @regis.lst -o ../regis

zcc +cpm -clib=8085 --math-am9511 -x -O2 --opt-code-speed=all @regis.lst -o ../regis_8085
```

The resulting `regis.lib` files should be moved to `~/target/lib/newlib/sccz80` or `~/target/lib/newlib/sdcc_ix` or `~/target/lib/newlib/sdcc_iy` respectively.

The resulting libraries can be installed into the z88dk library folders using the `z88dk-lib` tool. 

## Preparing XTerm to support ReGIS

XTerm is the only known software solution supporting ReGIS commands, but it doesn't support ReGIS in the default build. You'll need to enable ReGIS and build it yourself.
``` sh
$ sudo apt install -y libxaw7-dev libncurses-dev libxft-dev
$ wget https://invisible-island.net/datafiles/release/xterm.tar.gz
$ tar xf xterm.tar.gz
$ cd xterm-392
$ ./configure --enable-regis-graphics
$ make
$ sudo make install
```

As XTerm has no serial interface itself, so you'll need to use one. A suggestion is to use picocom.<br>
It is also useful for working with retrocomputers generally. First test that your installation is working as per below.

Adding the `--send-cmd` option will allow the use of `xmodem` or `sx` to send binary files to the RC2014 CP/M `xmodem` from within picocom.

``` sh
$ sudo apt install -y picocom
$ picocom -b 115200 -f h /dev/ttyUSB0 --send-cmd "sx -vv"
```

And, finally together with VT340 emulation.
``` sh
$ xterm +u8 -geometry 132x50 -ti 340 -tn 340 -title "ReGIS" -e picocom -b 115200 -p 2 -f h /dev/ttyUSB0 --send-cmd "sx -vv"
```

Another alternative is using VT125 emulation.
``` sh
$ xterm +u8 -geometry 132x50 -ti 125 -tn 125 -title "ReGIS" -e picocom -b 115200 -p 2 -f h /dev/ttyUSB0 --send-cmd "sx -vv"
```

### Windows Subsystem for Linux

WSL1 supports connection of Serial Devices via USB. WSL2 has some issues with this, particuarly with Windows 10.

But, the supported picocom distribution for Ubuntu 22.04 Version 3.1 (for example), utilises some modern terminal capabilities that WSL does not support.
So to avoid the use of these terminal system calls we have to use an older release of picocom. For example the [Version 2.2.2 provided with Ubuntu 18.04 LTS](https://manpages.ubuntu.com/manpages/bionic/man1/picocom.1.html). Be sure to use this older version of picocom if you are using WSL to run XTerm and picocom.

The tested method of accessing XTerm on WSL is [MobaXTerm](https://mobaxterm.mobatek.net/). This enhanced terminal for Windows includes an integrated Xserver. If MobaXTerm is used to access WSL XTerm, its window will automatically connect to the Windows desktop.

Read here for a full description on [how to enable ReGIS for Windows 10 and Linux desktop machines](https://feilipu.me/2022/09/28/regis-serial-graphics-for-arduino-rc2014/).

## Demonstration

There is a demonstration program, which should produce the below result (subject to improvement).

<div>
<table style="border: 2px solid #cccccc;">
<tbody>
<tr>
<td style="border: 1px solid #cccccc; padding: 6px;"><a href="https://github.com/feilipu/z88dk-libraries/blob/master/regis/demo/regis_demo.png" target="_blank"><img src="https://github.com/feilipu/z88dk-libraries/blob/master/regis/demo/regis_demo.png"/></a></td>
</tr>
<tr>
<th style="border: 1px solid #cccccc; padding: 6px;"><centre>RC2014 ReGIS - Picocom in XTerm<center></th>
</tr>
</tbody>
</table>
</div>

This is image generated from the below ReGIS code.<br>
Expected output (where `^` is the `ESC` character).
```
^P1pS(E)W(I(M))P[600,200]V[][-200,+200]V[][400,100]W(I(G))P[700,100]V(B)[+050,][,+050][-050,](E)V(W(S1))(B)[-100,][,-050][+100,](E)V(W(S1,E))(B)[-050,][,-025][+050,](E)W(I(C))P[200,100]C(A-180)[+100]C(A+180)[+050]W(I(B))P[200,300]C(W(S1))[+100]C(W(S1,E))[+050]W(I(W))T(S02)"hello world"^\
```

## Credits

For describing [how to get XTerm working with ReGIS](https://groups.google.com/g/rc2014-z80/c/fuji5iuJ3Jc/m/FNYwGGbaAQAJ), thanks Rob Gowin.<br>
For advising on how to get [ReGIS fonts in XTerm working](https://github.com/feilipu/z88dk-libraries/commit/107c23d2f31791b0a35bfc8833a7747a84544649#diff-45390165fce0bae9bf04313a098b2a42d5d727bf90d4be040f01ec0f7fee969d), thanks Thomas Dickey.

## License

This library is licensed under [The MIT License](http://opensource.org/licenses/mit-license.php). Check the LICENSE file for more information.

Contributing to this software is warmly welcomed. You can do this by [forking](https://help.github.com/articles/fork-a-repo), committing modifications and then [pull requests](https://help.github.com/articles/using-pull-requests).
