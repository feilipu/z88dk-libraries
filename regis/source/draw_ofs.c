/*
 * draw_ofs.c
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

/* Relative move offset direction */
void draw_ofs(window_t * win, uint16_t d, offset_t offset)
{
    uint16_t hypot;

    fprintf(win->fp, "P(W(M%d))%d", d, (uint8_t)offset);

    hypot = (uint16_t)((d*70)/99);  // d/sqrt(2)

    switch (offset)
    {
        case _EE: win->x += d; break;
        case _NE: win->y -= hypot; win->x += hypot; break;
        case _NN: win->y -= d; break;
        case _NW: win->y -= hypot; win->x -= hypot; break;
        case _WW: win->x -= d; break;
        case _SW: win->y += hypot; win->x -= hypot; break;
        case _SS: win->y += d; break;
        case _SE: win->y += hypot; win->x += hypot; break;
    }
}
