/*
 * Example of FreeRTOS task utilities
 * https://www.freertos.org/a00021.html
 */

// zcc +yaz180 -subtype=app -clib=sdcc_iy -SO3 -v -m --list --max-allocs-per-node100000 -llib/yaz180/freertos TaskUtilities.c -o TaskUtilities -create-app
// cat > /dev/ttyUSB0 < TaskUtilities.ihx

// zcc +scz180 -subtype=hbios -clib=sdcc_iy -SO3 -v -m --list --max-allocs-per-node100000 -llib/scz180/freertos TaskUtilities.c -o TaskUtilities -create-app
// cat > /dev/ttyUSB0 < TaskUtilities.ihx

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

/**
 * Task handles
 * https://www.freertos.org/a00019.html#xTaskHandle
 */
TaskHandle_t taskBlinkHandle;
TaskHandle_t taskDeletedHandle;
TaskHandle_t taskBlockedHandle;

// define four tasks
void TaskBlink( void *pvParameters );
void TaskSerial(void* pvParameters);
void TaskDeleted(void* pvParameters);
void TaskBlocked(void* pvParameters);

int main(void) {

#if __YAZ180
    io_pio_control = __IO_PIO_CNTL_00;      // enable the 82C55 for output on Port B.
#endif

    /**
     * Task creation
     */
    xTaskCreate(TaskBlink, // Task function
        "Blink", // Task name
        64, // Stack size 
        NULL, 
        0, // Priority
        &taskBlinkHandle); // Task handler

    xTaskCreate(TaskSerial,
        "Serial",
        128,
        NULL, 
        2,
        NULL);

    xTaskCreate(TaskDeleted,
        "Deleted",
        32,
        NULL, 
        1,
        &taskDeletedHandle);

    xTaskCreate(TaskBlocked,
        "Blocked",
        32,
        NULL, 
        1,
        &taskBlockedHandle);

    vTaskStartScheduler();

    return 0;
}

/**
 * Example of utilities usage
 */
void TaskSerial(void *pvParameters)
{

  (void) pvParameters;

    for (;;) {
        printf("======== Tasks status ========\n");
        printf("Tick count: %u",  xTaskGetTickCount() );
        printf(", Task count: %u\n\n", uxTaskGetNumberOfTasks() );

        // Serial task status
        printf("- TASK %s", pcTaskGetName(NULL) ); // Get task name without handler https://www.freertos.org/a00021.html#pcTaskGetName
        printf(", High Watermark: %i\n",uxTaskGetStackHighWaterMark(NULL) ); // https://www.freertos.org/uxTaskGetStackHighWaterMark.html 

        TaskHandle_t taskSerialHandle = xTaskGetCurrentTaskHandle(); // Get current task handle. https://www.freertos.org/a00021.html#xTaskGetCurrentTaskHandle

        printf("- TASK %s",pcTaskGetName(taskBlinkHandle) ); // Get task name with handler
        printf(", High Watermark: %i\n\n", uxTaskGetStackHighWaterMark(taskBlinkHandle) );

        printf("- TASK %s", pcTaskGetName(taskDeletedHandle) );
        printf(", High Watermark: %i\n", uxTaskGetStackHighWaterMark(taskDeletedHandle) );

        printf("- TASK %s", pcTaskGetName(taskBlockedHandle) );
        printf(", High Watermark: %i\n\n", uxTaskGetStackHighWaterMark(taskBlockedHandle) );

        vTaskDelay( 5000 / portTICK_PERIOD_MS );
    }
}

/**
 * Blocked tasks when run
 */
void TaskBlocked(void *pvParameters)
{
    (void) pvParameters;
    for (;;) {
        vTaskDelay( 90000 / portTICK_PERIOD_MS );
    }
}

/**
 * Deleted tasks when run
 */
void TaskDeleted(void *pvParameters)
{
    (void) pvParameters;

    vTaskDelay( 20000 / portTICK_PERIOD_MS ); // wait 20 seconds before deleting.

    vTaskDelete(NULL);
}

/* 
 * Blink task. 
 */
void TaskBlink(void *pvParameters)  // This is a task.
{
    (void) pvParameters;

#if __YAZ180
    uint8_t portBstate = 0x00;
    io_pio_port_b = 0x00;                   // YAZ180 TIL311
#elif __SCZ180
    uint8_t io_led_state = 0x00;
    io_led_status = 0x00;                   // SCZ180 Status LED
#endif

    TickType_t xLastWakeTime;
    /* The xLastWakeTime variable needs to be initialised with the current tick
    count.  Note that this is the only time we access this variable.  From this
    point on xLastWakeTime is managed automatically by the xTaskDelayUntil()
    API function. */
    xLastWakeTime = xTaskGetTickCount();

    for (;;) // A Task shall never return or exit.
    {
#if __YAZ180
        portBstate = 0x05;                  // YAZ180 TIL311
        io_pio_port_b = portBstate;
        xTaskDelayUntil( &xLastWakeTime, ( 500 / portTICK_PERIOD_MS ) );

        portBstate = 0x02;                  // YAZ180 TIL311
        io_pio_port_b = portBstate;
        xTaskDelayUntil( &xLastWakeTime, ( 500 / portTICK_PERIOD_MS )  );

#elif __SCZ180
        io_led_state ^= 0x01;               // SCZ180 Status LED
        io_led_status = io_led_state;
        xTaskDelayUntil( &xLastWakeTime, ( 500 / portTICK_PERIOD_MS ) );

        io_led_state ^= 0x01;               // SCZ180 Status LED
        io_led_status = io_led_state;
        xTaskDelayUntil( &xLastWakeTime, ( 500 / portTICK_PERIOD_MS ) );

#else
        xTaskDelayUntil( &xLastWakeTime, ( 1000 / portTICK_PERIOD_MS ) );
#endif
    }
}
