## Grove Temperature Humidity TH02
------------

<img src=https://statics3.seeedstudio.com/images/product/Grove%20Tem%20Hum%20Accuracy%20Mini.jpg width=300><img src=https://statics3.seeedstudio.com/product/Grove%20Tem%20Hum%20Accuracy%20Mini_02.jpg width=300>


[Grove - Temperature & Humidity Sensor (High-Accuracy & Mini)](https://www.seeedstudio.com/Grove-Temperature%26Humidity-Sensor-%28High-Accuracy-%26-Mini%29-p-1921.html)

This is a multifunctional sensor that gives you temperature and relative humidity information at the same time. It utilizes a TH02 sensor that can meet measurement needs of general purposes. It provides reliable readings when environment humidity condition inbetween 0-80% RH, and temperature condition inbetween 0-70°C, covering needs in most home and daily applications that don't contain extreme conditions.

Compiled with sccz80 version 16894-223f580fd-20200818, and using zsdcc version 4.1.6 [r12419](https://sourceforge.net/p/sdcc/code/12419/log/?path=/trunk/sdcc).

## Installation

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

## Usage
    1.git clone this repository.
    2.use `z88dk-lib` to import into z88k
	3.open the demo `TH02_demo.c`
	4.use incantation below
	5.profit

### ZSDCC
`zcc +yaz180 -subtype=app -v -m -SO3 --list --math32_z180  -llib/yaz180/th02 --c-code-in-asm --max-allocs-per-node400000 @test.lst -o test -create-app`

### SCCZ80
`zcc +yaz180 -subtype=app -clib=new -v -m -SO3 --list --math32_z180 -llib/yaz180/th02 @test.lst -o test -create-app`

## Preparation

The library can be compiled using the following command lines in Linux, with the `+target` modified to be relevant to your machine.

```
zcc +yaz180 --math32 -x -O2 -clib=new @th02.lst -o ../th02
zcc +yaz180 --math32 -x -SO3 -clib=sdcc_ix --max-allocs-per-node400000 @th02.lst -o ../th02
zcc +yaz180 --math32 -x -SO3 -clib=sdcc_iy --max-allocs-per-node400000 @th02.lst -o ../th02
```

The resulting `th02.lib` file should be moved to `~/target/lib/newlib/sccz80` or `~/target/lib/newlib/sdcc_ix` or `~/target/lib/newlib/sdcc_iy` respectively.

## License

This demo is licensed under [The MIT License](http://opensource.org/licenses/mit-license.php). Check LICENSE for more information.

Contributing to this software is warmly welcomed. You can do this basically by 
[forking](https://help.github.com/articles/fork-a-repo), committing modifications and then [pulling requests](https://help.github.com/articles/using-pull-requests) (follow the links above for operating guide). Adding change log and your contact into file header is encouraged. Thanks for your contribution.

Seeed Studio is an open hardware facilitation company based in Shenzhen, China. Benefiting from local manufacture power and convenient global logistic system, we integrate resources to serve new era of innovation. Seeed also works with global distributors and partners to push open hardware movement.

