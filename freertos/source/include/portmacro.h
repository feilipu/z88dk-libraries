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

#ifndef PORTMACRO_H
#define PORTMACRO_H

#ifdef __cplusplus
extern "C" {
#endif

/*-----------------------------------------------------------
 * Port specific definitions.
 *
 * The settings in this file configure FreeRTOS correctly for the
 * given hardware and compiler.
 *
 * These settings should not be altered.
 *-----------------------------------------------------------
 */

/* Type definitions. */

typedef uint16_t StackType_t;
typedef int8_t BaseType_t;
typedef uint8_t UBaseType_t;

#if configUSE_16_BIT_TICKS == 1
    typedef uint16_t TickType_t;
    #define portMAX_DELAY ( TickType_t ) 0xffff
#else
    typedef uint32_t TickType_t;
    #define portMAX_DELAY ( TickType_t ) 0xffffffffUL
#endif



/*-----------------------------------------------------------*/

/* Architecture specifics. */

#define portSTACK_GROWTH            ( -1 )
#define portTICK_PERIOD_MS          ( ( TickType_t ) 1000 / configTICK_RATE_HZ )
#define portBYTE_ALIGNMENT          1

/*-----------------------------------------------------------*/

/* Critical section management. */

#ifdef __SCCZ80

#define portENTER_CRITICAL()        \
    do{                             \
        asm(                        \
            "ld a,i             \n" \
            "di                 \n" \
            "push af            \n" \
            );                      \
    }while(0)


#define portEXIT_CRITICAL()         \
    do{                             \
        asm(                        \
            "pop af             \n" \
            "di                 \n" \
            "jp PO,ASMPC+4      \n" \
            "ei                 \n" \
            );                      \
    }while(0)

#define portDISABLE_INTERRUPTS()    \
    do{                             \
        asm(                        \
            "di                 \n" \
            );                      \
    }while(0)

#define portENABLE_INTERRUPTS()     \
    do{                             \
        asm(                        \
            "ei                 \n" \
            );                      \
    }while(0)

#define portNOP()                   \
    do{                             \
        asm(                        \
            "nop                \n" \
            );                      \
    }while(0)

/*
 * Macros to save all the registers, and the save the stack pointer into the TCB.
 */
 
#define portSAVE_CONTEXT()          \
    do{                             \
        asm(                        \
            "EXTERN _pxCurrentTCB   \n" \
            "push af            \n" \
            "ld a,i             \n" \
            "di                 \n" \
            "push af    ; if    \n" \
            "push bc            \n" \
            "push de            \n" \
            "push hl            \n" \
            "exx                \n" \
            "ex af,af           \n" \
            "push af            \n" \
            "push bc            \n" \
            "push de            \n" \
            "push hl            \n" \
            "push ix            \n" \
            "push iy            \n" \
            "ld hl,0            \n" \
            "add hl,sp          \n" \
            "ld de,(_pxCurrentTCB)\n"\
            "ex de,hl           \n" \
            "ld (hl),e          \n" \
            "inc hl             \n" \
            "ld (hl),d          \n" \
            );                      \
    }while(0)

#define portRESTORE_CONTEXT()       \
    do{                             \
        asm(                        \
            "EXTERN _pxCurrentTCB   \n" \
            "ld hl,(_pxCurrentTCB)  \n" \
            "ld e,(hl)          \n" \
            "inc hl             \n" \
            "ld d,(hl)          \n" \
            "ex de,hl           \n" \
            "ld sp,hl           \n" \
            "pop iy             \n" \
            "pop ix             \n" \
            "pop hl             \n" \
            "pop de             \n" \
            "pop bc             \n" \
            "pop af             \n" \
            "ex af,af           \n" \
            "exx                \n" \
            "pop hl             \n" \
            "pop de             \n" \
            "pop bc             \n" \
            "pop af      ; if   \n" \
            "jp PO,ASMPC+4      \n" \
            "ei                 \n" \
            "pop af             \n" \
            "ret                \n" \
            );                      \
    }while(0)

#define portSAVE_CONTEXT_IN_ISR()   \
    do{                             \
        asm(                        \
            "EXTERN _pxCurrentTCB   \n" \
            "push af            \n" \
            "ld a,0x7F          \n" \
            "inc a  ; set P/V   \n" \
            "push af     ; if   \n" \
            "push bc            \n" \
            "push de            \n" \
            "push hl            \n" \
            "exx                \n" \
            "ex af,af           \n" \
            "push af            \n" \
            "push bc            \n" \
            "push de            \n" \
            "push hl            \n" \
            "push ix            \n" \
            "push iy            \n" \
            "ld hl,0            \n" \
            "add hl,sp          \n" \
            "ld de,(_pxCurrentTCB)  \n" \
            "ex de,hl           \n" \
            "ld (hl),e          \n" \
            "inc hl             \n" \
            "ld (hl),d          \n" \
            );                      \
    }while(0)

#define portRESTORE_CONTEXT_IN_ISR()\
    do{                             \
        asm(                        \
            "EXTERN _pxCurrentTCB   \n" \
            "ld hl,(_pxCurrentTCB)  \n" \
            "ld e,(hl)          \n" \
            "inc hl             \n" \
            "ld d,(hl)          \n" \
            "ex de,hl           \n" \
            "ld sp,hl           \n" \
            "pop iy             \n" \
            "pop ix             \n" \
            "pop hl             \n" \
            "pop de             \n" \
            "pop bc             \n" \
            "pop af             \n" \
            "ex af,af           \n" \
            "exx                \n" \
            "pop hl             \n" \
            "pop de             \n" \
            "pop bc             \n" \
            "pop af      ; if   \n" \
            "jp PO,ASMPC+4      \n" \
            "ei                 \n" \
            "pop af             \n" \
            "reti               \n" \
            );                      \
    }while(0)

#endif

/*-----------------------------------------------------------*/

/* Critical section management. */

#ifdef __SDCC

#define portENTER_CRITICAL()        \
    do{                             \
        __asm                       \
            ld a,i                  \
            di                      \
            push af                 \
        __endasm;                   \
    }while(0)


#define portEXIT_CRITICAL()         \
    do{                             \
        __asm                       \
            pop af                  \
            di                      \
            jp PO,ASMPC+4           \
            ei                      \
        __endasm;                   \
    }while(0)

#define portDISABLE_INTERRUPTS()    \
    do{                             \
        __asm                       \
            di                      \
        __endasm;                   \
    }while(0)

#define portENABLE_INTERRUPTS()     \
    do{                             \
        __asm                       \
            ei                      \
        __endasm;                   \
    }while(0)

#define portNOP()                   \
    do{                             \
        __asm                       \
            nop                     \
        __endasm;                   \
    }while(0)

/*
 * Macros to save all the registers, and the save the stack pointer into the TCB.
 */
 
#define portSAVE_CONTEXT()          \
    do{                             \
        __asm                       \
            EXTERN _pxCurrentTCB    \
            push af                 \
            ld a,i                  \
            di                      \
            push af     ; if        \
            push bc                 \
            push de                 \
            push hl                 \
            exx                     \
            ex af,af                \
            push af                 \
            push bc                 \
            push de                 \
            push hl                 \
            push ix                 \
            push iy                 \
            ld hl,0                 \
            add hl,sp               \
            ld de,(_pxCurrentTCB)   \
            ex de,hl                \
            ld (hl),e               \
            inc hl                  \
            ld (hl),d               \
        __endasm;                   \
    }while(0)

#define portRESTORE_CONTEXT()       \
    do{                             \
        __asm                       \
            EXTERN _pxCurrentTCB    \
            ld hl,(_pxCurrentTCB)   \
            ld e,(hl)               \
            inc hl                  \
            ld d,(hl)               \
            ex de,hl                \
            ld sp,hl                \
            pop iy                  \
            pop ix                  \
            pop hl                  \
            pop de                  \
            pop bc                  \
            pop af                  \
            ex af,af                \
            exx                     \
            pop hl                  \
            pop de                  \
            pop bc                  \
            pop af      ; if        \
            jp PO,ASMPC+4           \
            ei                      \
            pop af                  \
            ret                     \
        __endasm;                   \
    }while(0)

#define portSAVE_CONTEXT_IN_ISR()   \
    do{                             \
        __asm                       \
            EXTERN _pxCurrentTCB    \
            push af                 \
            ld a,0x7F               \
            inc a       ; set P/V   \
            push af     ; if        \
            push bc                 \
            push de                 \
            push hl                 \
            exx                     \
            ex af,af                \
            push af                 \
            push bc                 \
            push de                 \
            push hl                 \
            push ix                 \
            push iy                 \
            ld hl,0                 \
            add hl,sp               \
            ld de,(_pxCurrentTCB)   \
            ex de,hl                \
            ld (hl),e               \
            inc hl                  \
            ld (hl),d               \
        __endasm;                   \
    }while(0)

#define portRESTORE_CONTEXT_IN_ISR()\
    do{                             \
        __asm                       \
            EXTERN _pxCurrentTCB    \
            ld hl,(_pxCurrentTCB)   \
            ld e,(hl)               \
            inc hl                  \
            ld d,(hl)               \
            ex de,hl                \
            ld sp,hl                \
            pop iy                  \
            pop ix                  \
            pop hl                  \
            pop de                  \
            pop bc                  \
            pop af                  \
            ex af,af                \
            exx                     \
            pop hl                  \
            pop de                  \
            pop bc                  \
            pop af      ; if        \
            jp PO,ASMPC+4           \
            ei                      \
            pop af                  \
            reti                    \
        __endasm;                   \
    }while(0)

#endif

/*-----------------------------------------------------------*/

/* Kernel utilities. */
extern void vPortYield( void );
#define portYIELD()                 vPortYield()

/*-----------------------------------------------------------*/

/* Task function macros as described on the FreeRTOS.org WEB site. */
#define portTASK_FUNCTION_PROTO( vFunction, pvParameters ) void vFunction( void *pvParameters )
#define portTASK_FUNCTION( vFunction, pvParameters ) void vFunction( void *pvParameters )

#ifdef __cplusplus
}
#endif

#endif /* PORTMACRO_H */

