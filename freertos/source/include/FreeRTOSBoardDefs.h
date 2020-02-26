/*
 * Copyright (C) 2020 Phillip Stevens  All Rights Reserved.
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
 * 1 tab == 4 spaces!
 *
 * This file is NOT part of the FreeRTOS distribution.
 *
 */

/* freeRTOSBoardDefs.h
 *
 * Board (hardware) specific definitions for the Z80 boards that I use regularly.
 * This includes
 * YAZ180 with Z8S180
 * SCZ180 with Z8S180
 * RC2014 with Z80
 * Spectrum Next with Z80N
 *
 * This file is NOT part of the FreeRTOS distribution.
 *
 */

#ifndef freeRTOSBoardDefs_h
#define freeRTOSBoardDefs_h

#ifdef __cplusplus
extern "C" {
#endif

#define configTICK_RATE_HZ			( ( TickType_t ) 256 )

#ifdef __SCCZ80

#define portRESET_TIMER_INTERRUPT() \
    do{                             \
        asm(                        \
            "EXTERN TCR, TMDR1L                                                 \n" \
            "in0 a,(TCR)            ; to clear the PRT0 interrupt, read the TCR \n" \
            "in0 a,(TMDR1L)         ; followed by the TMDR1                     \n" \
            );                      \
    }while(0)

#endif

#ifdef __SDCC

#define portRESET_TIMER_INTERRUPT() \
    do{                             \
        __asm                       \
            EXTERN TCR, TMDR1L                                                      \
            in0 a,(TCR)            ; to clear the PRT0 interrupt, read the TCR      \
            in0 a,(TMDR1L)         ; followed by the TMDR1                          \
        __endasm;                   \
    }while(0)

#endif

#ifdef __cplusplus
}
#endif

#endif // freeRTOSBoardDefs_h
