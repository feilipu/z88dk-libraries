/*
 * regis.c
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

#pragma printf  =  "%c %s %d"       // enables %c, %s, %d only

/****************************************************************************/
/***       Private Functions                                              ***/
/****************************************************************************/

#if __SCCZ80
void __LIB__ appendstring(window_t * win, char * text) __smallc __z88dk_callee;
#elif __SDCC
void appendstring(window_t * win, char * text) __z88dk_callee;
#endif


#if __SCCZ80
void __LIB__ appendstring(window_t * win, char * text) __smallc __z88dk_callee
#elif __SDCC
#include "include/sdcc/regis.h"
void appendstring(window_t * win, char * text) __z88dk_callee
#endif
{
    int l = strlen(win->command) + strlen(text) + 1;

    char * ptr = realloc(win->command, l);

    if( ptr )
    {
        win->command = ptr;
    }

    strcat(win->command, text);
}


/****************************************************************************/
/***       Functions                                                      ***/
/****************************************************************************/


/* Open a graphics window, in graphics mode, and inititialise graphics */
uint8_t window_new(window_t * win,uint16_t width,uint16_t height) 
{
    if(win != NULL)
    {
        win->command = (char *)malloc(20);
        if (width && width < WIDTH_MAX) win->width = width; else win->width = WIDTH_MAX-1;
        if (height && height < HEIGHT_MAX) win->height = height; else win->height = HEIGHT_MAX-1;
        sprintf(win->command, "%cP1pS(E)", ASCII_ESC);
        return 1;
    }
    else
    {
        return 0;
    }
}

/* Reset a graphics window, clear command string */
void window_reset(window_t * win) 
{
    if(win != NULL)
    {
        win->command[0] = 0;
    }
}

/* Write out instructions */
void window_write(window_t * win)
{
    fprintf(stdout, "%s", win->command);
}

/* Clear window */
void window_clear(window_t * win)
{
    char s[12];
    sprintf(s, "S(E)");

    appendstring(win, s);
}

/* Close a graphics window, return to text mode */
void window_close(window_t * win)
{
    fprintf(stdout, "%c%c\n", ASCII_ESC, ASCII_BSLASH); /* ESC \ */

    free(win->command);
}

/* Set writing mode */
void draw_mode(window_t * win,mode_t mode)
{
    char s[6];

    switch (mode)
    {
        case REP: sprintf(s,"W(R)"); break;
        case ERA: sprintf(s,"W(E)"); break;
        case OVL: sprintf(s,"W(V)"); break;
        case CPL: sprintf(s,"W(C)"); break;
    }
    

    appendstring(win, s);
}

/* Set writing pattern */
void draw_pattern(window_t * win,pattern_t pattern)
{
    char s[6];

    switch (pattern)
    {
        case P0: sprintf(s,"W(P0)"); break;
        case P1: sprintf(s,"W(P1)"); break;
        case P2: sprintf(s,"W(P2)"); break;
        case P3: sprintf(s,"W(P3)"); break;
        case P4: sprintf(s,"W(P4)"); break;
        case P5: sprintf(s,"W(P5)"); break;
        case P6: sprintf(s,"W(P6)"); break;
        case P7: sprintf(s,"W(P7)"); break;
        case P8: sprintf(s,"W(P8)"); break;
        case P9: sprintf(s,"W(P9)"); break;
    }

    appendstring(win, s);
}

/* Set writing intensity (colour) */
void draw_intensity(window_t * win,intensity_t intensity)
{
    char s[8];

    switch (intensity)
    {
        case D: sprintf(s,"W(I(D))"); break;
        case B: sprintf(s,"W(I(B))"); break;
        case R: sprintf(s,"W(I(R))"); break;
        case M: sprintf(s,"W(I(M))"); break;
        case G: sprintf(s,"W(I(G))"); break;
        case C: sprintf(s,"W(I(C))"); break;
        case Y: sprintf(s,"W(I(Y))"); break;
        case W: sprintf(s,"W(I(W))"); break;
    }

    appendstring(win, s);
}

/* Relative move position */
void draw_rel(window_t * win,int16_t dx,int16_t dy)
{
    char s[14];
    sprintf(s, "P[%+.3d,%+.3d]", dx, dy);

    appendstring(win, s);

    win->y += dy;
    win->x += dx;
}

/* Relative move offset direction */
void draw_ofs(window_t * win,uint16_t d,offset_t offset)
{
    char s[14];
    uint16_t hypot;
    sprintf(s, "P(W(M%d))%d", d, (uint8_t)offset);

    appendstring(win, s);
    
    hypot = (uint16_t)((d*70)/99);  // d/sqrt(2)

    switch (offset)
    {
        case EE: win->x += d; break;
        case NE: win->y -= hypot; win->x += hypot; break;
        case NN: win->y -= d; break;
        case NW: win->y -= hypot; win->x -= hypot; break;
        case WW: win->x -= d; break;
        case SW: win->y += hypot; win->x -= hypot; break;
        case SS: win->y += d; break;
        case SE: win->y += hypot; win->x += hypot; break;
    }
}

/* Set absolute position */
void draw_abs(window_t * win,uint16_t x,uint16_t y)
{
    char s[14];
    sprintf(s, "P[%.3d,%.3d]", x, y);

    appendstring(win, s);

    win->y = y;
    win->x = x;
}

/* Draw a pixel to screen at current position */
void draw_pixel_rel(window_t * win)
{
    char s[6];
    sprintf(s, "V[]");

    appendstring(win, s);
}

/* Erase a pixel at current position */
void draw_unpixel_rel(window_t * win)
{
    char s[12];
    sprintf(s, "V(W(E))[]");

    appendstring(win, s);
}

/* Draw a pixel at absolute location */
void draw_pixel_abs(window_t * win,uint16_t x,uint16_t y)
{
    char s[18];
    sprintf(s, "P[%.3d,%.3d]V[]", x, y);

    appendstring(win, s);
}

/* Erase a pixel at absolute location */
void draw_unpixel_abs(window_t * win,uint16_t x,uint16_t y)
{
    char s[24];
    sprintf(s, "P[%.3d,%.3d]V(W(E))[]", x, y);

    appendstring(win, s);
}

/* Draw a line to relative position */
void draw_line_rel(window_t * win,int16_t dx,int16_t dy)
{
    char s[16];
    sprintf(s, "V[][%+.3d,%+.3d]", dx, dy);

    appendstring(win, s);

    win->y += dy;
    win->x += dx;
}

/* Erase a line from current position */
void draw_unline_rel(window_t * win,int16_t dx,int16_t dy)
{
    char s[22];
    sprintf(s, "V(W(E))[][%+.3d,%+.3d]", dx, dy);

    appendstring(win, s);

    win->y += dy;
    win->x += dx;
}

/* Draw a line to absolute location */
void draw_line_abs(window_t * win,uint16_t x,uint16_t y)
{
    char s[16];
    sprintf(s, "V[][%.3d,%.3d]", x, y);

    appendstring(win, s);

    win->y = y;
    win->x = x;
}

/* Erase a line to absolute location */
void draw_unline_abs(window_t * win,uint16_t x,uint16_t y)
{
    char s[22];
    sprintf(s, "V(W(E))[][%.3d,%.3d]", x, y);

    appendstring(win, s);

    win->y = y;
    win->x = x;
}

/* Draw a box from current position */
void draw_box(window_t * win,int16_t width,int16_t height)
{
    char s[30];
    sprintf(s, "V(B)[%+.3d,][,%+.3d][%+.3d,](E)", width, height, -width, -height);

    appendstring(win, s);
}

/* Erase a box from current position */
void draw_unbox(window_t * win,int16_t width,int16_t height)
{
    char s[36];
    sprintf(s, "V(W(E))(B)[%+.3d,][,%+.3d][%+.3d,](E)", width, height, -width, -height);

    appendstring(win, s);
}

/* Draw a filled box from current position */
void draw_box_fill(window_t * win,int16_t width,int16_t height)
{
    char s[38];
    sprintf(s, "V(W(S1))(B)[%+.3d,][,%+.3d][%+.3d,](E)", width, height, -width, -height);

    appendstring(win, s);
}

/* Erase a filled box from current position */
void draw_unbox_fill(window_t * win,int16_t width,int16_t height)
{
    char s[40];
    sprintf(s, "V(W(S1,E))(B)[%+.3d,][,%+.3d][%+.3d,](E)", width, height, -width, -height);

    appendstring(win, s);
}

/* Draw a circle, centred on current position */
void draw_circle(window_t * win,uint16_t radius)
{
    char s[10];
    sprintf(s, "C[%+.3d]", radius);

    appendstring(win, s);
}

/* Erase a circle, centred on current position */
void draw_uncircle(window_t * win,uint16_t radius)
{
    char s[16];
    sprintf(s, "C(W(E))[%+.3d]", radius);

    appendstring(win, s);
}

/* Draw a circle filled, centred on current position */
void draw_circle_fill(window_t * win,uint16_t radius)
{
    char s[16];
    sprintf(s, "C(W(S1))[%+.3d]", radius);

    appendstring(win, s);
}

/* Erase a circle filled, centred on current position */
void draw_uncircle_fill(window_t * win,uint16_t radius)
{
    char s[20];
    sprintf(s, "C(W(S1,E))[%+.3d]", radius);

    appendstring(win, s);
}

/* Draw an arc (circle) in anticlockwise degrees (0 - 360), centred on current position */
void draw_arc(window_t * win,uint16_t radius,int16_t arc)
{
    char s[16];
    sprintf(s, "C(A%+.3d)[%+.3d]", arc, radius);

    appendstring(win, s);
}

/* Erase an arc (circle) in anticlockwise degrees (0 - 360), centred on current position */
void draw_unarc(window_t * win,uint16_t radius,int16_t arc)
{
    char s[20];
    sprintf(s, "C(W(E))(A%+.3d)[%+.3d]", arc, radius);

    appendstring(win, s);
}

/* Draw text from current position */
void draw_text(window_t * win,char * text,uint8_t size)
{
    char s[10];
    sprintf(s, "T(S%.2d)\"", size);
    appendstring(win, s);
    appendstring(win, text);
    appendstring(win, "\"");
}

/* Draw custom ReGIS from current position */
void draw_free(window_t * win,char * text)
{
    appendstring(win, text);
}

