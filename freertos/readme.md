## Introduction to the FreeRTOS functions

Developed in partnership with the world’s leading chip companies over a 15 year period, FreeRTOS is a market-leading real–time operating system (RTOS) for microcontrollers and small microprocessors. Distributed freely under the MIT open source license, FreeRTOS includes a kernel and a growing set of libraries suitable for use across all industry sectors. FreeRTOS is built with an emphasis on reliability, accessibility, and ease of use.

Current source version status is: 11.1.0, release 22 April 2024.

Over the past few years freeRTOS development has become increasingly 32-bit orientated, with little change or improvement for the 8-bit world. As such I'm treating this 22 April 2024 11.1.0 release as my LTS release.

Compiled with sccz80 version: 22309-5b13a4917f-20240417, and using zsdcc version 4.4.0 [r14648](https://sourceforge.net/p/sdcc/code/14648/log/?path=/trunk/sdcc).

## Installation

The `z88dk-lib` function is used to install for the desired target. e.g.

```bash
cd ..
z88dk-lib +target -f freertos
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

Once installed, the FreeRTOS library can be linked against on the compile line by adding `-llib/target/freertos` and the include files can be found with `#include <freertos/FreeRTOS.h>`, for example.

A simple usage example, for the `+scz180` target.

``` c
////////////////////////////////////////////////////////
////////////////////////////////////////////////////////
////    main.c
////////////////////////////////////////////////////////
////////////////////////////////////////////////////////

// zcc +yaz180 -subtype=app -clib=sdcc_iy -SO3 --max-allocs-per-node100000 -v -m --list -llib/yaz180/freertos main.c -o blink -create-app

// zcc +scz180 -subtype=hbios -clib=sdcc_iy -SO3 --max-allocs-per-node100000 -v -m --list -llib/scz180/freertos main.c -o blink -create-app

#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>

#if __YAZ180
#include <arch/yaz180.h>
#elif __SCZ180
#include <arch/scz180.h>
#endif

/* Scheduler include files. */
#include <freertos/FreeRTOS.h>
#include <freertos/timers.h>

/*-----------------------------------------------------------*/

static void TaskBlinkRedLED(void *pvParameters);
static void TaskBlinkGreenLED(void *pvParameters);

#if __YAZ180
static uint8_t portBstate;
#endif

#if __SCZ180
static uint8_t io_dio_state;
static uint8_t io_led_state;
#endif

/*-----------------------------------------------------------*/

static void TaskBlinkRedLED(void *pvParameters)
{
    (void) pvParameters;
    TickType_t xLastWakeTime;
    /* The xLastWakeTime variable needs to be initialised with the current tick
    count.  Note that this is the only time we access this variable.  From this
    point on xLastWakeTime is managed automatically by the xTaskDelayUntil()
    API function. */
    xLastWakeTime = xTaskGetTickCount();

    for(;;)
    {
#if __YAZ180
        portBstate |= 0x20;                 // YAZ180 TIL311
        io_pio_port_b = portBstate;
        xTaskDelayUntil( &xLastWakeTime, ( 400 / portTICK_PERIOD_MS ) );

        portBstate |= 0x50;                 // YAZ180 TIL311
        io_pio_port_b = portBstate;
        xTaskDelayUntil( &xLastWakeTime, ( 100 / portTICK_PERIOD_MS ) );

#elif __SCZ180
        io_dio_state ^= 0x01;               // SCZ180 DIO
        io_dio = io_dio_state;
        xTaskDelayUntil( &xLastWakeTime, ( 200 / portTICK_PERIOD_MS ) );

        io_dio_state ^= 0x01;               // SCZ180 DIO
        io_dio = io_dio_state;
        xTaskDelayUntil( &xLastWakeTime, ( 100 / portTICK_PERIOD_MS ) );

#else
        xTaskDelayUntil( &xLastWakeTime, ( 500 / portTICK_PERIOD_MS ) );

#endif
        printf("RedLED HighWater @ %u\r\n", uxTaskGetStackHighWaterMark(NULL));
    }

}

/*-----------------------------------------------------------*/
static void TaskBlinkGreenLED(void *pvParameters)
{
    (void) pvParameters;
    TickType_t xLastWakeTime;
    /* The xLastWakeTime variable needs to be initialised with the current tick
    count.  Note that this is the only time we access this variable.  From this
    point on xLastWakeTime is managed automatically by the xTaskDelayUntil()
    API function. */
    xLastWakeTime = xTaskGetTickCount();

    for(;;)
    {
#if __YAZ180
        portBstate |= 0x05;                 // YAZ180 TIL311
        io_pio_port_b = portBstate;
        xTaskDelayUntil( &xLastWakeTime, ( 200 / portTICK_PERIOD_MS ) );

        portBstate |= 0x02;                 // YAZ180 TIL311
        io_pio_port_b = portBstate;
        xTaskDelayUntil( &xLastWakeTime, ( 100 / portTICK_PERIOD_MS )  );

#elif __SCZ180
        io_led_state ^= 0x01;               // SCZ180 Status LED
        io_led_status = io_led_state;
        xTaskDelayUntil( &xLastWakeTime, ( 200 / portTICK_PERIOD_MS ) );

        io_led_state ^= 0x01;               // SCZ180 Status LED
        io_led_status = io_led_state;
        xTaskDelayUntil( &xLastWakeTime, ( 100 / portTICK_PERIOD_MS ) );

#else
        xTaskDelayUntil( &xLastWakeTime, ( 300 / portTICK_PERIOD_MS ) );
#endif
        printf("xTaskGetTickCount %u\r\n", xTaskGetTickCount());
        printf("GreenLED HighWater @ %u\r\n", uxTaskGetStackHighWaterMark(NULL));
    }
}

/*---------------------------------------------------------------------------*/

int main(void)
{

#if __YAZ180
    io_pio_control = __IO_PIO_CNTL_00;      // enable the 82C55 for output on Port B.
    io_pio_port_b = 0x00;                   // on YAZ180 TIL311
#endif

    xTaskCreate(
        TaskBlinkRedLED
        ,  "RedLED"
        ,  96
        ,  NULL
        ,  3
        ,  NULL ); //

    xTaskCreate(
        TaskBlinkGreenLED
        ,  "GreenLED"
        ,  96
        ,  NULL
        ,  3
        ,  NULL ); //

    vTaskStartScheduler();

    return 0;
}
```

## Preparation

The FreeRTOS library can be compiled using the following command lines in Linux, with the `+target` modified to be relevant to your machine.

```
zcc +yaz180 -clib=new -x -O2 --opt-code-speed=all --math32 @freertos.lst -o ../freertos
zcc +yaz180 -clib=sdcc_ix -x -SO3 --max-allocs-per-node400000 --math32 @freertos.lst -o ../freertos
zcc +yaz180 -clib=sdcc_iy -x -SO3 --max-allocs-per-node400000 --math32 @freertos.lst -o ../freertos
```

```
zcc +scz180 -clib=new -x -O2 --opt-code-speed=all --math32 @freertos.lst -o ../freertos
zcc +scz180 -clib=sdcc_ix -x -SO3 --max-allocs-per-node400000 --math32 @freertos.lst -o ../freertos
zcc +scz180 -clib=sdcc_iy -x -SO3 --max-allocs-per-node400000 --math32 @freertos.lst -o ../freertos
```

The resulting `freertos.lib` file should be moved to `~/target/lib/newlib/sccz80` or `~/target/lib/newlib/sdcc_ix` or `~/target/lib/newlib/sdcc_iy` respectively.

## Documentation

The canonical source for information is the [FreeRTOS Web Site](https://www.freertos.org/).
Within this site, the [Getting Started](https://www.freertos.org/FreeRTOS-quick-start-guide.html) page is very useful.

The AVRfreeRTOS Repository has plenty of examples, ranging from [Blink](https://github.com/feilipu/avrfreertos/blob/master/MegaBlink/main.c) through to a [Synthesiser](https://github.com/feilipu/avrfreertos/tree/master/GA_Synth) for the Goldilocks Analogue.

## Licence

The FreeRTOS kernel is released under the MIT open source license, the text of which is provided below.

This license covers the FreeRTOS kernel source files, which are located in the /FreeRTOS/Source directory of the official FreeRTOS kernel download.  It also covers most of the source files in the demo application projects, which are located in the /FreeRTOS/Demo directory of the official FreeRTOS download.  The demo projects may also include third party software that is not part of FreeRTOS and is licensed separately to FreeRTOS.  Examples of third party software includes header files provided by chip or tools vendors, linker scripts, peripheral drivers, etc.  All the software in subdirectories of the /FreeRTOS directory is either open source or distributed with permission, and is free for use.  For the avoidance of doubt, refer to the comments at the top of each source file.

### License text

Copyright (C) 2024 Amazon.com, Inc. or its affiliates.  All Rights Reserved.

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
