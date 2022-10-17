/*
 * draw_intensity.c
 *
 * Copyright (c) 2021 Phillip Stevens
 * Create Time: July 2021
 *
 * The MIT License (MIT)
 *
 * Permission is hereby granted,free of charge,to any person obtaining a copy
 * of this software and associated documentation files (the "Software"),to deal
 * in the Software without restriction,including without limitation the rights
 * to use,copy,modify,merge,publish,distribute,sublicense,and/or sell
 * copies of the Software,and to permit persons to whom the Software is
 * furnished to do so,subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS",WITHOUT WARRANTY OF ANY KIND,EXPRESS OR
 * IMPLIED,INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,DAMAGES OR OTHER
 * LIABILITY,WHETHER IN AN ACTION OF CONTRACT,TORT OR OTHERWISE,ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

/****************************************************************************/
/***        Include files                                                 ***/
/****************************************************************************/


#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#if __SCCZ80
#include "include/sccz80/regis.h"
#elif __SDCC
#include "include/sdcc/regis.h"
#endif

/****************************************************************************/
/***       Private Functions                                              ***/
/****************************************************************************/

#if __SCCZ80
void __LIB__ appendstring(window_t * win, char const * text) __smallc __z88dk_callee;
#elif __SDCC
void appendstring(window_t * win, char const * text) __z88dk_callee;
#endif

/****************************************************************************/
/***       Functions                                                      ***/
/****************************************************************************/

/* Set writing intensity (colour) */
void draw_intensity(window_t * win, w_intensity_t intensity)
{
    char s[8];

    switch (intensity)
    {
        case _D: sprintf(s,"W(I(D))"); break;
        case _B: sprintf(s,"W(I(B))"); break;
        case _R: sprintf(s,"W(I(R))"); break;
        case _M: sprintf(s,"W(I(M))"); break;
        case _G: sprintf(s,"W(I(G))"); break;
        case _C: sprintf(s,"W(I(C))"); break;
        case _Y: sprintf(s,"W(I(Y))"); break;
        case _W: sprintf(s,"W(I(W))"); break;
    }

    appendstring(win, s);
}

