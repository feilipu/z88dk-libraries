/*
   Example of a FreeRTOS mutex
   https://www.freertos.org/Real-time-embedded-RTOS-mutexes.html
*/

// zcc +yaz180 -subtype=app -clib=sdcc_iy -SO3 -v -m --list --max-allocs-per-node100000 -llib/yaz180/freertos Mutex.c -o Mutex -create-app
// cat > /dev/ttyUSB0 < Mutex.ihx

// zcc +scz180 -subtype=hbios -clib=sdcc_iy -SO3 -v -m --list --max-allocs-per-node100000 -llib/scz180/freertos Mutex.c -o Mutex -create-app
// cat > /dev/ttyUSB0 < Mutex.ihx

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

// Include mutex support
#include <freertos/semphr.h>

/*
   Declaring a global variable of type SemaphoreHandle_t

*/
SemaphoreHandle_t mutex;

void TaskMutex(void* pvParameters);

int globalCount = 0;

int main() {

    /*
     Create a mutex.
     https://www.freertos.org/CreateMutex.html
    */
    mutex = xSemaphoreCreateMutex();
    if (mutex != NULL) {
    printf("Mutex Semaphore created\n");
    }

    /*
     Create tasks
    */
    xTaskCreate(TaskMutex, // Task function
          "Task1", // Task name for humans
          128, 
          1000, // Pass delay as task parameter
          1, // Task priority
          NULL);

    xTaskCreate(TaskMutex, "Task2", 128, 1000, 1, NULL);

    vTaskStartScheduler();

    return 0;
}


void TaskMutex(void *pvParameters)
{
  int delay = *((int*)pvParameters); // Use task parameters to define delay

    for (;;) {
        /**
           Take mutex
           https://www.freertos.org/a00122.html
        */
        if (xSemaphoreTake(mutex, 10) == pdTRUE) {

              printf("%s, Count read value: %i", pcTaskGetName(NULL), globalCount++);// Get task name, increment count
              printf(", Updated value: %i\n", globalCount);

              /**
                 Give mutex
                 https://www.freertos.org/a00123.html
              */
              xSemaphoreGive(mutex);
        }

        vTaskDelay(delay / portTICK_PERIOD_MS);
    }
}
