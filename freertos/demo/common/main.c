/*
 * FreeRTOS V202104.00
 * Copyright (C) 2020 Amazon.com, Inc. or its affiliates.  All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 * the Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * https://www.FreeRTOS.org
 * https://github.com/FreeRTOS
 *
 */

/*
 * Creates all the demo application tasks, then starts the scheduler.
 *
 * Main. c also creates a task called "Print".  This only executes every two
 * seconds but has the highest priority so is guaranteed to get processor time.
 * Its main function is to check that all the other tasks are still operational.
 * Nearly all the tasks in the demo application maintain a unique count that is
 * incremented each time the task successfully completes its function.  Should any
 * error occur within the task the count is permanently halted.  The print task
 * checks the count of each task to ensure it has changed since the last time the
 * print task executed.
 *
 * If all the tasks are still incrementing their unique counts the print task
 * displays an "OK" message.
 *
 * The print task blocks on the queue into which messages that require displaying
 * are posted.  It will therefore only block for the full 2 seconds if no messages
 * are posted onto the queue.
 *
 */

 ////////////////////////////////////////////////////////
 ////////////////////////////////////////////////////////
 ////    main.c
 ////////////////////////////////////////////////////////
 ////////////////////////////////////////////////////////

 // zcc +yaz180 -subtype=app -clib=sdcc_iy -SO3 -v -m --list --max-allocs-per-node200000 --math32 -llib/yaz180/freertos @common.lst -o common -create-app
 // cat > /dev/ttyUSB0 < common.ihx

 // zcc +scz180 -subtype=hbios -clib=sdcc_iy -SO3 -v -m --list --max-allocs-per-node200000 --math32 -llib/scz180/freertos @common.lst -o common -create-app
 // cat > /dev/ttyUSB0 < common.ihx

#include <stdlib.h>
#include <stdbool.h>

#if __YAZ180
#include <arch/yaz180.h>

#elif __SCZ180
#include <arch/scz180.h>
#include <arch/hbios.h>                     /* Declarations of HBIOS functions */
#pragma output CRT_ORG_BSS = 0xD500         // move bss origin, check build output to confirm there is no overlap between data and bss sections

#endif

#pragma printf = "%s"                       // enables %s format only

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

/* Demo file headers. */

#include "include/fileIO.h"
#include "include/print.h"

#include "include/BlockQ.h"
#include "include/PollQ.h"
#include "include/death.h"
#include "include/integer.h"
#include "include/flop.h"
#include "include/semtest.h"
#include "include/countsem.h"
#include "include/dynamic.h"
#include "include/QPeek.h"
#include "include/recmutex.h"

/* Priority definitions for all the tasks in the demo application. */
#define mainMATH_TASK_PRIORITY              ( tskIDLE_PRIORITY + 1 )
#define mainSEMAPHORE_TASK_PRIORITY         ( tskIDLE_PRIORITY + 2 )
#define mainQUEUE_POLL_PRIORITY             ( tskIDLE_PRIORITY + 3 )
#define mainQUEUE_BLOCK_PRIORITY            ( tskIDLE_PRIORITY + 4 )
#define mainCREATOR_TASK_PRIORITY           ( tskIDLE_PRIORITY + 6 )
#define mainPRINT_TASK_PRIORITY             ( tskIDLE_PRIORITY + 7 )

#define mainPRINT_STACK_SIZE                ( ( uint16_t ) 512 )

/* Task function for the "Print" task as described at the top of the file. */
static void vErrorChecks( void *pvParameters );

/* Function that checks the unique count of all the other tasks as described at
the top of the file. */
static void prvCheckOtherTasksAreStillRunning( void );

/*-----------------------------------------------------------*/
int main( void )
{
    /* Initialise utilities. */
    vPrintInitialise();

    /* CREATE ALL THE DEMO APPLICATION TASKS. */

//  vStartIntegerMathTasks( mainMATH_TASK_PRIORITY );
//  vStartMathTasks( mainMATH_TASK_PRIORITY );
//  vStartPolledQueueTasks( mainQUEUE_POLL_PRIORITY );
//  vStartBlockingQueueTasks( mainQUEUE_BLOCK_PRIORITY );
//  vStartSemaphoreTasks( mainSEMAPHORE_TASK_PRIORITY );

//  vStartCountingSemaphoreTasks();
//  vStartDynamicPriorityTasks();
//  vStartQueuePeekTasks();
//  vStartRecursiveMutexTasks();

    /* Create the "Print" task as described at the top of the file. */
    xTaskCreate( vErrorChecks, "Print", mainPRINT_STACK_SIZE, NULL, mainPRINT_TASK_PRIORITY, NULL );

    /* This task has to be created last as it keeps account of the number of tasks
    it expects to see running. */
    vCreateSuicidalTasks( mainCREATOR_TASK_PRIORITY );

    /* Set the scheduler running.  This function will not return unless a task
    calls vTaskEndScheduler(). */
    vTaskStartScheduler();

    return 0;
}
/*-----------------------------------------------------------*/

static void vErrorChecks( void *pvParameters )
{
TickType_t xExpectedWakeTime;
const TickType_t xPrintRate = ( TickType_t ) 2000 / portTICK_PERIOD_MS;
const TickType_t xMaxAllowableTimeDifference =  ( TickType_t ) 1000 / portTICK_PERIOD_MS;
TickType_t xWakeTime;
TickType_t xTimeDifference;
const char * pcReceivedMessage;
const char * pcTaskBlockedTooLongMsg = "Print task blocked too long!\r\n";

    /* Stop warnings. */
    ( void ) pvParameters;

    /* Loop continuously, blocking, then checking all the other tasks are still
    running, before blocking once again.  This task blocks on the queue of messages
    that require displaying so will wake either by its time out expiring, or a
    message becoming available. */
    while(1)
    {

        /* Calculate the time we will unblock if no messages are received
        on the queue.  This is used to check that we have not blocked for too long. */
        xExpectedWakeTime = xTaskGetTickCount();
        xExpectedWakeTime += xPrintRate;

        /* Block waiting for either a time out or a message to be posted that
        required displaying. */
        pcReceivedMessage = pcPrintGetNextMessage( xPrintRate );

        /* Was a message received? */
        if( pcReceivedMessage == NULL )
        {
            /* A message was not received so we timed out, did we unblock at the
            expected time? */
            xWakeTime = xTaskGetTickCount();

            /* Calculate the difference between the time we unblocked and the
            time we should have unblocked. */
            if( xWakeTime > xExpectedWakeTime )
            {
                xTimeDifference = (TickType_t)(xWakeTime - xExpectedWakeTime);
            }
            else
            {
                xTimeDifference = (TickType_t)(xExpectedWakeTime - xWakeTime);
            }

            if( xTimeDifference > xMaxAllowableTimeDifference )
            {
                /* We blocked too long - create a message that will get
                printed out the next time around. */
                vPrintDisplayMessage( &pcTaskBlockedTooLongMsg );
            }

            /* Check the other tasks are still running, just in case. */
            prvCheckOtherTasksAreStillRunning();
        }
        else
        {
            /* We unblocked due to a message becoming available.  Send the message
            for printing. */
            vDisplayMessage( pcReceivedMessage );
        }
    }
}
/*-----------------------------------------------------------*/

static void prvCheckOtherTasksAreStillRunning( void )
{
BaseType_t xErrorHasOccurred = pdFALSE;

/*  if( xAreIntegerMathsTaskStillRunning() != pdTRUE )
    {
        vDisplayMessage( "Integer maths task count unchanged!\r\n" );
        xErrorHasOccurred = pdTRUE;
    } // */

/*  if( xAreMathsTaskStillRunning() != pdTRUE )
    {
        vDisplayMessage( "Floating point maths task count unchanged!\r\n" );
        xErrorHasOccurred = pdTRUE;
    } // */

/*  if( xArePollingQueuesStillRunning() != pdTRUE )
    {
        vDisplayMessage( "Polling queue count unchanged!\r\n" );
        xErrorHasOccurred = pdTRUE;
    } // */

/*  if( xAreBlockingQueuesStillRunning() != pdTRUE )
    {
        vDisplayMessage( "Blocking queues count unchanged!\r\n" );
        xErrorHasOccurred = pdTRUE;
    } // */

/*  if( xAreSemaphoreTasksStillRunning() != pdTRUE )
    {
        vDisplayMessage( "Semaphore take count unchanged!\r\n" );
        xErrorHasOccurred = pdTRUE;
    } // */

/*  if( xAreCountingSemaphoreTasksStillRunning() != pdTRUE )
    {
        vDisplayMessage( "Counting semaphore count unchanged!\r\n" );
        xErrorHasOccurred = pdTRUE;
    } // */

/*  if( xAreDynamicPriorityTasksStillRunning() != pdTRUE )
    {
        vDisplayMessage( "Dynamic task priority count unchanged!\r\n" );
        xErrorHasOccurred = pdTRUE;
    }  // */

/*  if( xAreQueuePeekTasksStillRunning() != pdTRUE )
    {
        vDisplayMessage( "Queue peek count unchanged!\r\n" );
        xErrorHasOccurred = pdTRUE;
    } // */

/*  if( xAreRecursiveMutexTasksStillRunning() != pdTRUE )
    {
        vDisplayMessage( "Recursive mutex count unchanged!\r\n" );
        xErrorHasOccurred = pdTRUE;
    } // */

/*  if( xIsCreateTaskStillRunning() != pdTRUE )
    {
        vDisplayMessage( "Incorrect number of tasks running!\r\n" );
        xErrorHasOccurred = pdTRUE;
    } // */

    if( xErrorHasOccurred == pdFALSE )
    {
        vDisplayMessage( "OK\r\n" );
    }
}
/*-----------------------------------------------------------*/
