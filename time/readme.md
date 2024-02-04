## Introduction to the Z80 Time functions

This file describes the time functions implemented in Z80 time library

The implementation aspires to conform with ISO/IEC 9899 (C90). However, due to limitations of the target processor and the nature of its development environment, a practical implementation must of necessity deviate from the standard.

+ __Section 7.23.2.1__ `clock()`
The type `clock_t`, the macro `CLOCKS_PER_SEC`, and the function `clock()` are not implemented. We consider these items belong to operating system code, or to application code when no operating system is present. In z88dk `__CPU_CLOCK` is defined.

+ __Section 7.23.2.3__ `mktime()`
The standard specifies that `mktime()` should return `(time_t)-1`, if the time cannot be represented. This implementation always returns a 'best effort' representation.

+ __Section 7.23.2.4__ `time()`
The standard specifies that `time()` should return `(time_t)-1`, if the time is not available. Since the application must initialize the time system, this functionality is not implemented.

+ __Section 7.23.2.2__ `difftime()`
Due to the lack of a 64 bit double, the function `difftime()` returns a long integer. In most cases this change will be invisible to the user, handled automatically by the compiler.

+ __Section 7.23.1.4__ `struct tm`
Per the standard, `struct tm->tm_isdst` is greater than zero when Daylight Saving time is in effect. This implementation further specifies that, when positive, the value of `tm_isdst` represents the amount time is advanced during Daylight Saving time.

+ __Section 7.23.3.5__ `strftime()`
Only the 'C' locale is supported, therefore the modifiers 'E' and 'O' are ignored. The 'Z' conversion is also ignored, due to the lack of time zone name.


In addition to the above departures from the standard, there are some behaviors which are different from what is often expected, though allowed under the standard.

There is no 'platform standard' method to obtain the current time, time zone, or daylight savings 'rules' in the z80 environment. Therefore the application must initialize the time system with this information. The functions `set_zone()`, `set_dst()`, and `set_system_time()` are provided for initialization. Once initialized, system time is normally maintained by calling the system function `system_tick()` at one second intervals with a hardware interrupt provided by a Programmable Reload Timer or external clock.

Though not specified in the standard, it is often expected that `time_t` is a signed integer representing an offset in seconds from Midnight Jan 1 1970... i.e. 'Unix time'. This implementation uses an unsigned 32 bit integer offset from Midnight Jan 1 2000. The use of this 'epoch' helps to simplify the conversion functions, while the 32 bit value allows time to be properly represented until Tue Feb 7 06:28:15 2136 UTC. The macros `UNIX_OFFSET` and `NTP_OFFSET` are defined to assist in converting to and from Unix and NTP time stamps.

Unlike desktop counterparts, it is impractical to implement or maintain the 'zoneinfo' database. Therefore no attempt is made to account for time zone, daylight saving, or leap seconds in past dates. All calculations are made according to the currently configured time zone and daylight saving 'rule'.

In addition to C standard functions, re-entrant versions of `ctime()`, `asctime()`, `gmtime()` and `localtime()` are provided which, in addition to being re-entrant, have the property of claiming less permanent storage in RAM. An additional time conversion, `isotime()`, and its re-entrant version, uses far less storage than either `ctime()` or `asctime()`.

Along with the usual smattering of utility functions, such as `is_leap_year()`, this library includes a set of functions related the sun and moon, as well as sidereal time functions.

Compiled with sccz80 version 22168-da9bcc8cc0-20240130, and using zsdcc version 4.4.0 [r14648](https://sourceforge.net/p/sdcc/code/14648/log/?path=/trunk/sdcc).

## Installation

The `z88dk-lib` function is used to install for the desired target. e.g.

```bash
cd ..
z88dk-lib +target time
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

Once installed, the time library can be linked against on the compile line by adding `-llib/target/time` and the include file can be found with `#include <lib/target/time.h>`.

A simple usage example, for the `+yaz180` target.

```c
#include <stdlib.h>
#include <stdio.h>

#include <arch/yaz180.h>
#include <arch/yaz180/system_time.h>

#include <lib/yaz180/time.h>

// zcc +yaz180 -subtype=app -clib=sdcc_iy -SO3 --max-allocs-per-node200000 --opt-code-size -v --list -m -llib/yaz180/time @atest.lst -o time_app -create-app


uint16_t i;

struct tm CurrTimeDate;         // set up an array for the RTC info.
time_t theTime;
char timeStore[26];

void main(void)
{
    system_tick_init((void *)0x2044);

    set_zone((int32_t)10 * ONE_HOUR);       // Australian Eastern Standard Time
    set_system_time(1505695200 - UNIX_OFFSET);

    while(1)
    {
        time(&theTime);

        gmtime_r(&theTime, &CurrTimeDate);
        isotime_r(&CurrTimeDate, timeStore);
        printf("ISO Time: %s", timeStore);

        localtime_r(&theTime, &CurrTimeDate);
        asctime_r(&CurrTimeDate, timeStore);
        printf("    AEST Time: %s\r\n", timeStore);

//      printf("SYSTEM Time: %s\r\n", ctime( (time_t *)&theTime));  // all in one line
    }
}
```

## Internals

`time_t __system_time` represents seconds elapsed from Midnight, Jan 1 2000 UTC (the Y2K 'epoch').
Its range allows this implementation to represent time up to Tue Feb 7 06:28:15 2136 UTC.

`uint8_t __system_time_fraction` represents fractions of a second and when rolled over, leads to an increment of the `__system_time` variable. This depends on the specific machine implemenation.

The `tm` structure contains a representation of time 'broken down' into components of the
Gregorian calendar.

The normal ranges of the elements are..
```c
    tm_sec      seconds after the minute - [ 0 to 59 ]
    tm_min      minutes after the hour - [ 0 to 59 ]
    tm_hour     hours since midnight - [ 0 to 23 ]
    tm_mday     day of the month - [ 1 to 31 ]
    tm_wday     days since Sunday - [ 0 to 6 ]
    tm_mon      months since January - [ 0 to 11 ]
    tm_year     years since 2000
    tm_yday     days since January 1 - [ 0 to 365 ]
    tm_isdst    Daylight Saving Time flag *
```
*The value of `tm_isdst` is zero if Daylight Saving Time is not in effect, and is negative if
the information is not available.

When Daylight Saving Time is in effect, the value represents the number of
seconds the clock is advanced.

See the `set_dst()` function for more information about Daylight Saving.

## Preparation

The library can be compiled using the following command lines in Linux, with the `+target` modified to be relevant to your machine.

```
zcc +target -clib=new -x -O2 --opt-code-speed=all --math32 @time.lst -o ../time
zcc +target -clib=sdcc_ix -x -SO3 --max-allocs-per-node400000 --math32 @time.lst -o ../time
zcc +target -clib=sdcc_iy -x -SO3 --max-allocs-per-node400000 --math32 @time.lst -o ../time
```

The resulting `time.lib` file should be moved to `~/target/lib/newlib/sccz80` or `~/target/lib/newlib/sdcc_ix` or `~/target/lib/newlib/sdcc_iy` respectively.

## Licence

(C)2012 Michael Duane Rice All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer. Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution. Neither the name of the copyright holders nor the names of contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
