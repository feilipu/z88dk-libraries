////////////////////////////////////////////////////////
////////////////////////////////////////////////////////
////    TaskStatus.c
////////////////////////////////////////////////////////
////////////////////////////////////////////////////////

// zcc +yaz180 -subtype=app -clib=sdcc_iy -SO3 -v -m --list --max-allocs-per-node100000 -llib/yaz180/freertos TaskStatus.c -o TaskStatus -create-app
// cat > /dev/ttyUSB0 < TaskStatus.ihx

// zcc +scz180 -subtype=hbios -clib=sdcc_iy -SO3 -v -m --list --max-allocs-per-node100000 -llib/scz180/freertos TaskStatus.c -o TaskStatus -create-app
// cat > /dev/ttyUSB0 < TaskStatus.ihx

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

//define task handles
TaskHandle_t TaskBlink_Handler;
TaskHandle_t TaskSerial_Handler;

// define two tasks for Blink & Serial
void TaskBlink( void *pvParameters );
void TaskSerial(void* pvParameters);

int main(void) {

    io_pio_control = __IO_PIO_CNTL_00;      // enable the 82C55 for output on Port B.

  // Now set up two tasks to run independently.
    xTaskCreate(
        TaskBlink
        ,  "Blink"   // A name just for humans
        ,  128  // This stack size can be checked & adjusted by reading the Stack Highwater
        ,  NULL //Parameters passed to the task function
        ,  2  // Priority, with 2 (configMAX_PRIORITIES - 1) being the highest, and 0 being the lowest.
        ,  &TaskBlink_Handler );//Task handle

    xTaskCreate(
        TaskSerial
        ,  "Serial"
        ,  128  // Stack size
        ,  NULL //Parameters passed to the task function
        ,  1  // Priority
        ,  &TaskSerial_Handler );  //Task handle

    vTaskStartScheduler();

    return 0;
}

/*--------------------------------------------------*/
/*---------------------- Tasks ---------------------*/
/*--------------------------------------------------*/

void TaskSerial(void* pvParameters){
/*
 Serial
 Send "s" or "r" through the serial port to control the suspend and resume of the LED light task.
 This example code is in the public domain.
*/
    (void) pvParameters;

    uint8_t inChar = 0;

    for (;;) { // A Task shall never return or exit.
        while( (inChar = getchar()) != 0) {
            switch(inChar){
                case 's':
                  vTaskSuspend(TaskBlink_Handler); 
                  printf("Suspend!\n");
                  break;
                case 'r':
                  vTaskResume(TaskBlink_Handler);
                  printf("Resume!\n");
                  break;
                default:
                  break;
            }
            vTaskDelay(1);
        }
    }
}

void TaskBlink(void *pvParameters)  // This is a task.
{
    (void) pvParameters;

    io_pio_port_b = 0x00;                   // on YAZ180 TIL311

    uint8_t portBstate;

    TickType_t xLastWakeTime;
    /* The xLastWakeTime variable needs to be initialised with the current tick
    count.  Note that this is the only time we access this variable.  From this
    point on xLastWakeTime is managed automatically by the xTaskDelayUntil()
    API function. */
    xLastWakeTime = xTaskGetTickCount();

    for (;;) // A Task shall never return or exit.
    {
        portBstate = 0x05;                  // YAZ180 TIL311
        io_pio_port_b = portBstate;
        xTaskDelayUntil( &xLastWakeTime, ( 500 / portTICK_PERIOD_MS ) );

        portBstate = 0x02;                  // YAZ180 TIL311
        io_pio_port_b = portBstate;
        xTaskDelayUntil( &xLastWakeTime, ( 500 / portTICK_PERIOD_MS )  );

    }
}
