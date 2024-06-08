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
 * Creates eight tasks, each of which loops continuously performing a floating
 * point calculation - using single precision variables.
 *
 * All the tasks run at the idle priority and never block or yield.  This causes
 * all eight tasks to time slice with the idle task.  Running at the idle priority
 * means that these tasks will get pre-empted any time another task is ready to run
 * or a time slice occurs.  More often than not the pre-emption will occur mid
 * calculation, creating a good test of the schedulers context switch mechanism - a
 * calculation producing an unexpected result could be a symptom of a corruption in
 * the context of a task.
 */


#include <stdlib.h>
#include <math.h>

/* Scheduler include files. */
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

/* Demo program include files. */
#include "include/flop.h"

#define mathSTACK_SIZE          ( configMINIMAL_STACK_SIZE + 64 )
#define mathNUMBER_OF_TASKS     ( 8 )

/* Four tasks, each of which performs a different floating point calculation.
Each of the four is created twice. */
static portTASK_FUNCTION_PROTO( vCompetingMathTask1, pvParameters );
static portTASK_FUNCTION_PROTO( vCompetingMathTask2, pvParameters );
static portTASK_FUNCTION_PROTO( vCompetingMathTask3, pvParameters );
static portTASK_FUNCTION_PROTO( vCompetingMathTask4, pvParameters );

/* These variables are used to check that all the tasks are still running.  If a
task gets a calculation wrong it will
stop incrementing its check variable. */
static volatile uint16_t usTaskCheck[ mathNUMBER_OF_TASKS ] = { ( uint16_t ) 0 };

/*-----------------------------------------------------------*/

void vStartMathTasks( UBaseType_t uxPriority )
{
    xTaskCreate( vCompetingMathTask1, "Math1", mathSTACK_SIZE, ( void * ) &( usTaskCheck[ 0 ] ), uxPriority, NULL );
    xTaskCreate( vCompetingMathTask2, "Math2", mathSTACK_SIZE, ( void * ) &( usTaskCheck[ 1 ] ), uxPriority, NULL );
    xTaskCreate( vCompetingMathTask3, "Math3", mathSTACK_SIZE, ( void * ) &( usTaskCheck[ 2 ] ), uxPriority, NULL );
    xTaskCreate( vCompetingMathTask4, "Math4", mathSTACK_SIZE, ( void * ) &( usTaskCheck[ 3 ] ), uxPriority, NULL );
    xTaskCreate( vCompetingMathTask1, "Math5", mathSTACK_SIZE, ( void * ) &( usTaskCheck[ 4 ] ), uxPriority, NULL );
    xTaskCreate( vCompetingMathTask2, "Math6", mathSTACK_SIZE, ( void * ) &( usTaskCheck[ 5 ] ), uxPriority, NULL );
    xTaskCreate( vCompetingMathTask3, "Math7", mathSTACK_SIZE, ( void * ) &( usTaskCheck[ 6 ] ), uxPriority, NULL );
    xTaskCreate( vCompetingMathTask4, "Math8", mathSTACK_SIZE, ( void * ) &( usTaskCheck[ 7 ] ), uxPriority, NULL );
}
/*-----------------------------------------------------------*/

static portTASK_FUNCTION( vCompetingMathTask1, pvParameters )
{
volatile float f1, f2, f3, f4;
volatile uint16_t *pusTaskCheckVariable;
volatile float fAnswer;
BaseType_t sError = pdFALSE;

    f1 = 123.4567;
    f2 = 2345.6789;
    f3 = -918.222;

    fAnswer = ( f1 + f2 ) * f3;

    /* The variable this task increments to show it is still running is passed in
    as the parameter. */
    pusTaskCheckVariable = ( uint16_t * ) pvParameters;

    /* Keep performing a calculation and checking the result against a constant. */
    for(;;)
    {
        f1 = 123.4567;
        f2 = 2345.6789;
        f3 = -918.222;

        f4 = ( f1 + f2 ) * f3;

        #if configUSE_PREEMPTION == 0
            taskYIELD();
        #endif

        /* If the calculation does not match the expected constant, stop the
        increment of the check variable. */
        if( fabs( f4 - fAnswer ) > 0.01 )
        {
            sError = pdTRUE;
        }

        if( sError == pdFALSE )
        {
            /* If the calculation has always been correct, increment the check
            variable so we know this task is still running okay. */
            ( *pusTaskCheckVariable )++;
        }

        #if configUSE_PREEMPTION == 0
            taskYIELD();
        #endif

    }
}
/*-----------------------------------------------------------*/

static portTASK_FUNCTION( vCompetingMathTask2, pvParameters )
{
volatile float f1, f2, f3, f4;
volatile uint16_t *pusTaskCheckVariable;
volatile float fAnswer;
BaseType_t sError = pdFALSE;

    f1 = -389.38;
    f2 = 32498.2;
    f3 = -2.0001;

    fAnswer = ( f1 / f2 ) * f3;


    /* The variable this task increments to show it is still running is passed in
    as the parameter. */
    pusTaskCheckVariable = ( uint16_t * ) pvParameters;

    /* Keep performing a calculation and checking the result against a constant. */
    for( ;; )
    {
        f1 = -389.38;
        f2 = 32498.2;
        f3 = -2.0001;

        f4 = ( f1 / f2 ) * f3;

        #if configUSE_PREEMPTION == 0
            taskYIELD();
        #endif

        /* If the calculation does not match the expected constant, stop the
        increment of the check variable. */
        if( fabs( f4 - fAnswer ) > 0.01 )
        {
            sError = pdTRUE;
        }

        if( sError == pdFALSE )
        {
            /* If the calculation has always been correct, increment the check
            variable so we know
            this task is still running okay. */
            ( *pusTaskCheckVariable )++;
        }

        #if configUSE_PREEMPTION == 0
            taskYIELD();
        #endif
    }
}
/*-----------------------------------------------------------*/

static portTASK_FUNCTION( vCompetingMathTask3, pvParameters )
{
volatile float *pfArray, fTotal1, fTotal2, fDifference, fPosition;
volatile uint16_t *pusTaskCheckVariable;
const size_t xArraySize = 10;
size_t xPosition;
BaseType_t sError = pdFALSE;

    /* The variable this task increments to show it is still running is passed in
    as the parameter. */
    pusTaskCheckVariable = ( uint16_t * ) pvParameters;

    pfArray = ( float * ) pvPortMalloc( xArraySize * sizeof( float ) );

    /* Keep filling an array, keeping a running total of the values placed in the
    array.  Then run through the array adding up all the values.  If the two totals
    do not match, stop the check variable from incrementing. */
    for( ;; )
    {
        fTotal1 = 0.0;
        fTotal2 = 0.0;
        fPosition = 0.0;

        for( xPosition = 0; xPosition < xArraySize; xPosition++ )
        {
            pfArray[ xPosition ] = fPosition + 5.5;
            fTotal1 += fPosition + 5.5;
        }

        #if configUSE_PREEMPTION == 0
            taskYIELD();
        #endif

        for( xPosition = 0; xPosition < xArraySize; xPosition++ )
        {
            fTotal2 += pfArray[ xPosition ];
        }

        fDifference = fTotal1 - fTotal2;
        if( fabs( fDifference ) > 0.001 )
        {
            sError = pdTRUE;
        }

        #if configUSE_PREEMPTION == 0
            taskYIELD();
        #endif

        if( sError == pdFALSE )
        {
            /* If the calculation has always been correct, increment the check
            variable so we know    this task is still running okay. */
            ( *pusTaskCheckVariable )++;
        }
    }
}
/*-----------------------------------------------------------*/

static portTASK_FUNCTION( vCompetingMathTask4, pvParameters )
{
volatile float *pfArray, fTotal1, fTotal2, fDifference, fPosition;
volatile uint16_t *pusTaskCheckVariable;
const size_t xArraySize = 10;
size_t xPosition;
BaseType_t sError = pdFALSE;

    /* The variable this task increments to show it is still running is passed in
    as the parameter. */
    pusTaskCheckVariable = ( uint16_t * ) pvParameters;

    pfArray = ( float * ) pvPortMalloc( xArraySize * sizeof( float ) );

    /* Keep filling an array, keeping a running total of the values placed in the
    array.  Then run through the array adding up all the values.  If the two totals
    do not match, stop the check variable from incrementing. */
    for( ;; )
    {
        fTotal1 = 0.0;
        fTotal2 = 0.0;
        fPosition = 0.0;

        for( xPosition = 0; xPosition < xArraySize; xPosition++ )
        {
            pfArray[ xPosition ] = fPosition * 12.123;
            fTotal1 += fPosition * 12.123;
        }

        #if configUSE_PREEMPTION == 0
            taskYIELD();
        #endif

        for( xPosition = 0; xPosition < xArraySize; xPosition++ )
        {
            fTotal2 += pfArray[ xPosition ];
        }

        fDifference = fTotal1 - fTotal2;
        if( fabs( fDifference ) > 0.001 )
        {
            sError = pdTRUE;
        }

        #if configUSE_PREEMPTION == 0
            taskYIELD();
        #endif

        if( sError == pdFALSE )
        {
            /* If the calculation has always been correct, increment the check
            variable so we know    this task is still running okay. */
            ( *pusTaskCheckVariable )++;
        }
    }
}
/*-----------------------------------------------------------*/

/* This is called to check that all the created tasks are still running. */
BaseType_t xAreMathsTaskStillRunning( void )
{
/* Keep a history of the check variables so we know if they have been incremented
since the last call. */
static uint16_t usLastTaskCheck[ mathNUMBER_OF_TASKS ] = { ( uint16_t ) 0 };
BaseType_t xReturn = pdTRUE, xTask;

    /* Check the maths tasks are still running by ensuring their check variables
    are still incrementing. */
    for( xTask = 0; xTask < mathNUMBER_OF_TASKS; xTask++ )
    {
        if( usTaskCheck[ xTask ] == usLastTaskCheck[ xTask ] )
        {
            /* The check has not incremented so an error exists. */
            xReturn = pdFALSE;
        }

        usLastTaskCheck[ xTask ] = usTaskCheck[ xTask ];
    }

    return xReturn;
}
