/*
 * FreeRTOS Kernel V10.3.0
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
 * http://www.FreeRTOS.org
 * http://aws.amazon.com/freertos
 *
 * 1 tab == 4 spaces!
 */


#include <stdlib.h>

#include "include/FreeRTOS.h"

#if __SDCC
#include "include/sdcc/task.h"
#elif __SCCZ80
#include "include/sccz80/task.h"
#endif

/*-----------------------------------------------------------
 * Implementation of functions defined in portable.h for the Z80 port.
 *----------------------------------------------------------*/

/* Start tasks with interrupts enabled. */
#define portFLAGS_INT_ENABLED           ( (StackType_t) 0x80 )

#define    portSCHEDULER_ISR               WDT_vect

/*-----------------------------------------------------------*/

/* We require the address of the pxCurrentTCB variable, but don't want to know
any details of its type. */
typedef void TCB_t;
extern volatile TCB_t * volatile pxCurrentTCB;

/*-----------------------------------------------------------*/
/*
 * Perform hardware setup to enable ticks from Watchdog Timer.
 */
static void prvSetupTimerInterrupt( void );

/*-----------------------------------------------------------*/


/*
 * See header file for description.
 */
StackType_t *pxPortInitialiseStack( StackType_t *pxTopOfStack, TaskFunction_t pxCode, void *pvParameters )
{
    /* Place the parameter on the stack in the expected location. */
    *pxTopOfStack-- = ( StackType_t ) pvParameters;

    /* Place the task return address on stack. Not used */
    *pxTopOfStack-- = ( StackType_t ) 0;
	
    /* The start of the task code will be popped off the stack last, so place
    it on first. */
    *pxTopOfStack-- = ( StackType_t ) pxCode;

    /* Now the registers. */
    *pxTopOfStack-- = ( StackType_t ) 0xAFAF;   /* AF  */
    *pxTopOfStack-- = ( StackType_t ) 0x0404;   /* IF  */
	*pxTopOfStack-- = ( StackType_t ) 0xBCBC;   /* BC  */
    *pxTopOfStack-- = ( StackType_t ) 0xDEDE;   /* DE  */
    *pxTopOfStack-- = ( StackType_t ) 0xEFEF;   /* HL  */
    *pxTopOfStack-- = ( StackType_t ) 0xFAFA;   /* AF' */
    *pxTopOfStack-- = ( StackType_t ) 0xCBCB;   /* BC' */
    *pxTopOfStack-- = ( StackType_t ) 0xEDED;   /* DE' */
    *pxTopOfStack-- = ( StackType_t ) 0xFEFE;   /* HL' */
	*pxTopOfStack-- = ( StackType_t ) 0xCEFA;   /* IX  */
	*pxTopOfStack   = ( StackType_t ) 0xADDE;   /* IY  */

    return pxTopOfStack;
}
/*-----------------------------------------------------------*/

BaseType_t xPortStartScheduler( void )
{
    /* Setup the relevant timer hardware to generate the tick. */
    prvSetupTimerInterrupt();

    /* Restore the context of the first task that is going to run. */
    portRESTORE_CONTEXT();

    /* Should not get here. */
    return pdTRUE;
}
/*-----------------------------------------------------------*/

void vPortEndScheduler( void )
{
    /* It is unlikely that the Z80 port will get stopped.
     * If required simply
     * disable the tick interrupt here.
     */
}
/*-----------------------------------------------------------*/

/*
 * Manual context switch.  The first thing we do is save the registers so we
 * can use a naked attribute.
 */
void vPortYield( void )
{
    portSAVE_CONTEXT();
    vTaskSwitchContext();
    portRESTORE_CONTEXT();
}
/*-----------------------------------------------------------*/

/*
 * Context switch function used by the tick.  This must be identical to
 * vPortYield() from the call to vTaskSwitchContext() onwards.  The only
 * difference from vPortYield() is the tick count is incremented as the
 * call comes from the tick ISR.
 */
void vPortYieldFromTick( void )
{
    portSAVE_CONTEXT_IN_ISR();

    if( xTaskIncrementTick() != pdFALSE )
    {
        vTaskSwitchContext();
    }

    portRESTORE_CONTEXT_IN_ISR();
}
/*-----------------------------------------------------------*/

//initialize watchdog
void prvSetupTimerInterrupt( void )
{
    //reset watchdog
    wdt_reset();

    //set up WDT Interrupt (rather than the WDT Reset).
    wdt_interrupt_enable( portUSE_WDTO );
}

/*-----------------------------------------------------------*/

void timer_isr(void) __naked
{	
#if configUSE_PREEMPTION == 1
	/*
 	 * Tick ISR for preemptive scheduler.  We can use a naked attribute as
	 * the context is saved at the start of vPortYieldFromTick().  The tick
	 * count is incremented after the context is saved.
	 */
    portSAVE_CONTEXT_IN_ISR();
    vTaskIncrementTick();
    vTaskSwitchContext();
    portRESTORE_CONTEXT_IN_ISR();
#else
	/*
	 * Tick ISR for the cooperative scheduler.  All this does is increment the
	 * tick count.  We don't need to switch context, this can only be done by
	 * manual calls to taskYIELD();
	 */
    portSAVE_CONTEXT_IN_ISR();
  	vTaskIncrementTick();
    portRESTORE_CONTEXT_IN_ISR();
#endif
} // configUSE_PREEMPTION
