/*
 * Example of a basic FreeRTOS queue
 * https://www.freertos.org/Embedded-RTOS-Queues.html
 */

// zcc +yaz180 -subtype=app -clib=sdcc_iy -SO3 -v -m --list --max-allocs-per-node100000 -llib/yaz180/freertos StructArray.c -o StructArray -create-app
// cat > /dev/ttyUSB0 < StructArray.ihx

// zcc +scz180 -subtype=hbios -clib=sdcc_iy -SO3 -v -m --list --max-allocs-per-node100000 -llib/scz180/freertos StructArray.c -o StructArray -create-app
// cat > /dev/ttyUSB0 < StructArray.ihx

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

#if __SCZ180
static uint8_t io_dio_state;
static uint8_t io_led_state;
#endif

// Define a Structure Array
struct myArray{
  uint8_t pin[2];
  uint8_t ReadValue[2];
};

//Function Declaration
void TaskBlink(void *pvParameters);
void POT(void *pvParameters);
void TaskSerial(void *pvParameters);

/* 
 * Declaring a global variable of type QueueHandle_t 
 * 
 */
QueueHandle_t structArrayQueue;

int main(void) {

#if __YAZ180
    io_pio_control = __IO_PIO_CNTL_00;      // enable the 82C55 for output on Port B.
#endif

    /**
    * Create a queue.
    * https://www.freertos.org/a00116.html
    */
    structArrayQueue=xQueueCreate(10,   //Queue length
        sizeof(struct myArray));        //Queue item size
                              
    if(structArrayQueue!=NULL) {

        xTaskCreate(TaskBlink,  // Task function
            "Blink",// Task name
            128,// Stack size 
            NULL,
            0,// Priority
            NULL);

        // Create other task that publish data in the queue if it was created.
        xTaskCreate(POT,// Task function
            "AnalogRead",// Task name
            128,  // Stack size
            NULL,
            2,// Priority
            NULL);

        // Create task that consumes the queue if it was created.
        xTaskCreate(TaskSerial,// Task function
            "PrintSerial",// A name just for humans
            128,// This stack size can be checked & adjusted by reading the Stack Highwater
            NULL,
            1, // Priority, with configMAX_PRIORITIES - 1 being the highest, and 0 being the lowest.
            NULL);
    }

    vTaskStartScheduler();

    return 0;
}


/**
 * Analog read task for Pin A0 and A1
 * Reads an digital input on pin 0 and pin 1 
 * Send the read value through the queue.
 */
void POT(void *pvParameters) {
    (void) pvParameters;

    for (;;){
    
        // Read the input on Port B:
        struct myArray currentVariable;

        currentVariable.pin[0]=0;
        currentVariable.pin[1]=1;

#if __YAZ180
        // Read the input on port B
        uint8_t sensorValue = io_pio_port_b;
#elif __SCZ180
        // Read the input on DIO
        uint8_t sensorValue = io_dio;
#endif

        currentVariable.ReadValue[0]=sensorValue&0x01;
        currentVariable.ReadValue[1]=(sensorValue&0x02) >> 1;

        /**
        * Post an item on a queue.
        * https://www.freertos.org/a00117.html
        */
        xQueueSend(structArrayQueue,&currentVariable,portMAX_DELAY);

        // 100 tick delay in between reads for stability
        vTaskDelay(100);
    }
}

/**
 * Serial task.
 * Prints the received items from the queue to the serial monitor.
 */
void TaskSerial(void *pvParameters) {
    (void) pvParameters;

    for (;;){

        struct myArray currentVariable;

        /**
         * Read an item from a queue.
         * https://www.freertos.org/a00118.html
         */
        if(xQueueReceive(structArrayQueue,&currentVariable,portMAX_DELAY) == pdPASS ) {
            for(int i=0;i<2;i++){
                printf("PIN: %i", currentVariable.pin[i]);
                printf(" Value: %i\n", currentVariable.ReadValue[i]);
            }
        }
    }
}

/* 
 * Blink task. 
 */
void TaskBlink(void *pvParameters) { // This is a task.

    (void) pvParameters;

#if __YAZ180
    io_pio_port_b = 0x00;                   // YAZ180 TIL311
#elif __SCZ180
    io_led_status = 0x00;                   // SCZ180 Status LED
#endif

    uint8_t portBstate;

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

        printf("xTaskGetTickCount %u\r\n", xTaskGetTickCount());
        printf("GreenLED HighWater @ %u\r\n", uxTaskGetStackHighWaterMark(NULL));

    }
}
