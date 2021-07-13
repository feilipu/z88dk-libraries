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

/****************************************************************************/
/***       Static Functions                                               ***/
/****************************************************************************/

static void appendstring(window_t * win, char * text);
static void appendnumber(window_t * win, int n);

// --------------------------------------------------------
// STATIC FUNCTION appendstring
// --------------------------------------------------------
static void appendstring(window_t * win, char * text)
{
    int l = strlen(win->command) + strlen(text) + 1;

    char * ptr = realloc(win->command, l);

    if( ptr )
    {
        win->command = ptr;
    }

    strcat(win->command, text);
}

// --------------------------------------------------------
// STATIC FUNCTION appendnumber
// --------------------------------------------------------
static void appendnumber(window_t * win, int n)
{
    char sn[8];

    sprintf(sn, "%.3d", n);

    appendstring(win, sn);
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
        case DK: sprintf(s,"W(I(D))"); break;
        case BL: sprintf(s,"W(I(B))"); break;
        case RD: sprintf(s,"W(I(R))"); break;
        case MA: sprintf(s,"W(I(M))"); break;
        case GR: sprintf(s,"W(I(G))"); break;
        case CY: sprintf(s,"W(I(C))"); break;
        case YE: sprintf(s,"W(I(Y))"); break;
        case WH: sprintf(s,"W(I(W))"); break;
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
    
    hypot = (uint16_t)(((uint32_t)d*408)/577);  // d/sqrt(2)

    switch (offset)
    {
        case  E: win->x += d; break;
        case NE: win->y -= hypot; win->x += hypot; break;
        case  N: win->y -= d; break;
        case NW: win->y -= hypot; win->x -= hypot; break;
        case  W: win->x -= d; break;
        case SW: win->y += hypot; win->x -= hypot; break;
        case  S: win->y += d; break;
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
    char s[10];
    sprintf(s, "V(W(E))[]");

    appendstring(win, s);
}

/* Draw a pixel at absolute location */
void draw_pixel_abs(window_t * win,uint16_t x,uint16_t y)
{
    char s[20];
    sprintf(s, "P[%.3d,%.3d]V[]", x, y);

    appendstring(win, s);
}

/* Erase a pixel at absolute location */
void draw_unpixel_abs(window_t * win,uint16_t x,uint16_t y)
{
    char s[26];
    sprintf(s, "P[%.3d,%.3d]V(W(E))[]", x, y);

    appendstring(win, s);
}

/* Draw a line to relative position */
void draw_line_rel(window_t * win,int16_t dx,int16_t dy)
{
    char s[14];
    sprintf(s, "V[%+.3d,%+.3d]", dx, dy);

    appendstring(win, s);

    win->y += dy;
    win->x += dx;
}

/* Erase a line from current position */
void draw_unline_rel(window_t * win,int16_t dx,int16_t dy)
{
    char s[20];
    sprintf(s, "V(W(E))[%+.3d,%+.3d]", dx, dy);

    appendstring(win, s);

    win->y += dy;
    win->x += dx;
}

/* Draw a line to absolute location */
void draw_line_abs(window_t * win,uint16_t x,uint16_t y)
{
    char s[14];
    sprintf(s, "V[%.3d,%.3d]", x, y);

    appendstring(win, s);

    win->y = y;
    win->x = x;
}

/* Erase a line to absolute location */
void draw_unline_abs(window_t * win,uint16_t x,uint16_t y)
{
    char s[20];
    sprintf(s, "V(W(E))[%.3d,%.3d]", x, y);

    appendstring(win, s);

    win->y = y;
    win->x = x;
}

/* Draw a box from current position */
void draw_box(window_t * win,int16_t width,int16_t height)
{
    char s[30];
    sprintf(s, "V[%+.3d,][,%+.3d][%+.3d,][,%+.3d]", width, height, -width, -height);

    appendstring(win, s);
}

/* Erase a box from current position */
void draw_unbox(window_t * win,int16_t width,int16_t height)
{
    char s[36];
    sprintf(s, "V(W(E))[%+.3d,][,%+.3d][%+.3d,][,%+.3d]", width, height, -width, -height);

    appendstring(win, s);
}

/* Draw a filled box from current position */
void draw_box_fill(window_t * win,int16_t width,int16_t height)
{
    char s[36];
    sprintf(s, "V(W(S1))[%+.3d,][,%+.3d][%+.3d,][,%+.3d]", width, height, -width, -height);

    appendstring(win, s);
}

/* Erase a filled box from current position */
void draw_unbox_fill(window_t * win,int16_t width,int16_t height)
{
    char s[36];
    sprintf(s, "V(W(S1,E))[%+.3d,][,%+.3d][%+.3d,][,%+.3d]", width, height, -width, -height);

    appendstring(win, s);
}

/* Draw an arc (circle) in anticlockwise degrees (0 - 360), centred on current position */
void draw_arc(window_t * win,int16_t dx,int16_t dy,int16_t arc)
{
    char s[20];
    sprintf(s, "C(A%+.3d)[%+.3d,%+.3d]", arc, dx, dy);

    appendstring(win, s);
}

/* Erase an arc (circle) in anticlockwise degrees (0 - 360), centred on current position */
void draw_unarc(window_t * win,int16_t dx,int16_t dy,int16_t arc)
{
    char s[26];
    sprintf(s, "C(W(E))(A%+.3d)[%+.3d,%+.3d]", arc, dx, dy);

    appendstring(win, s);
}

/* Draw an arc (circle) filled in anticlockwise degrees (0 - 360), centred on current position */
void draw_arc_fill(window_t * win,int16_t dx,int16_t dy,int16_t arc)
{
    char s[26];
    sprintf(s, "C(W(S1))(A%+.3d)[%+.3d,%+.3d]", arc, dx, dy);

    appendstring(win, s);
}

/* Erase an arc (circle) filled in anticlockwise degrees (0 - 360), centred on current position */
void draw_unarc_fill(window_t * win,int16_t dx,int16_t dy,int16_t arc)
{
    char s[30];
    sprintf(s, "C(W(S1,E))(A%+.3d)[%+.3d,%+.3d]", arc, dx, dy);

    appendstring(win, s);
}

/* Draw text from current position */
void draw_text(window_t * win,char *text,uint8_t textlen,uint8_t size)
{

}
