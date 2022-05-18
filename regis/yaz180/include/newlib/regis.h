/*
 * regis.h
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

include(__link__.m4)
undefine(`m4_SCCZ80_NOLIB')

#ifndef _REGIS_H
#define _REGIS_H

#ifdef __cplusplus
"C" {
#endif

#include <stdint.h>

#define ASCII_NULL      0x00        // null
#define ASCII_BELL      0x07        // bell
#define ASCII_BS        0x08        // backspace
#define ASCII_TAB       0x09        // horizontal tab
#define ASCII_LF        0x0A        // line feed
#define ASCII_VT        0x0B        // vertical tab
#define ASCII_FF        0x0C        // form feed (new page)
#define ASCII_CR        0x0D        // carriage return

#define ASCII_ESC       0x1B        // escape

#define ASCII_SPACE     0x20        // space
#define ASCII_HASH      0x23        // #
#define ASCII_LB        0x28        // (
#define ASCII_RB        0x29        // )
#define ASCII_COMMA     0x2C        // ,
#define ASCII_PERIOD    0x2E        // .
#define ASCII_SLASH     0x2f        // /

#define ASCII_SEMI      0x3B        // ;

#define ASCII_LSB       0x5B        // [
#define ASCII_BSLASH    0x5C        // back slash
#define ASCII_RSB       0x5D        // ]


#define WIDTH_MAX       768         // maximum width  (ReGIS maximum 768)
#define HEIGHT_MAX      480         // desired height (ReGIS maximum 480)



/* offset direction */
typedef enum offset_e
{
    EE  = 0,
    NE  = 1,
    NN  = 2,
    NW  = 3,
    WW  = 4,
    SW  = 5,
    SS  = 6,
    SE  = 7
} offset_t;

/* writing mode */
typedef enum mode_e
{
    REP = 0,            // (R) Replace Mode (standard mode)
    ERA = 1,            // (E) Erase Mode - NOR
    OVL = 2,            // (V) Overlay Mode - OR
    CPL = 3             // (C) Complement Mode - XOR
} mode_t;

/* writing pattern */
typedef enum pattern_e
{
    P0  = 0,            // blank
    P1  = 1,            // solid
    P2  = 2,            // dash
    P3  = 3,            // dit dash
    P4  = 4,            // dot
    P5  = 5,            // dit dit dash
    P6  = 6,
    P7  = 7,
    P8  = 8,
    P9  = 9
} pattern_t;

/* intensity (colour) I(c) */
typedef enum intensity_e
{
    D   = 0,            // Dark (black)
    B   = 1,            // Blue
    R   = 2,            // Red
    M   = 3,            // Magenta
    G   = 4,            // Green
    C   = 5,            // Cyan
    Y   = 6,            // Yellow
    W   = 7             // White
} intensity_t;

/* Structure to use when opening a window - as per usual,if type <> 0
 * then open graphics window number with width (in pixels) width.
 */

typedef struct window_s {
    char * command;     // active command string terminated in null

    uint16_t x;         // current cursor position
    uint16_t y;         // current cursor position

    uint16_t width;     // desired window width  (ReGIS maximum 768)
    uint16_t height;    // desired window height (ReGIS maximum 480)

    pattern_t pattern;  // current writing pattern
    mode_t mode;        // current writing mode
} window_t;

/****************************************************************************/
/***        Function Definitions                                          ***/
/****************************************************************************/

/* Open a graphics window, in graphics mode, and inititialise graphics */
__OPROTO(,,uint8_t,,window_new,window_t * win,uint16_t width,uint16_t height)

/* Reset a graphics window, clear command string */
__OPROTO(,,void,,window_reset,window_t * win)

/* Write out instructions */
__OPROTO(,,void,,window_write,window_t * win)

/* Clear window */
__OPROTO(,,void,,window_clear,window_t * win)

/* Close a graphics window, return to text mode */
__OPROTO(,,void,,window_close,window_t * win)

/* Set writing mode */
__OPROTO(,,void,,draw_mode,window_t * win,mode_t mode)

/* Set writing pattern */
__OPROTO(,,void,,draw_pattern,window_t * win,pattern_t pattern)

/* Set writing intensity (colour) */
__OPROTO(,,void,,draw_intensity,window_t * win,intensity_t intensity)

/* Relative move position */
__OPROTO(,,void,,draw_rel,window_t * win,int16_t dx,int16_t dy)

/* Relative move offset direction */
__OPROTO(,,void,,draw_ofs,window_t * win,uint16_t d,offset_t offset)

/* Set absolute position */
__OPROTO(,,void,,draw_abs,window_t * win,uint16_t x,uint16_t y)

/* Draw a pixel to screen at current position */
__OPROTO(,,void,,draw_pixel_rel,window_t * win)

/* Erase a pixel at current position */
__OPROTO(,,void,,draw_unpixel_rel,window_t * win)

/* Draw a pixel at absolute location */
__OPROTO(,,void,,draw_pixel_abs,window_t * win,uint16_t x,uint16_t y)

/* Erase a pixel at absolute location */
__OPROTO(,,void,,draw_unpixel_abs,window_t * win,uint16_t x,uint16_t y)

/* Draw a line to relative position */
__OPROTO(,,void,,draw_line_rel,window_t * win,int16_t dx,int16_t dy)

/* Erase a line from current position */
__OPROTO(,,void,,draw_unline_rel,window_t * win,int16_t dx,int16_t dy)

/* Draw a line to absolute location */
__OPROTO(,,void,,draw_line_abs,window_t * win,uint16_t x,uint16_t y)

/* Erase a line to absolute location */
__OPROTO(,,void,,draw_unline_abs,window_t * win,uint16_t x,uint16_t y)

/* Draw a box from current position */
__OPROTO(,,void,,draw_box,window_t * win,int16_t width,int16_t height)

/* Erase a box from current position */
__OPROTO(,,void,,draw_unbox,window_t * win,int16_t width,int16_t height)

/* Draw a filled box from current position */
__OPROTO(,,void,,draw_box_fill,window_t * win,int16_t width,int16_t height)

/* Erase a filled box from current position */
__OPROTO(,,void,,draw_unbox_fill,window_t * win,int16_t width,int16_t height)

/* Draw a circle, centred on current position */
__OPROTO(,,void,,draw_circle,window_t * win,uint16_t radius)

/* Erase a circle, centred on current position */
__OPROTO(,,void,,draw_uncircle,window_t * win,uint16_t radius)

/* Draw a circle filled, centred on current position */
__OPROTO(,,void,,draw_circle_fill,window_t * win,uint16_t radius)

/* Erase a circle filled, centred on current position */
__OPROTO(,,void,,draw_uncircle_fill,window_t * win,uint16_t radius)

/* Draw an arc (circle) in anticlockwise degrees (0 - 360), centred on current position */
__OPROTO(,,void,,draw_arc,window_t * win,uint16_t radius,int16_t arc)

/* Erase an arc (circle) in anticlockwise degrees (0 - 360), centred on current position */
__OPROTO(,,void,,draw_unarc,window_t * win,uint16_t radius,int16_t arc)

/* Draw text from current position */
__OPROTO(,,void,,draw_text,window_t * win,char * text,uint8_t size)

/* Draw custom ReGIS from current position */
__OPROTO(,,void,,draw_free,window_t * win,char * text)

#ifdef __cplusplus
}
#endif

#endif  /* _REGIS_H  */