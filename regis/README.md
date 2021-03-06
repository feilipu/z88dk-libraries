## ReGIS - Remote Graphics Instruction Set
------------

ReGIS interprets commands that allow you to simply and efficiently control a video monitor screen and draw pictures on the screen with lines, curves, and circles using a serial interface (USART). Also, ReGIS provides commands to include scalable text characters in pictures. The ReGIS graphics language is designed for conciseness and easy transport of code from the host to the ReGIS device. The language consists of commands that are modified by options.

Compiled with sccz80 version 16894-223f580fd-20200818, and using zsdcc version 4.1.6 r12419.

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
`zcc +rc2014 -subtype=cpm -v -m --list -llib/rc2014/regis --max-allocs-per-node100000 regis_demo.c -o regis -create-app`

### SCCZ80
`zcc +rc2014 -subtype=cpm -clib=new -v -m -O2 --list -llib/rc2014/regis regis_demo.c -o regis -create-app`

## Preparation

The library can be compiled using the following command lines in Linux, with the `+target` (eg. `+rc2014`) modified to be relevant to your machine.

```
zcc +rc2014 --math32 -x -O2 -clib=new @regis.lst -o ../regis
zcc +rc2014 --math32 -x -SO3 -clib=sdcc_ix --max-allocs-per-node400000 @regis.lst -o ../regis
zcc +rc2014 --math32 -x -SO3 -clib=sdcc_iy --max-allocs-per-node400000 @regis.lst -o ../regis
```

The resulting `regis.lib` files should be moved to `~/target/lib/newlib/sccz80` or `~/target/lib/newlib/sdcc_ix` or `~/target/lib/newlib/sdcc_iy` respectively.

## Preparing XTERM to support ReGIS

XTERM is the only known software solution supporting ReGIS commands (to be improved I'm sure). But it doesn't support ReGIS in the default build. You'll need to enable ReGIS yourself.
``` sh
% sudo apt install -y libxaw7-dev libncurses-dev libxft-dev
% wget https://invisible-island.net/datafiles/release/xterm.tar.gz
% tar xf xterm.tar.gz
% cd xterm-368
% ./configure --enable-regis-graphics
% make
% sudo make install
```

As XTERM has no serial interface itself, so you'll need to use one. A suggestion is to use picocom.<br>
It is also useful for working with retrocomputers generally. First test that your installation is working as per below.

Adding the `--send-cmd` option will allow the use of `xmodem` or `sx` to send binary files to the RC2014 CP/M `xmodem` from within picocom.

``` sh
% sudo apt install -y picocom
% picocom -b 115200 -f h /dev/ttyUSB0 --send-cmd "sx -vv"
```

And, finally together with VT340 emulation.
``` sh
xterm +u8 -geometry 132x50 -ti 340 -tn 340 -e picocom -b 115200 -f h /dev/ttyUSB0 --send-cmd "sx -vv"
```

Another alternative is using VT125 emulation.
``` sh
xterm +u8 -geometry 132x50 -ti 125 -tn 125 -e picocom -b 115200 -f h /dev/ttyUSB0 --send-cmd "sx -vv"
```

## Demonstration

There is a demonstration program, which should produce the below result (subject to improvement).

<div>
<table style="border: 2px solid #cccccc;">
<tbody>
<tr>
<td style="border: 1px solid #cccccc; padding: 6px;"><a href="https://github.com/feilipu/z88dk-libraries/blob/master/regis/demo/regis_demo.png" target="_blank"><img src="https://github.com/feilipu/z88dk-libraries/blob/master/regis/demo/regis_demo.png"/></a></td>
</tr>
<tr>
<th style="border: 1px solid #cccccc; padding: 6px;"><centre>RC2014 ReGIS - Picocom in XTERM<center></th>
</tr>
</tbody>
</table>
</div>

This is image generated from the below ReGIS code.<br>
Expected output (where `^` is the `ESC` character).
```
^P1pS(E)W(I(M))P[600,200]V[][-200,+200]V[][400,100]W(I(G))P[700,100]V(B)[+050,][,+050][-050,](E)V(W(S1))(B)[-100,][,-050][+100,](E)V(W(S1,E))(B)[-050,][,-025][+050,](E)W(I(C))P[200,100]C(A-180)[+100]C(A+180)[+050]W(I(B))P[200,300]C(W(S1))[+100]C(W(S1,E))[+050]W(I(W))T(S01)"hello world"^\
```

## Credits

For describing [how to get XTERM working with ReGIS](https://groups.google.com/g/rc2014-z80/c/fuji5iuJ3Jc/m/FNYwGGbaAQAJ), thanks Rob Gowin.<br>
For advising on how to get [ReGIS fonts in XTERM working](https://github.com/feilipu/z88dk-libraries/commit/107c23d2f31791b0a35bfc8833a7747a84544649#diff-45390165fce0bae9bf04313a098b2a42d5d727bf90d4be040f01ec0f7fee969d), thanks Thomas Dickey.

## License

This library is licensed under [The MIT License](http://opensource.org/licenses/mit-license.php). Check the LICENSE file for more information.

Contributing to this software is warmly welcomed. You can do this by [forking](https://help.github.com/articles/fork-a-repo), committing modifications and then [pull requests](https://help.github.com/articles/using-pull-requests).

