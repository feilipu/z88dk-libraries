/***************************************************************************/
/*                                                                         */
/*  3D Rotating Cube. Created for Amiga and PC by Stefan Henrikson 1993.   */
/*                                                                         */
/*  Modified for Z88 BASIC 1993, SmallC+ Z88 1998 by Dennis Groning.       */
/*                                                                         */
/*  Modified for z88dk yaz180 i2c_lcd by Phillip Stevens July 2020         */
/*                                                                         */
/***************************************************************************/

// ZSDCC CPM

// zcc +yaz180 -subtype=cpm -SO3 -v -m --list --math32 -llib/yaz180/i2c_lcd --c-code-in-asm --max-allocs-per-node100000 cube.c -o cube -create-app

// zcc +yaz180 -subtype=app -SO3 -v -m --list --math32 -llib/yaz180/i2c_lcd --c-code-in-asm --max-allocs-per-node100000 cube.c -o cube -create-app

// SCCZ80
// zcc +yaz180 -subtype=app -clib=new -v -m --list --math32 -llib/yaz180/i2c_lcd cube.c -o cube -create-app

// SCCZ80 IEEE 16-Bit Maths
// zcc +yaz180 -subtype=app -clib=new -v -m --list --math32 --math16 -llib/yaz180/i2c_lcd cube.c -o cube16 -create-app

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <cpu.h>

#include <stdio.h>

#include <time.h>
#include <sys/time.h>

#include <math.h>

#include <arch/yaz180.h>
#include <arch/yaz180/i2c.h>
#include <arch/yaz180/system_time.h>

#include <lib/yaz180/i2c_lcd.h>

/////////////////////////////////////////////////////////////////////////

#pragma printf = "%u %x %li %lu" // enables %u %li %lu only

#pragma output noprotectmsdos
#pragma output noredir

/////////////////////////////////////////////////////////////////////////

#ifdef __MATH_MATH16
  #define FLOAT _Float16
  #define SIN(x) sinf16(x)
  #define COS(x) cosf16(x)
#else
  #define FLOAT float
  #define SIN(x) sin(x)
  #define COS(x) cos(x)
#endif

#define MAX_X   128
#define MAX_Y    64
#define NODES     8
#define SIZE     16

#define MAX_X2  (MAX_X/2)
#define MAX_Y2  (MAX_Y/2)

extern uint8_t LCD_Port;

int main()
{
    FLOAT x[NODES], y[NODES], z[NODES];
    FLOAT vx=0, vy=0, vz=0;
    FLOAT xg[NODES], yg[NODES], zg[NODES];
    FLOAT mx=0, my=0;
    FLOAT cx,cy,cz,sx,sy,sz;
    FLOAT t1,t2,t3;

    uint8_t node;

//  struct timespec startTime, endTime, resTime;

    LCD_Init(I2C2_PORT);

//  printf("LCD Cube on I2C port: %u\n", (LCD_Port == I2C1_PORT ? 1 : 2));

    x[0]=-SIZE; y[0]=-SIZE; z[0]=-SIZE;
    x[1]=-SIZE; y[1]= SIZE; z[1]=-SIZE;
    x[2]= SIZE; y[2]= SIZE; z[2]=-SIZE;
    x[3]= SIZE; y[3]=-SIZE; z[3]=-SIZE;
    x[4]=-SIZE; y[4]=-SIZE; z[4]= SIZE;
    x[5]=-SIZE; y[5]= SIZE; z[5]= SIZE;
    x[6]= SIZE; y[6]= SIZE; z[6]= SIZE;
    x[7]= SIZE; y[7]=-SIZE; z[7]= SIZE;

    for(node=0;node<NODES;++node) {
            xg[node]=x[node];
            yg[node]=y[node];
            zg[node]=z[node];
    }

//  clock_gettime(CLOCK_REALTIME, &startTime);

    do {
        cx=COS(vx); cy=COS(vy); sx=SIN(vx); sy=SIN(vy);
        cz=COS(vz); sz=SIN(vz);

        LCD_DrawLineAt(x[0]+mx, x[1]+mx, y[0]+my, y[1]+my, WHITE);
        LCD_DrawLineAt(x[1]+mx, x[2]+mx, y[1]+my, y[2]+my, WHITE);
        LCD_DrawLineAt(x[2]+mx, x[3]+mx, y[2]+my, y[3]+my, WHITE);
        LCD_DrawLineAt(x[3]+mx, x[0]+mx, y[3]+my, y[0]+my, WHITE);
        LCD_DrawLineAt(x[4]+mx, x[5]+mx, y[4]+my, y[5]+my, WHITE);
        LCD_DrawLineAt(x[5]+mx, x[6]+mx, y[5]+my, y[6]+my, WHITE);
        LCD_DrawLineAt(x[6]+mx, x[7]+mx, y[6]+my, y[7]+my, WHITE);
        LCD_DrawLineAt(x[7]+mx, x[4]+mx, y[7]+my, y[4]+my, WHITE);
        LCD_DrawLineAt(x[0]+mx, x[4]+mx, y[0]+my, y[4]+my, WHITE);
        LCD_DrawLineAt(x[3]+mx, x[7]+mx, y[3]+my, y[7]+my, WHITE);
        LCD_DrawLineAt(x[2]+mx, x[6]+mx, y[2]+my, y[6]+my, WHITE);
        LCD_DrawLineAt(x[1]+mx, x[5]+mx, y[1]+my, y[5]+my, WHITE);

        mx=(MAX_X2-SIZE*1.8)*COS(vx)+MAX_X2;
        my=(MAX_Y2-SIZE*1.8)*SIN(vy)+MAX_Y2;

        for(node=0;node<NODES;++node) {

            t1=yg[node]*cx-zg[node]*sx;
            t2=yg[node]*sx+zg[node]*cx;
            t3=xg[node]*cy;
            x[node] = (t3 + t2*sy)*cz;
            x[node] = x[node] - t1*sz;

            y[node] = (t3 + t2*sy)*sz;
            y[node] = y[node] + t1*cz;

            z[node]=-xg[node]*sy+t2*cy;
        }

        LCD_DrawLineAt(x[0]+mx, x[1]+mx, y[0]+my, y[1]+my, BLACK);
        LCD_DrawLineAt(x[1]+mx, x[2]+mx, y[1]+my, y[2]+my, BLACK);
        LCD_DrawLineAt(x[2]+mx, x[3]+mx, y[2]+my, y[3]+my, BLACK);
        LCD_DrawLineAt(x[3]+mx, x[0]+mx, y[3]+my, y[0]+my, BLACK);
        LCD_DrawLineAt(x[4]+mx, x[5]+mx, y[4]+my, y[5]+my, BLACK);
        LCD_DrawLineAt(x[5]+mx, x[6]+mx, y[5]+my, y[6]+my, BLACK);
        LCD_DrawLineAt(x[6]+mx, x[7]+mx, y[6]+my, y[7]+my, BLACK);
        LCD_DrawLineAt(x[7]+mx, x[4]+mx, y[7]+my, y[4]+my, BLACK);
        LCD_DrawLineAt(x[0]+mx, x[4]+mx, y[0]+my, y[4]+my, BLACK);
        LCD_DrawLineAt(x[3]+mx, x[7]+mx, y[3]+my, y[7]+my, BLACK);
        LCD_DrawLineAt(x[2]+mx, x[6]+mx, y[2]+my, y[6]+my, BLACK);
        LCD_DrawLineAt(x[1]+mx, x[5]+mx, y[1]+my, y[5]+my, BLACK);

        vx+=0.03; vy+=0.05; vz+=0.02;   

        cpu_delay_ms( 92 );

//      clock_gettime(CLOCK_REALTIME,&endTime);
//      timersub(&endTime, &startTime, &resTime);

//      printf("Elapsed: %li sec %lu msec\n", resTime.tv_sec, resTime.tv_nsec/1000000 );
//      memcpy((void *)startTime, (void *)endTime, sizeof(endTime));
        
    } while (1);
    return 0;
}
