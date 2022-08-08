/*
 * Example of a basic FreeRTOS queue
 * https://www.freertos.org/Embedded-RTOS-Queues.html
 */

// zcc +yaz180 -subtype=app -clib=sdcc_iy -SO3 -v -m --list --max-allocs-per-node100000 -llib/yaz180/freertos StructQueue.c -o StructQueue -create-app
// cat > /dev/ttyUSB0 < StructQueue.ihx

// zcc +scz180 -subtype=hbios -clib=sdcc_iy -SO3 -v -m --list --max-allocs-per-node100000 -llib/scz180/freertos StructQueue.c -o StructQueue -create-app
// cat > /dev/ttyUSB0 < StructQueue.ihx

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

// Include queue support
#include <freertos/queue.h>

// Define a struct
struct portRead {
  uint8_t port;
  uint8_t value;
};

//Function Declaration
void TaskBlink(void *pvParameters);
void TaskReadPortA(void *pvParameters);
void TaskReadPortB(void *pvParameters);
void TaskSerial(void *pvParameters);

/* 
 * Declaring a global variable of type QueueHandle_t 
 * 
 */
QueueHandle_t structQueue;

int main() {

#if __YAZ180
    io_pio_control = __IO_PIO_CNTL_00;      // enable the 82C55 for output on Port B.
#endif

    /**
    * Create a queue.
    * https://www.freertos.org/a00116.html
    */
    structQueue = xQueueCreate(10,  // Queue length
        sizeof(struct portRead));   // Queue item size

    if (structQueue != NULL) {

        // Create task that consumes the queue if it was created.
        xTaskCreate(TaskSerial, // Task function
            "Serial", // A name just for humans
            128,  // This stack size can be checked & adjusted by reading the Stack Highwater
            NULL, 
            2, // Priority, with 3 (configMAX_PRIORITIES - 1) being the highest, and 0 being the lowest.
            NULL);


        // Create task that publish data in the queue if it was created.
        xTaskCreate(TaskReadPortA, // Task function
            "ReadPort0", // Task name
            128,  // Stack size
            NULL, 
            1, // Priority
            NULL);

        // Create other task that publish data in the queue if it was created.
        xTaskCreate(TaskReadPortB, // Task function
            "ReadPort1", // Task name
            128,  // Stack size
            NULL, 
            1, // Priority
            NULL);
        }

        xTaskCreate(TaskBlink, // Task function
            "Blink", // Task name
            128, // Stack size 
            NULL, 
            0, // Priority
            NULL );

    vTaskStartScheduler();

    return 0;
}


/**
 * Read task for Port A
 * Reads an input on port A and send the read value through the queue.
 */
void TaskReadPortA(void *pvParameters)
{
    (void) pvParameters;
    struct portRead currentPortRead;

    for (;;) {
        // Read the input on port 0:

        currentPortRead.port= 0;

#if __YAZ180
        // Read the input on port A
        currentPortRead.value = io_pio_port_a;
#elif __SCZ180
        // Read the input on DIO
        currentPortRead.value = io_dio&0x0F;
#endif

        /**
         * Post an item on a queue.
         * https://www.freertos.org/a00117.html
         */
        xQueueSend(structQueue, &currentPortRead, portMAX_DELAY);

        vTaskDelay(200);
    }
}


/**
 * Read task for Port B
 * Reads an input on port B and send the read value through the queue.
 */
void TaskReadPortB(void *pvParameters)
{
    (void) pvParameters;
    struct portRead currentPortRead;

    for (;;) {
        // Read the input on port 1:

        currentPortRead.port= 1;
        
#if __YAZ180
        // Read the input on port B
        currentPortRead.value = io_pio_port_b;
#elif __SCZ180
        // Read the input on DIO
        currentPortRead.value = (io_dio&0xF0) >> 4;
#endif

        /**
         * Post an item on a queue.
         * https://www.freertos.org/a00117.html
         */
        xQueueSend(structQueue, &currentPortRead, portMAX_DELAY);

        vTaskDelay(100);
    }
}

/**
 * Serial task.
 * Prints the received items from the queue to the serial monitor.
 */
void TaskSerial(void * pvParameters) {
  (void) pvParameters;

    for (;;) {

        struct portRead currentPortRead;

        /**
         * Read an item from a queue.
         * https://www.freertos.org/a00118.html
         */
        if (xQueueReceive(structQueue, &currentPortRead, portMAX_DELAY) == pdPASS) {
            printf("Port: %i", currentPortRead.port);
            printf(" Value: %i\n",currentPortRead.value);
        }
    }
}

/* 
 * Blink task. 
 */
void TaskBlink(void *pvParameters) { // This is a task.

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
