## Introduction to the FreeRTOS functions


#### Features


## Preparation

The FreeRTOS library can be compiled using the following command lines in Linux, with the `+target` modified to be relevant to your machine.

`zcc +yaz180 -clib=new -x -SO3 --math32 @freertos.lst -o ../freertos`
`zcc +yaz180 -clib=sdcc_ix -x -SO3 --math32 --max-allocs-per-node400000 @freertos.lst -o ../freertos`
`zcc +yaz180 -clib=sdcc_iy -x -SO3 --math32 --max-allocs-per-node400000 @freertos.lst -o ../freertos`

`zcc +target -clib=new -x -SO3 --math32 @freertos.lst -o ../freertos`
`zcc +target -clib=sdcc_ix -x -SO3 --math32 --max-allocs-per-node400000 @freertos.lst -o ../freertos`
`zcc +target -clib=sdcc_iy -x -SO3 --math32 --max-allocs-per-node400000 @freertos.lst -o ../freertos`

The resulting `freertos.lib` file should be moved to `~/target/lib/newlib/sccz80` or `~/target/lib/newlib/sdcc_ix` or `~/target/lib/newlib/sdcc_iy` respectively.

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

Once installed, the FreeRTOS library can be linked against on the compile line by adding `-llib/target/freertos` and the include file can be found with `#include <lib/target/freertos.h>`.

A simple usage example, for the `+yaz180` target.

``` c

////////////////////////////////////////////////////////
////////////////////////////////////////////////////////
////    main.c
////////////////////////////////////////////////////////
////////////////////////////////////////////////////////

// zcc +yaz180 -subtype=app -clib=sdcc_iy -SO3 -v -m --list --max-allocs-per-node100000 -llib/yaz180/freertos main.c -o blink -create-app

#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>

#include <arch/yaz180.h>

/* Scheduler include files. */
#include "freertos/FreeRTOS.h"
#include "freertos/timers.h"

/*-----------------------------------------------------------*/

static void TaskBlinkRedLED(void *pvParameters);

static void TaskBlinkGreenLED(void *pvParameters);
/*-----------------------------------------------------------*/


/*-----------------------------------------------------------*/


static void TaskBlinkRedLED(void *pvParameters) 
{
    (void) pvParameters;
    TickType_t xLastWakeTime;
	/* The xLastWakeTime variable needs to be initialised with the current tick
	count.  Note that this is the only time we access this variable.  From this
	point on xLastWakeTime is managed automatically by the vTaskDelayUntil()
	API function. */
	xLastWakeTime = xTaskGetTickCount();

    for(;;)
    {

    	io_pio_port_b |= 0x20;
		vTaskDelayUntil( &xLastWakeTime, ( 500 / portTICK_PERIOD_MS ) );

    	io_pio_port_b &= 0x0F;
		vTaskDelayUntil( &xLastWakeTime, ( 500 / portTICK_PERIOD_MS ) );

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
	point on xLastWakeTime is managed automatically by the vTaskDelayUntil()
	API function. */
	xLastWakeTime = xTaskGetTickCount();

    for(;;)
    {
    	io_pio_port_b |= 0x05;
		vTaskDelayUntil( &xLastWakeTime, ( 100 / portTICK_PERIOD_MS ) );

    	io_pio_port_b &= 0xF0;
		vTaskDelayUntil( &xLastWakeTime, ( 100 / portTICK_PERIOD_MS )  );

		printf("xTaskGetTickCount %u\r\n", xTaskGetTickCount());
 		printf("GreenLED HighWater @ %u\r\n", uxTaskGetStackHighWaterMark(NULL));
    }
}

/*---------------------------------------------------------------------------*/

int main(void)
{

    io_pio_control = __IO_PIO_CNTL_00;      // enable the 82C55 for output on Port B.
    io_pio_port_b = 0x00;

    xTaskCreate(
		TaskBlinkRedLED
		,  "RedLED"
		,  256
		,  NULL
		,  3
		,  NULL ); // 

    xTaskCreate(
		TaskBlinkGreenLED
		,  "GreenLED"
		,  256
		,  NULL
		,  3
		,  NULL ); //

	vTaskStartScheduler();
}

```


## Documentation


## Licence

The FreeRTOS kernel is released under the MIT open source license, the text of which is provided below.

This license covers the FreeRTOS kernel source files, which are located in the /FreeRTOS/Source directory of the official FreeRTOS kernel download.  It also covers most of the source files in the demo application projects, which are located in the /FreeRTOS/Demo directory of the official FreeRTOS download.  The demo projects may also include third party software that is not part of FreeRTOS and is licensed separately to FreeRTOS.  Examples of third party software includes header files provided by chip or tools vendors, linker scripts, peripheral drivers, etc.  All the software in subdirectories of the /FreeRTOS directory is either open source or distributed with permission, and is free for use.  For the avoidance of doubt, refer to the comments at the top of each source file.


License text:
-------------

Copyright (C) 2020 Amazon.com, Inc. or its affiliates.  All Rights Reserved.

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

