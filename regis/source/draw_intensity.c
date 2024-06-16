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
/***       Functions                                                      ***/
/****************************************************************************/

/* Set writing intensity (colour) */
void draw_intensity(window_t * win, w_intensity_t intensity)
{
    switch (intensity)
    {
        case _D: fputs("W(I(D))", win->fp); break;
        case _B: fputs("W(I(B))", win->fp); break;
        case _R: fputs("W(I(R))", win->fp); break;
        case _M: fputs("W(I(M))", win->fp); break;
        case _G: fputs("W(I(G))", win->fp); break;
        case _C: fputs("W(I(C))", win->fp); break;
        case _Y: fputs("W(I(Y))", win->fp); break;
        case _W: fputs("W(I(W))", win->fp); break;
    }
}
