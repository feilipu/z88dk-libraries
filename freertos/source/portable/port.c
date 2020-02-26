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

/*-----------------------------------------------------------*/

/* We require the address of the pxCurrentTCB variable, but don't want to know
any details of its type. */
typedef void TCB_t;
extern volatile TCB_t * volatile pxCurrentTCB;

/*-----------------------------------------------------------*/

/*
 * Perform hardware setup to enable ticks from Timer.
 */
static void prvSetupTimerInterrupt( void ) __preserves_regs(b,c,iyh,iyl);

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

BaseType_t xPortStartScheduler( void ) __preserves_regs(a,b,c,d,e,iyh,iyl) __naked
{
    /* Setup the relevant timer hardware to generate the tick. */
    prvSetupTimerInterrupt();

    /* Restore the context of the first task that is going to run. */
    portRESTORE_CONTEXT();

    /* Should not get here. */
    return pdFALSE;
}
/*-----------------------------------------------------------*/

void vPortEndScheduler( void ) __preserves_regs(b,c,d,e,h,l,iyh,iyl) __naked
{
    /* It is unlikely that the Z80 port will get stopped.
     * If required simply
     * disable the tick interrupt here.
     */
    do{
#ifdef __SCCZ80
        asm(                                                                \
            "EXTERN TCR, TCR_TIE1, TCR_TDE1                             \n" \
            "; disable down counting and interrupts for PRT1            \n" \
            "in0 a,(TCR)                                                \n" \
            "xor TCR_TIE1|TCR_TDE1                                      \n" \
            "out0 (TCR),a                                               \n" \
            );
#endif

#ifdef __SDCC
        __asm
            EXTERN TCR, TCR_TIE1, TCR_TDE1
            ; disable down counting and interrupts for PRT1
            in0 a,(TCR)
            xor TCR_TIE1|TCR_TDE1
            out0 (TCR),a
        __endasm;
#endif
    }while(0);
}

/*-----------------------------------------------------------*/

/*
 * Manual context switch.  The first thing we do is save the registers so we
 * can use a naked attribute.
 */
void vPortYield( void ) __preserves_regs(a,b,c,d,e,h,l,iyh,iyl) __naked
{
    portSAVE_CONTEXT();
    vTaskSwitchContext();
    portRESTORE_CONTEXT();
}
/*-----------------------------------------------------------*/

/*
 * Initialize Timer (PRT1 for YAZ180, first implementation).
 */
void prvSetupTimerInterrupt( void ) __preserves_regs(b,c,iyh,iyl)
{
    do{
#ifdef __SCCZ80
        asm(                                                                \
            "EXTERN __CPU_CLOCK                                         \n" \
            "EXTERN RLDR1L, RLDR1H                                      \n" \
            "EXTERN TCR, TCR_TIE1, TCR_TDE1                             \n" \
            "; address of ISR                                           \n" \
            "ld de,_timer_isr                                           \n" \
            "; address of PRT1 Jump Address                             \n" \
            "ld hl,0xFFE6           ; YAZ180 PRT1 address               \n" \
            "ld (hl),e                                                  \n" \
            "inc hl                                                     \n" \
            "ld (hl),d                                                  \n" \
            "; we do 256 ticks per second                               \n" \
            "ld hl,__CPU_CLOCK/20/256-1                                 \n" \
            "out0(RLDR1L),l                                             \n" \
            "out0(RLDR1H),h                                             \n" \
            "; enable down counting and interrupts for PRT1             \n" \
            "in0 a,(TCR)                                                \n" \
            "or TCR_TIE1|TCR_TDE1                                       \n" \
            "out0 (TCR),a                                               \n" \
            );
#endif

#ifdef __SDCC
        __asm
            EXTERN __CPU_CLOCK
            EXTERN RLDR1L, RLDR1H
            EXTERN TCR, TCR_TIE1, TCR_TDE1
            ; address of ISR
            ld de,_timer_isr
            ; address of PRT1 Jump Address
            ld hl,0xFFE6            ; YAZ180 PRT1 address
            ld (hl),e
            inc hl
            ld (hl),d
            ; we do 256 ticks per second
            ld hl,__CPU_CLOCK/20/256-1 
            out0(RLDR1L),l
            out0(RLDR1H),h
            ; enable down counting and interrupts for PRT1
            in0 a,(TCR)
            or TCR_TIE1|TCR_TDE1
            out0 (TCR),a
        __endasm;
#endif
    }while(0);
}


/*-----------------------------------------------------------*/

void timer_isr(void) __preserves_regs(a,b,c,d,e,h,l,iyh,iyl) __naked
{	
#if configUSE_PREEMPTION == 1
	/*
 	 * Tick ISR for preemptive scheduler.  We can use a naked attribute as
	 * the context is saved at the start of timer_isr().  The tick
	 * count is incremented after the context is saved.
     *
     * Context switch function used by the tick.  This must be identical to
     * vPortYield() from the call to vTaskSwitchContext() onwards.  The only
     * difference from vPortYield() is the tick count is incremented as the
     * call comes from the tick ISR.
     */
    portSAVE_CONTEXT_IN_ISR();
    portRESET_TIMER_INTERRUPT();
    xTaskIncrementTick();
    vTaskSwitchContext();
    portRESTORE_CONTEXT_IN_ISR();
#else
	/*
	 * Tick ISR for the cooperative scheduler.  All this does is increment the
	 * tick count.  We don't need to switch context, this can only be done by
	 * manual calls to taskYIELD();
	 */
    portSAVE_CONTEXT_IN_ISR();
    portRESET_TIMER_INTERRUPT();
  	xTaskIncrementTick();
    portRESTORE_CONTEXT_IN_ISR();
#endif
} // configUSE_PREEMPTION
