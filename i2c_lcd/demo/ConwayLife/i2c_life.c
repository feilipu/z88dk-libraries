/*
    Title:          i2c_life.c
    Date Created:   16/10/2019
    Last Modified:  28/10/2019
    Target:         Zilog Z180
    Environment:    z88dk
    Purpose:        Drive Sparking I2C LCD Display
    Application:    Conway's Life
    Author:         Phillip Stevens

    Adapted for the Seeed Studio Sparking I2C LCD.
    Distributed under the terms of the GNU General Public License, please see below.

    Portions Copyright 2008 Windell H. Oskay

    Distributed under the terms of the GNU General Public License, please see below.
    Adapted from a program by David Gustafik, and re-released under the GPL. :)
    We'd really like to see improved versions of this software!

    Portions Copyright David Gustafik, 2007

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

*/

// ZSDCC
// zcc +yaz180 -subtype=app -v -m -SO3 --list --math32 -llib/yaz180/i2c_lcd --c-code-in-asm --max-allocs-per-node200000 i2c_life.c -o i2c_life -create-app

// zcc +yaz180 -subtype=cpm -v -m -SO3 --list --math32 -llib/yaz180/i2c_lcd --c-code-in-asm --max-allocs-per-node200000 i2c_life.c -o i2c_life -create-app


// SCCZ80
// zcc +yaz180 -subtype=app -clib=new -v -m -SO3 --list --math32 -llib/yaz180/i2c_lcd i2c_life.c -o i2c_life -create-app

/////////////////////////////////////////////////////////////////////////

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <time.h>
#include <sys/time.h>

#include <arch/yaz180.h>
#include <arch/yaz180/i2c.h>
#include <arch/yaz180/system_time.h>

#include <lib/yaz180/i2c_lcd.h>

/////////////////////////////////////////////////////////////////////////

#pragma printf = "%li %lu %s %u" // enables %li %lu %s, %u only

/////////////////////////////////////////////////////////////////////////

// Define the size of the screen in pixels
// Origin [0,0] is top left of screen

#define CELLS_X LCD_X_SIZE_MAX
#define CELLS_Y LCD_Y_SIZE_MAX

#define CELL_TOTAL_BYTES (CELLS_X*CELLS_Y/8)

/////////////////////////////////////////////////////////////////////////

extern uint8_t LCD_Port;    // Global for PCA9665 Device Port MSB

static uint8_t current_generation[CELL_TOTAL_BYTES];
static uint8_t old_generation[CELL_TOTAL_BYTES];

/////////////////////////////////////////////////////////////////////////

uint8_t get_cell(uint8_t *from, uint8_t x, uint8_t y)
{
    x &= (uint8_t)(CELLS_X-1);   // ensure x < 128 this is non-portable (but fast)
    y &= (uint8_t)(CELLS_Y-1);   // ensure y < 64 this is non-portable (but fast)

    return (from[x + (y>>3)*CELLS_X] & (uint8_t)(1 << y%8)) ? 1 : 0;
}

/////////////////////////////////////////////////////////////////////////

uint16_t get_total(uint8_t *from)
{
    uint16_t total = 0;

    for (uint8_t x=0; x < CELLS_X; ++x)
    {
        for (uint8_t y=0; y < CELLS_Y; ++y)
        {
            if (get_cell(from,x,y)) ++total;
        }
    }
    return total;
}

/////////////////////////////////////////////////////////////////////////

void set_cell(uint8_t *to, uint8_t x, uint8_t y, uint8_t value)
{
    if (value)
        to[x + (y>>3)*CELLS_X] |= (uint8_t)(1 << y%8);
    else
        to[x + (y>>3)*CELLS_X] &= (uint8_t)~(1 << y%8);
}

/////////////////////////////////////////////////////////////////////////

void fill_cell(uint8_t *to, uint8_t x, uint8_t y)
{
    to[x + (y>>3)*CELLS_X] |= (uint8_t)(1 << y%8);
}

/////////////////////////////////////////////////////////////////////////

void clear_cell(uint8_t *to, uint8_t x, uint8_t y)
{
    to[x + (y>>3)*CELLS_X] &= (uint8_t)~(1 << y%8);
}

/////////////////////////////////////////////////////////////////////////

void fill_random_data(uint8_t *to)
{
    // random seed generator is fractions of a second
    srand( (uint16_t)_system_time ^ _system_time_fraction );

    for (uint16_t cell = 0; cell < CELL_TOTAL_BYTES; ++cell)
    {
           to[cell] = (uint8_t)rand();
    }
}

/////////////////////////////////////////////////////////////////////////

void clear_data(uint8_t *to)
{
    memset( to, 0, CELL_TOTAL_BYTES );
}

/////////////////////////////////////////////////////////////////////////

void copy_old_new(uint8_t *to, uint8_t *from)
{
    memcpy_far( to, 0, from, 0, CELL_TOTAL_BYTES );
}

/////////////////////////////////////////////////////////////////////////

uint8_t get_neighbours(uint8_t *from, uint8_t x, uint8_t y)
{
    return   get_cell(from,x-1,y-1) + get_cell(from,x-1,y) + get_cell(from,x-1,y+1) \
           + get_cell(from,x  ,y-1) +                        get_cell(from,x  ,y+1) \
           + get_cell(from,x+1,y-1) + get_cell(from,x+1,y) + get_cell(from,x+1,y+1);
}

/////////////////////////////////////////////////////////////////////////

uint8_t get_difference(uint8_t *a,uint8_t *b)
{
    uint16_t diff=0;

    for (uint8_t  x=0; x < CELLS_X; ++x)
    {
        for (uint8_t  y=0; y < CELLS_Y; ++y)
        {
            if ((get_cell(a,x,y) && !get_cell(b,x,y)) || (!get_cell(a,x,y) && get_cell(b,x,y)))
                ++diff;
        }
    }
    return diff;
}

/////////////////////////////////////////////////////////////////////////

void display(uint8_t *from)
{
    LCD_WriteSeriesDispRAM( &from[CELLS_X*0], CELLS_X, 0, 0);
    LCD_WriteSeriesDispRAM( &from[CELLS_X*1], CELLS_X, 0, 1);
    LCD_WriteSeriesDispRAM( &from[CELLS_X*2], CELLS_X, 0, 2);
    LCD_WriteSeriesDispRAM( &from[CELLS_X*3], CELLS_X, 0, 3);
    LCD_WriteSeriesDispRAM( &from[CELLS_X*4], CELLS_X, 0, 4);
    LCD_WriteSeriesDispRAM( &from[CELLS_X*5], CELLS_X, 0, 5);
    LCD_WriteSeriesDispRAM( &from[CELLS_X*6], CELLS_X, 0, 6);
    LCD_WriteSeriesDispRAM( &from[CELLS_X*7], CELLS_X, 0, 7);
}

/////////////////////////////////////////////////////////////////////////

void main(void)
{
    uint16_t generations = 0;
    
    struct timespec startTime, endTime, resTime;

    clock_gettime(CLOCK_REALTIME,&startTime);

    LCD_Init(I2C2_PORT);
    printf("LCD life on I2C port: %u\n", (LCD_Port == __IO_I2C1_PORT_MSB ? 1 : 2));

    LCD_WorkingModeConf(ON, ON, WM_RamMode);// Start RAM mode (characters won't work anymore).    

    fill_random_data(old_generation);
    display(old_generation);

    while (1)    //Main Loop
    {
        for (uint8_t x=0; x < CELLS_X; ++x)
        {
            for (uint8_t y=0; y < CELLS_Y; ++y)        // Inner loop; should be made quick as possible.
            {
                uint8_t neighbours;

                neighbours = get_neighbours(old_generation, x, y);

                if (neighbours < 2 || neighbours > 3)
                    clear_cell(current_generation, x, y);

                if (neighbours == 3)
                    fill_cell(current_generation, x, y);
            }
        }

        display(current_generation);
        printf("Generation: %u\n", ++generations );

        // Exit after a few generations
        if ( generations > 512 ) break;

#if 0
        // Boringness detector:
        if( get_difference(old_generation,current_generation) < 8 || get_total(current_generation) < 6)
        {
            current_generation[7]  = (uint8_t)rand();
            current_generation[8]  = (uint8_t)rand();
            current_generation[9]  = (uint8_t)rand();
            current_generation[10] = (uint8_t)rand();
            current_generation[11] = (uint8_t)rand();
            current_generation[12] = (uint8_t)rand();
            current_generation[13] = (uint8_t)rand();
            current_generation[14] = (uint8_t)rand();

            current_generation[1000] = (uint8_t)rand();
            current_generation[1001] = (uint8_t)rand();
            current_generation[1002] = (uint8_t)rand();
            current_generation[1003] = (uint8_t)rand();
            current_generation[1004] = (uint8_t)rand();
            current_generation[1005] = (uint8_t)rand();
            current_generation[1006] = (uint8_t)rand();
            current_generation[1007] = (uint8_t)rand();

            generations = 0;
        }
#endif
        copy_old_new(old_generation, current_generation);
    }

    clock_gettime(CLOCK_REALTIME,&endTime);
    timersub(&endTime, &startTime, &resTime);
    printf("Elapsed: %li sec %lu msec\n", resTime.tv_sec, resTime.tv_nsec/1000000 );
}

