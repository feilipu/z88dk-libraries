////////////////////////////////////////////////////////
////////////////////////////////////////////////////////
////    main.c
////////////////////////////////////////////////////////
////////////////////////////////////////////////////////

// zcc +yaz180 -subtype=app -clib=sdcc_iy -SO3 -v -m --list --max-allocs-per-node100000 -llib/yaz180/freertos main.c -o blink -create-app

// zcc +scz180 -subtype=hbios -clib=sdcc_iy -SO3 -v -m --list --max-allocs-per-node100000 -llib/scz180/freertos main.c -o blink -create-app

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

//      io_pio_port_b |= 0x20;              // YAZ180 TIL311
        vTaskDelayUntil( &xLastWakeTime, ( 400 / portTICK_PERIOD_MS ) );

//      io_pio_port_b &= 0x0F;              // YAZ180 TIL311
        vTaskDelayUntil( &xLastWakeTime, ( 100 / portTICK_PERIOD_MS ) );

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
//      io_pio_port_b |= 0x05;              // YAZ180 TIL311
        vTaskDelayUntil( &xLastWakeTime, ( 200 / portTICK_PERIOD_MS ) );

//      io_pio_port_b &= 0xF0;              // YAZ180 TIL311
        vTaskDelayUntil( &xLastWakeTime, ( 100 / portTICK_PERIOD_MS )  );

        printf("xTaskGetTickCount %u\r\n", xTaskGetTickCount());
        printf("GreenLED HighWater @ %u\r\n", uxTaskGetStackHighWaterMark(NULL));
    }
}

/*---------------------------------------------------------------------------*/

int main(void)
{

//  io_pio_control = __IO_PIO_CNTL_00;      // enable the 82C55 for output on Port B.
//  io_pio_port_b = 0x00;                   // on YAZ180 TIL311

    xTaskCreate(
        TaskBlinkRedLED
        ,  "RedLED"
        ,  128
        ,  NULL
        ,  3
        ,  NULL ); // 

    xTaskCreate(
        TaskBlinkGreenLED
        ,  "GreenLED"
        ,  128
        ,  NULL
        ,  3
        ,  NULL ); //

    vTaskStartScheduler();
    
    return 0;
}
