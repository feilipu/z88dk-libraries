/*
 * regis.h
 *
 * Copyright (c) 2021-24 Phillip Stevens
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


#ifndef _REGIS_H
#define _REGIS_H

#include <stdint.h>
#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

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
    _EE  = 0,
    _NE  = 1,
    _NN  = 2,
    _NW  = 3,
    _WW  = 4,
    _SW  = 5,
    _SS  = 6,
    _SE  = 7
} offset_t;

/* writing mode */
typedef enum w_mode_e
{
    _REP = 0,           // (R) Replace Mode (standard mode)
    _ERA = 1,           // (E) Erase Mode - NOR
    _OVL = 2,           // (V) Overlay Mode - OR
    _CPL = 3            // (C) Complement Mode - XOR
} w_mode_t;

/* writing pattern */
typedef enum w_pattern_e
{
    _P0  = 0,           // blank
    _P1  = 1,           // solid
    _P2  = 2,           // dash
    _P3  = 3,           // dit dash
    _P4  = 4,           // dot
    _P5  = 5,           // dit dit dash
    _P6  = 6,
    _P7  = 7,
    _P8  = 8,
    _P9  = 9
} w_pattern_t;

/* intensity (colour) I(c) */
typedef enum w_intensity_e
{
    _D   = 0,           // Dark (black)
    _B   = 1,           // Blue
    _R   = 2,           // Red
    _M   = 3,           // Magenta
    _G   = 4,           // Green
    _C   = 5,           // Cyan
    _Y   = 6,           // Yellow
    _W   = 7            // White
} w_intensity_t;

/* Structure to use when opening a window - as per usual,if type <> 0
 * then open graphics window number with width (in pixels) width.
 */

typedef struct window_s {
    FILE * fp;          // display (output) device

    uint16_t width;     // desired window width  (ReGIS maximum 768)
    uint16_t height;    // desired window height (ReGIS maximum 480)
} window_t;

/****************************************************************************/
/***        Function Definitions                                          ***/
/****************************************************************************/

/* Open a graphics window, in graphics mode, and inititialise graphics */
uint8_t __LIB__ window_new(window_t * win,uint16_t width,uint16_t height,FILE * fp) __smallc;


/* Clear window */
void __LIB__ window_clear(window_t * win) __smallc;



/* Close a graphics window, return to text mode */
void __LIB__ window_close(window_t * win) __smallc;



/* Set writing mode */
void __LIB__ draw_mode(window_t * win,w_mode_t mode) __smallc;



/* Set writing pattern */
void __LIB__ draw_pattern(window_t * win,w_pattern_t pattern) __smallc;



/* Set writing intensity (colour) */
void __LIB__ draw_intensity(window_t * win,w_intensity_t intensity) __smallc;



/* Relative move position */
void __LIB__ draw_rel(window_t * win,int16_t dx,int16_t dy) __smallc;



/* Relative move offset direction */
void __LIB__ draw_ofs(window_t * win,uint16_t d,offset_t offset) __smallc;



/* Set absolute position */
void __LIB__ draw_abs(window_t * win,uint16_t x,uint16_t y) __smallc;



/* Draw a pixel to screen at current position */
void __LIB__ draw_pixel_rel(window_t * win) __smallc;



/* Erase a pixel at current position */
void __LIB__ draw_unpixel_rel(window_t * win) __smallc;



/* Draw a pixel at absolute location */
void __LIB__ draw_pixel_abs(window_t * win,uint16_t x,uint16_t y) __smallc;



/* Erase a pixel at absolute location */
void __LIB__ draw_unpixel_abs(window_t * win,uint16_t x,uint16_t y) __smallc;



/* Draw a line to relative position */
void __LIB__ draw_line_rel(window_t * win,int16_t dx,int16_t dy) __smallc;



/* Erase a line from current position */
void __LIB__ draw_unline_rel(window_t * win,int16_t dx,int16_t dy) __smallc;



/* Draw a line to absolute location */
void __LIB__ draw_line_abs(window_t * win,uint16_t x,uint16_t y) __smallc;



/* Erase a line to absolute location */
void __LIB__ draw_unline_abs(window_t * win,uint16_t x,uint16_t y) __smallc;



/* Draw a box from current position */
void __LIB__ draw_box(window_t * win,int16_t width,int16_t height) __smallc;



/* Erase a box from current position */
void __LIB__ draw_unbox(window_t * win,int16_t width,int16_t height) __smallc;



/* Draw a filled box from current position */
void __LIB__ draw_box_fill(window_t * win,int16_t width,int16_t height) __smallc;



/* Erase a filled box from current position */
void __LIB__ draw_unbox_fill(window_t * win,int16_t width,int16_t height) __smallc;



/* Draw a circle, centred on current position */
void __LIB__ draw_circle(window_t * win,uint16_t radius) __smallc;



/* Erase a circle, centred on current position */
void __LIB__ draw_uncircle(window_t * win,uint16_t radius) __smallc;



/* Draw a circle filled, centred on current position */
void __LIB__ draw_circle_fill(window_t * win,uint16_t radius) __smallc;



/* Erase a circle filled, centred on current position */
void __LIB__ draw_uncircle_fill(window_t * win,uint16_t radius) __smallc;



/* Draw an arc (circle) in anticlockwise degrees (0 - 360), centred on current position */
void __LIB__ draw_arc(window_t * win,uint16_t radius,int16_t arc) __smallc;



/* Erase an arc (circle) in anticlockwise degrees (0 - 360), centred on current position */
void __LIB__ draw_unarc(window_t * win,uint16_t radius,int16_t arc) __smallc;



/* Draw text from current position */
void __LIB__ draw_text(window_t * win,char const * text,uint8_t size) __smallc;



/* Draw custom ReGIS from current position */
void __LIB__ draw_free(window_t * win,char const * text) __smallc;



#ifdef __cplusplus
}
#endif

#endif  /* _REGIS_H  */
