/* Sample application to demonstrate FT800 primitives, widgets and customized screen shots */


#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

/* time interface include file */
#include <time.h>
#include <sys/time.h>

#include <arch/yaz180.h>
#include <arch/yaz180/i2c.h>

/* Scheduler include files. */
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

/* Gameduino 2 include file. */
#include "GPU_API.h"
#include "X11_RGB.h"

#include "GPU_SampleApp.h"

#define GPU_APP_DELAY_BTW_APIS            (1000)
#define GPU_APP_ENABLE_DELAY()             (vTaskDelay( GPU_APP_DELAY_BTW_APIS / portTICK_PERIOD_MS ))
#define GPU_APP_ENABLE_DELAY_MS(x)         (vTaskDelay( x / portTICK_PERIOD_MS ))

/* Globals used for GPU HAL management */

extern GPU_HAL_Context_t        theScreen;
extern GPU_HAL_Context_t        *pScreen;

#if defined(GPU_APP_ENABLE_APIS_SET0) || defined(GPU_APP_ENABLE_APIS_SET1)|| defined(GPU_APP_ENABLE_APIS_SET2) || defined(GPU_APP_ENABLE_APIS_SET4)
extern GPU_API_Bitmap_header_t  Lena_Bitmap_RawData_Header[];
extern ft_const_uint8_t         Lena_Bitmap_RawData[];
#endif

#ifdef GPU_APP_ENABLE_APIS_SET2
extern ft_const_uint8_t Lenaface40[];
extern ft_const_uint8_t Mandrill256[];
#endif

#ifdef GPU_APP_ENABLE_APIS_SET3
extern ft_const_uint8_t Roboto_BoldCondensed_12[];
#endif

#ifdef GPU_APP_ENABLE_APIS_SET0

/******************************************************************************/
/* Example code to display few points at various offsets with various colours */
/******************************************************************************/
ft_void_t    GPU_APP_Points()
{
    /* Construct DL of points */
    GPU_HAL_Wr32(pScreen, RAM_DL + 0 , CLEAR_COLOR_RGB(128,128,128));
    GPU_HAL_Wr32(pScreen, RAM_DL + 4 , CLEAR(1,1,1));
    GPU_HAL_Wr32(pScreen, RAM_DL + 8 , COLOR_RGB(128, 0, 0) );
    GPU_HAL_Wr32(pScreen, RAM_DL + 12, POINT_SIZE(5 * 16) );
    GPU_HAL_Wr32(pScreen, RAM_DL + 16, BEGIN(POINTS) );
    GPU_HAL_Wr32(pScreen, RAM_DL + 20, VERTEX2F((DispWidth/5) * 16, (DispHeight/2) * 16) );
    GPU_HAL_Wr32(pScreen, RAM_DL + 24, COLOR_RGB(0, 128, 0) );
    GPU_HAL_Wr32(pScreen, RAM_DL + 28, POINT_SIZE(15 * 16) );
    GPU_HAL_Wr32(pScreen, RAM_DL + 32, VERTEX2F((DispWidth*2/5) * 16, (DispHeight/2) * 16) );
    GPU_HAL_Wr32(pScreen, RAM_DL + 36, COLOR_RGB(0, 0, 128) );
    GPU_HAL_Wr32(pScreen, RAM_DL + 40, POINT_SIZE(25 * 16) );
    GPU_HAL_Wr32(pScreen, RAM_DL + 44, VERTEX2F((DispWidth*3/5) * 16, (DispHeight/2) * 16) );
    GPU_HAL_Wr32(pScreen, RAM_DL + 48, COLOR_RGB(128, 128, 0) );
    GPU_HAL_Wr32(pScreen, RAM_DL + 52, POINT_SIZE(35 * 16) );
    GPU_HAL_Wr32(pScreen, RAM_DL + 56, VERTEX2F((DispWidth*4/5) * 16, (DispHeight/2) * 16) );
    GPU_HAL_Wr32(pScreen, RAM_DL + 60, DISPLAY()); // display the image

    /* Do a swap */
    GPU_API_GPU_DLSwap(DLSWAP_FRAME);
    GPU_APP_ENABLE_DELAY();
}

ft_void_t GPU_APP_Lines()
{
    ft_int16_t LineHeight = 25;

    GPU_API_Write_DLCmd(CLEAR(1, 1, 1)); // clear screen
    GPU_API_Write_DLCmd(COLOR_RGB(128, 0, 0) );
    GPU_API_Write_DLCmd(LINE_WIDTH(5 * 16) );
    GPU_API_Write_DLCmd(BEGIN(LINES) );
    GPU_API_Write_DLCmd(VERTEX2F((DispWidth/4) * 16,((DispHeight - LineHeight)/2) * 16) );
    GPU_API_Write_DLCmd(VERTEX2F((DispWidth/4) * 16,((DispHeight + LineHeight)/2) * 16) );
    GPU_API_Write_DLCmd(COLOR_RGB(0, 128, 0) );
    GPU_API_Write_DLCmd(LINE_WIDTH(10 * 16) );
    LineHeight = 40;
    GPU_API_Write_DLCmd(VERTEX2F((DispWidth*2/4) * 16,((DispHeight - LineHeight)/2) * 16) );
    GPU_API_Write_DLCmd(VERTEX2F((DispWidth*2/4) * 16,((DispHeight + LineHeight)/2) * 16) );
    GPU_API_Write_DLCmd(COLOR_RGB(128, 128, 0) );
    GPU_API_Write_DLCmd(LINE_WIDTH(20 * 16) );
    LineHeight = 55;
    GPU_API_Write_DLCmd(VERTEX2F((DispWidth*3/4) * 16, ((DispHeight - LineHeight)/2) * 16) );
    GPU_API_Write_DLCmd(VERTEX2F((DispWidth*3/4) * 16, ((DispHeight + LineHeight)/2) * 16) );
    GPU_API_Write_DLCmd(DISPLAY() );

    /* Reset the DL Buffer index, set for the next group of DL commands */
    GPU_API_Reset_DLBuffer();

    /* Do a swap */
    GPU_API_GPU_DLSwap(DLSWAP_FRAME);
    GPU_APP_ENABLE_DELAY();
}
ft_void_t    GPU_APP_Rectangles()
{
    ft_int16_t RectWidth,RectHeight;

    GPU_API_Write_DLCmd(CLEAR(1, 1, 1)); // clear screen
    GPU_API_Write_DLCmd(COLOR_RGB(0, 0, 128) );
    GPU_API_Write_DLCmd(LINE_WIDTH(1 * 16) );//LINE_WIDTH is used for corner curvature
    GPU_API_Write_DLCmd(BEGIN(RECTS) );
    RectWidth = 5;RectHeight = 25;
    GPU_API_Write_DLCmd(VERTEX2F( ((DispWidth/4) - (RectWidth/2)) * 16,((DispHeight - RectHeight)/2) * 16) );
    GPU_API_Write_DLCmd(VERTEX2F( ((DispWidth/4) + (RectWidth/2)) * 16,((DispHeight + RectHeight)/2) * 16) );
    GPU_API_Write_DLCmd(COLOR_RGB(0, 128, 0) );
    GPU_API_Write_DLCmd(LINE_WIDTH(5 * 16) );
    RectWidth = 10;RectHeight = 40;
    GPU_API_Write_DLCmd(VERTEX2F( ((DispWidth*2/4) - (RectWidth/2)) * 16,((DispHeight - RectHeight)/2) * 16) );
    GPU_API_Write_DLCmd(VERTEX2F( ((DispWidth*2/4) + (RectWidth/2)) * 16,((DispHeight + RectHeight)/2) * 16) );
    GPU_API_Write_DLCmd(COLOR_RGB(128, 128, 0) );
    GPU_API_Write_DLCmd(LINE_WIDTH(10 * 16) );
    RectWidth = 20;RectHeight = 55;
    GPU_API_Write_DLCmd(VERTEX2F( ((DispWidth*3/4) - (RectWidth/2)) * 16,((DispHeight - RectHeight)/2) * 16) );
    GPU_API_Write_DLCmd(VERTEX2F( ((DispWidth*3/4) + (RectWidth/2)) * 16,((DispHeight + RectHeight)/2) * 16) );
    GPU_API_Write_DLCmd(DISPLAY() );

    /* Reset the DL Buffer index, set for the next group of DL commands */
    GPU_API_Reset_DLBuffer();

    /* Do a swap */
    GPU_API_GPU_DLSwap(DLSWAP_FRAME);
    GPU_APP_ENABLE_DELAY();
}

/* Display lena face at different locations with various colours, -ve offsets, alpha blend etc */
ft_void_t    GPU_APP_Bitmap()
{
    GPU_API_Bitmap_header_t *p_bmhdr;
    ft_int16_t BMoffsetx,BMoffsety;

    p_bmhdr = (GPU_API_Bitmap_header_t *)&Lena_Bitmap_RawData_Header[0];
    /* Copy raw data into address 0 followed by generation of bitmap */
    GPU_HAL_WrMem(pScreen, RAM_G, &Lena_Bitmap_RawData[p_bmhdr->ArrayOffset], p_bmhdr->Stride*p_bmhdr->Height);

    GPU_API_Write_DLCmd(CLEAR(1, 1, 1)); // clear screen
    GPU_API_Write_DLCmd(COLOR_RGB(255,255,255));
    GPU_API_Write_DLCmd(BITMAP_SOURCE(RAM_G));
    GPU_API_Write_DLCmd(BITMAP_LAYOUT(p_bmhdr->Format, p_bmhdr->Stride, p_bmhdr->Height));
    GPU_API_Write_DLCmd(BITMAP_SIZE(NEAREST, BORDER, BORDER, p_bmhdr->Width, p_bmhdr->Height));
    GPU_API_Write_DLCmd(BEGIN(BITMAPS)); // start drawing bitmaps
    BMoffsetx = ((DispWidth/4) - (p_bmhdr->Width/2));
    BMoffsety = ((DispHeight/2) - (p_bmhdr->Height/2));
    GPU_API_Write_DLCmd(VERTEX2II(BMoffsetx, BMoffsety, 0, 0));
    GPU_API_Write_DLCmd(COLOR_RGB(255, 64, 64)); // red at (200, 120)
    BMoffsetx = ((DispWidth*2/4) - (p_bmhdr->Width/2));
    BMoffsety = ((DispHeight/2) - (p_bmhdr->Height/2));
    GPU_API_Write_DLCmd(VERTEX2II(BMoffsetx, BMoffsety, 0, 0));
    GPU_API_Write_DLCmd(COLOR_RGB(64, 180, 64)); // green at (216, 136)
    BMoffsetx += (p_bmhdr->Width/2);
    BMoffsety += (p_bmhdr->Height/2);
    GPU_API_Write_DLCmd(VERTEX2II(BMoffsetx, BMoffsety, 0, 0));
    GPU_API_Write_DLCmd(COLOR_RGB(255, 255, 64)); // transparent yellow at (232, 152)
    GPU_API_Write_DLCmd(COLOR_A(150));
    BMoffsetx += (p_bmhdr->Width/2);
    BMoffsety += (p_bmhdr->Height/2);
    GPU_API_Write_DLCmd(VERTEX2II(BMoffsetx, BMoffsety, 0, 0));
    GPU_API_Write_DLCmd(COLOR_A(255));
    GPU_API_Write_DLCmd(COLOR_RGB(255,255,255));
    GPU_API_Write_DLCmd(VERTEX2F(-10*16, -10*16));//for -ve coordinates use vertex2f instruction
    GPU_API_Write_DLCmd(END());
    GPU_API_Write_DLCmd(DISPLAY() );

    /* Reset the DL Buffer index, set for the next group of DL commands */
    GPU_API_Reset_DLBuffer();

    /* Do a swap */
    GPU_API_GPU_DLSwap(DLSWAP_FRAME);
    GPU_APP_ENABLE_DELAY();
}

/* inbuilt font example for proportional and non proportional text - hello world */
ft_void_t    GPU_APP_Fonts()
{
    ft_int32_t i,j,hoffset,voffset,stringlen;
    ft_uint32_t FontTableAddress;
    ft_const_uint8_t Display_string[] = "Hello World";
    GPU_Fonts_t Display_fontstruct;

    hoffset = ((DispWidth - 100)/2);voffset = DispHeight/2;

    /* Read the font address from 0xFFFFC location */
    FontTableAddress = GPU_HAL_Rd32(pScreen, 0xFFFFC);
        stringlen = sizeof(Display_string) - 1;
    for(i=0;i<16;i++)
    {
        /* Read the font table from hardware */
        GPU_HAL_RdMem(pScreen,(FontTableAddress + i*GPU_FONT_TABLE_SIZE),(ft_uint8_t *)&Display_fontstruct,GPU_FONT_TABLE_SIZE);

        GPU_API_Write_DLCmd(CLEAR(1, 1, 1)); // clear screen
        GPU_API_Write_DLCmd(COLOR_RGB(255, 255, 255)); // clear screen

        /* Display string at the centre of display */
        GPU_API_Write_DLCmd(BEGIN(BITMAPS));
        hoffset = ((DispWidth - 120)/2);
        voffset = ((DispHeight - Display_fontstruct.FontHeightInPixels)/2);

        /* Display hello world by off setting with respect to char size */
        for(j=0;j<stringlen;j++)
        {
            GPU_API_Write_DLCmd(VERTEX2II(hoffset,voffset,(i+16),Display_string[j]));
            hoffset += Display_fontstruct.FontWidth[Display_string[j]];
        }
        GPU_API_Write_DLCmd(END());
        GPU_API_Write_DLCmd(DISPLAY() );

        /* Reset the DL Buffer index, set for the next group of DL commands */
        GPU_API_Reset_DLBuffer();

        /* Do a swap */
        GPU_API_GPU_DLSwap(DLSWAP_FRAME);
        GPU_APP_ENABLE_DELAY_MS(500);
    }
}
/* display text8x8 of abcdefgh */
ft_void_t    GPU_APP_Text8x8()
{
    /* Write the data into RAM_G */
    ft_const_uint8_t Text_Array[] = "abcdefgh";
    ft_int32_t String_size,hoffset = 16,voffset = 16;


        String_size = sizeof(Text_Array) - 1;
    GPU_HAL_WrMem(pScreen,RAM_G,(ft_uint8_t *)Text_Array, String_size);

    /*
          abcdefgh
          abcdefgh
    */

    GPU_API_Write_DLCmd(CLEAR(1, 1, 1)); // clear screen
    GPU_API_Write_DLCmd(BITMAP_SOURCE(RAM_G));
    GPU_API_Write_DLCmd(BITMAP_LAYOUT(TEXT8X8, 1*8,1));//L1 format, each input data element is in 1 byte size
    GPU_API_Write_DLCmd(BITMAP_SIZE(NEAREST, BORDER, REPEAT, 8*8, 8*2));//output is 8x8 format - draw 8 characters in horizontal repeated in 2 line

    GPU_API_Write_DLCmd(BEGIN(BITMAPS));
    /* Display text 8x8 at hoffset, voffset location */
    GPU_API_Write_DLCmd(VERTEX2F(hoffset*16,voffset*16));

    /*
           abcdabcdabcdabcd
           efghefghefghefgh
    */
    GPU_API_Write_DLCmd(BITMAP_LAYOUT(TEXT8X8, 1*4,2));//L1 format and each datatype is 1 byte size
    GPU_API_Write_DLCmd(BITMAP_SIZE(NEAREST, REPEAT, BORDER, 8*16, 8*2));//each character is 8x8 in size -  so draw 32 characters in horizontal and 32 characters in vertical
    hoffset = DispWidth/2;
    voffset = DispHeight/2;
    GPU_API_Write_DLCmd(VERTEX2F(hoffset*16,voffset*16));
    GPU_API_Write_DLCmd(END());

    GPU_API_Write_DLCmd(DISPLAY() );

    /* Reset the DL Buffer index, set for the next group of DL commands */
    GPU_API_Reset_DLBuffer();

    /* Do a swap */
    GPU_API_GPU_DLSwap(DLSWAP_FRAME);
    GPU_APP_ENABLE_DELAY();
}

/* display textVGA of random values */
ft_void_t    GPU_APP_TextVGA()
{
    /* Write the data into RAM_G */
    ft_uint16_t Text_Array[160];
    ft_int32_t String_size,hoffset = 32,voffset = 32,i;

    srand(time( (void *)0 )); // set the seed based on the current system time.

    for(i=0;i<160;i++)
    {
        Text_Array[i] =  (uint16_t)rand();
    }

    String_size = 160*2;
    GPU_HAL_WrMem(pScreen,RAM_G,(ft_uint8_t*)Text_Array, String_size);


    GPU_API_Write_DLCmd(CLEAR(1, 1, 1)); // clear screen
    GPU_API_Write_DLCmd(BITMAP_SOURCE(RAM_G));

    /* mandatory for textvga as background color is also one of the parameter in textvga format */
    GPU_API_Write_DLCmd(BLEND_FUNC(ONE,ZERO));

    //draw 8x8
    GPU_API_Write_DLCmd(BITMAP_LAYOUT(TEXTVGA, 2*4,8));//L1 format, but each input data element is of 2 bytes in size
    GPU_API_Write_DLCmd(BITMAP_SIZE(NEAREST, BORDER, BORDER, 8*8, 8*8));//output is 8x8 format - draw 8 characters in horizontal and 8 vertical
    GPU_API_Write_DLCmd(BEGIN(BITMAPS));
    GPU_API_Write_DLCmd(VERTEX2F(hoffset*16,voffset*16));
    GPU_API_Write_DLCmd(END());

    //draw textvga
    GPU_API_Write_DLCmd(BITMAP_LAYOUT(TEXTVGA, 2*16,8));//L1 format but each datatype is 16bit size
    GPU_API_Write_DLCmd(BITMAP_SIZE(NEAREST, BORDER, REPEAT, 8*32, 8*32));//8 pixels per character and 32 rows/columns
    GPU_API_Write_DLCmd(BEGIN(BITMAPS));
    hoffset = DispWidth/2;
    voffset = DispHeight/2;
    /* Display textvga at hoffset, voffset location */
    GPU_API_Write_DLCmd(VERTEX2F(hoffset*16,voffset*16));
    GPU_API_Write_DLCmd(END());

    GPU_API_Write_DLCmd(DISPLAY() );

    /* Reset the DL Buffer index, set for the next group of DL commands */
    GPU_API_Reset_DLBuffer();

    /* Do a swap */
    GPU_API_GPU_DLSwap(DLSWAP_FRAME);
    GPU_APP_ENABLE_DELAY();
}


#ifdef GPU_APP_ENABLE_APIS_SET0
#define GRAPH_STRING        512

ft_void_t    GPU_APP_Bargraph()
{
    /* Write the data into RAM_G */
    ft_uint8_t Y_Array[GRAPH_STRING];
    ft_int32_t hoffset = 0, voffset = 0;

    hoffset = 0;
    voffset = 0;

    srand(time( (void *)0 )); // set the seed based on the current system time.

    for(uint16_t i=0; i<GRAPH_STRING; i++)
    {
        Y_Array[i] = (ft_uint8_t)rand() >> 1; // range from 0 to 128
    }

    GPU_HAL_WrMem(pScreen, RAM_G, (ft_uint8_t *)&Y_Array[0], GRAPH_STRING);

    GPU_API_Write_DLCmd(CLEAR_COLOR_RGB(255,255,255));
    GPU_API_Write_DLCmd(CLEAR(1, 1, 1)); // clear screen
    GPU_API_Write_DLCmd(BITMAP_SOURCE(RAM_G));
    GPU_API_Write_DLCmd(BITMAP_LAYOUT(BARGRAPH, 256,1));
    GPU_API_Write_DLCmd(COLOR_RGB(128, 0,0));
    GPU_API_Write_DLCmd(BITMAP_SIZE(NEAREST, BORDER, BORDER, 256, 256));
    GPU_API_Write_DLCmd(BEGIN(BITMAPS));
    /* Display text 8x8 at hoffset, voffset location */
    GPU_API_Write_DLCmd(VERTEX2II(hoffset,voffset,0,0));
    hoffset = 256;
    GPU_API_Write_DLCmd(VERTEX2II(hoffset,voffset,0,1));

    GPU_API_Write_DLCmd(DISPLAY() );

    /* Reset the DL Buffer index, set for the next group of DL commands */
    GPU_API_Reset_DLBuffer();

    /* Do a swap */
    GPU_API_GPU_DLSwap(DLSWAP_FRAME);
    GPU_APP_ENABLE_DELAY();

#if 0
    for(uint16_t i=0; i<GRAPH_STRING; i++)
    {
        ft_int32_t tmpval;
//      ft_int32_t tmpidx;

//        tmpidx = (i + 256)&(512 - 1);
//      tmpidx = i;
        tmpval = 128 + ((ft_int32_t)(i/3)*GPU_API_qsin(-65536*i/48)/65536);//within range

        Y_Array[i] = tmpval&0xff;
    }

    GPU_HAL_WrMem(pScreen,RAM_G,(ft_uint8_t *)Y_Array, GRAPH_STRING);

    GPU_API_Write_DLCmd(DISPLAY() );

    /* Reset the DL Buffer index, set for the next group of DL commands */
    GPU_API_Reset_DLBuffer();

    /* Do a swap */
    GPU_API_GPU_DLSwap(DLSWAP_FRAME);
    GPU_APP_ENABLE_DELAY();
#endif

    GPU_API_Write_DLCmd(CLEAR(1, 1, 1)); // clear screen
    GPU_API_Write_DLCmd(BITMAP_SOURCE(RAM_G));
    GPU_API_Write_DLCmd(BITMAP_LAYOUT(BARGRAPH, 256,1));
    GPU_API_Write_DLCmd(BITMAP_SIZE(NEAREST, BORDER, BORDER, 256, 256));
    GPU_API_Write_DLCmd(BEGIN(BITMAPS));
    GPU_API_Write_DLCmd(COLOR_RGB(0xff, 0, 0));
    /* Display bargraph at hoffset, voffset location */
    hoffset = 0;
    voffset = 0;
    GPU_API_Write_DLCmd(VERTEX2II(hoffset,voffset,0,0));
    hoffset = 256;
    voffset = 0;
    GPU_API_Write_DLCmd(VERTEX2II(hoffset,voffset,0,1));
    hoffset = 0;
    voffset += 4;
    GPU_API_Write_DLCmd(COLOR_RGB(0, 0, 0));
    GPU_API_Write_DLCmd(VERTEX2II(hoffset,voffset,0,0));
    hoffset = 256;
    GPU_API_Write_DLCmd(VERTEX2II(hoffset,voffset,0,1));

    GPU_API_Write_DLCmd(DISPLAY() );

    /* Reset the DL Buffer index, set for the next group of DL commands */
    GPU_API_Reset_DLBuffer();

    /* Do a swap */
    GPU_API_GPU_DLSwap(DLSWAP_FRAME);
    GPU_APP_ENABLE_DELAY();

}
#endif

ft_void_t    GPU_APP_LineStrips()
{

    GPU_API_Write_DLCmd(CLEAR_COLOR_RGB(5, 45, 10) );
    GPU_API_Write_DLCmd(COLOR_RGB(255, 168, 64) );
    GPU_API_Write_DLCmd(CLEAR(1 ,1 ,1) );
    GPU_API_Write_DLCmd(BEGIN(LINE_STRIP) );
    GPU_API_Write_DLCmd(VERTEX2F(16 * 16,16 * 16) );
    GPU_API_Write_DLCmd(VERTEX2F((DispWidth*2/3) * 16,(DispHeight*2/3) * 16) );
    GPU_API_Write_DLCmd(VERTEX2F((DispWidth - 80) * 16,(DispHeight - 20) * 16) );
    GPU_API_Write_DLCmd(DISPLAY() );

    /* Reset the DL Buffer index, set for the next group of DL commands */
    GPU_API_Reset_DLBuffer();

    /* Do a swap */
    GPU_API_GPU_DLSwap(DLSWAP_FRAME);
    GPU_APP_ENABLE_DELAY();

}

ft_void_t    GPU_APP_EdgeStrips()
{

    GPU_API_Write_DLCmd(CLEAR_COLOR_RGB(5, 45, 10) );
    GPU_API_Write_DLCmd(COLOR_RGB(255, 168, 64) );
    GPU_API_Write_DLCmd(CLEAR(1 ,1 ,1) );
    GPU_API_Write_DLCmd(BEGIN(EDGE_STRIP_R) );
    GPU_API_Write_DLCmd(VERTEX2F(16 * 16,16 * 16) );
    GPU_API_Write_DLCmd(VERTEX2F((DispWidth*2/3) * 16,(DispHeight*2/3) * 16) );
    GPU_API_Write_DLCmd(VERTEX2F((DispWidth - 80) * 16,(DispHeight - 20) * 16) );
    GPU_API_Write_DLCmd(DISPLAY() );

    /* Reset the DL Buffer index, set for the next group of DL commands */
    GPU_API_Reset_DLBuffer();

    /* Do a swap */
    GPU_API_GPU_DLSwap(DLSWAP_FRAME);
    GPU_APP_ENABLE_DELAY();

}
ft_void_t    GPU_APP_Scissor()
{

    GPU_API_Write_DLCmd(CLEAR(1,1,1)); // Clear to black
    GPU_API_Write_DLCmd(SCISSOR_XY(40, 20)); // Scissor rectangle top left at (40, 20)
    GPU_API_Write_DLCmd(SCISSOR_SIZE(40, 40)); // Scissor rectangle is 40 x 40 pixels
    GPU_API_Write_DLCmd(CLEAR_COLOR_RGB(255, 255, 0)); // Clear to yellow
    GPU_API_Write_DLCmd(CLEAR(1, 1, 1));
    GPU_API_Write_DLCmd(DISPLAY());

    /* Reset the DL Buffer index, set for the next group of DL commands */
    GPU_API_Reset_DLBuffer();

    /* Do a swap */
    GPU_API_GPU_DLSwap(DLSWAP_FRAME);
    GPU_APP_ENABLE_DELAY();
}


ft_void_t GPU_APP_Polygon()
{

    GPU_API_Write_DLCmd(CLEAR(1, 1, 1)); // clear screen
    GPU_API_Write_DLCmd(COLOR_RGB(255, 0, 0) );
    GPU_API_Write_DLCmd(STENCIL_OP(INCR,INCR) );
    GPU_API_Write_DLCmd(COLOR_MASK(0,0,0,0) );//mask all the colors
    GPU_API_Write_DLCmd(BEGIN(EDGE_STRIP_L));
    GPU_API_Write_DLCmd(VERTEX2II((DispWidth/2),(DispHeight/4),0,0));
    GPU_API_Write_DLCmd(VERTEX2II((DispWidth*4/5),(DispHeight*4/5),0,0));
    GPU_API_Write_DLCmd(VERTEX2II((DispWidth/4),(DispHeight/2),0,0));
    GPU_API_Write_DLCmd(VERTEX2II((DispWidth/2),(DispHeight/4),0,0));
    GPU_API_Write_DLCmd(END());
    GPU_API_Write_DLCmd(COLOR_MASK(1,1,1,1) );//enable all the colors
    GPU_API_Write_DLCmd(STENCIL_FUNC(EQUAL,1,255));
    GPU_API_Write_DLCmd(BEGIN(EDGE_STRIP_L));
    GPU_API_Write_DLCmd(VERTEX2II(DispWidth,0,0,0));
    GPU_API_Write_DLCmd(VERTEX2II(DispWidth,DispHeight,0,0));
    GPU_API_Write_DLCmd(END());

    /* Draw lines at the borders to make sure anti aliazing is also done */
    GPU_API_Write_DLCmd(STENCIL_FUNC(ALWAYS,0,255));
    GPU_API_Write_DLCmd(LINE_WIDTH(1*16));
    GPU_API_Write_DLCmd(COLOR_RGB(0, 0, 0) );
    GPU_API_Write_DLCmd(BEGIN(LINES));
    GPU_API_Write_DLCmd(VERTEX2II((DispWidth/2),(DispHeight/4),0,0));
    GPU_API_Write_DLCmd(VERTEX2II((DispWidth*4/5),(DispHeight*4/5),0,0));
    GPU_API_Write_DLCmd(VERTEX2II((DispWidth*4/5),(DispHeight*4/5),0,0));
    GPU_API_Write_DLCmd(VERTEX2II((DispWidth/4),(DispHeight/2),0,0));
    GPU_API_Write_DLCmd(VERTEX2II((DispWidth/4),(DispHeight/2),0,0));
    GPU_API_Write_DLCmd(VERTEX2II((DispWidth/2),(DispHeight/4),0,0));
    GPU_API_Write_DLCmd(END());
    GPU_API_Write_DLCmd(DISPLAY() );

    /* Reset the DL Buffer index, set for the next group of DL commands */
    GPU_API_Reset_DLBuffer();

    /* Do a swap */
    GPU_API_GPU_DLSwap(DLSWAP_FRAME);
    GPU_APP_ENABLE_DELAY();
}

ft_void_t GPU_APP_Cube()
{
    ft_uint32_t points[6*5],x,y,i,z;
    ft_int16_t xoffset,yoffset,CubeEdgeSz;

// colour vertices
    ft_uint16_t colors[6][3] = {{255,0,0},
                                {255,0,150},
                                {0,255,0},
                                {110,120,110},
                                {0,0,255},
                                {128,128,0}
                                };

// Cube dimension is of 100*100*100
    CubeEdgeSz = 100;
    xoffset = (DispWidth/2 - CubeEdgeSz);yoffset=((DispHeight - CubeEdgeSz)/2);


// xy plane(front)
       points[0] = VERTEX2F(xoffset*16,yoffset*16);
       points[1] = VERTEX2F((xoffset+CubeEdgeSz)*16,yoffset*16);
       points[2] = VERTEX2F((xoffset+CubeEdgeSz)*16,(yoffset+CubeEdgeSz)*16);
       points[3] = VERTEX2F(xoffset*16,(yoffset+CubeEdgeSz)*16);
       points[4] = points[0];

//yz plane (left)
       x = (xoffset+(CubeEdgeSz/2));            //     xoff+w/2
       y = (yoffset-(CubeEdgeSz/2));            //     yoff-h/2

       points[5] = points[0];
       points[6] = VERTEX2F(x*16,y*16);
       points[7] = VERTEX2F(x*16,(y+CubeEdgeSz)*16);
       points[8] = points[3];
       points[9] = points[5];

//xz plane(top)
       points[10] = points[0];
       points[11] = points[1];
       points[12] = VERTEX2F((x+CubeEdgeSz)*16,(y)*16);
       points[13] = points[6];
       points[14] = points[10];

//xz plane(bottom)
       points[15] = points[3];
       points[16] = points[2];
       points[17] = VERTEX2F((x+CubeEdgeSz)*16,(y+CubeEdgeSz)*16);
       points[18] = points[7];
       points[19] = points[15];

//yz plane (right)
       points[20] = points[2];
       points[21] = points[17];
       points[22] = points[12];
       points[23] = points[1];
       points[24] = points[20];

//yz plane (back)
       points[25] = points[6];
       points[26] = points[7];
       points[27] = points[17];
       points[28] = points[12];
       points[29] = points[25];


       GPU_API_Write_DLCmd(CLEAR(1,1,1));
       GPU_API_Write_DLCmd(LINE_WIDTH(16));
       GPU_API_Write_DLCmd(CLEAR_COLOR_RGB(255,255,255) );
       GPU_API_Write_DLCmd(CLEAR(1,1,1) );
       GPU_API_Write_DLCmd(COLOR_RGB(255,255,255));

// Draw a cube
       GPU_API_Write_DLCmd(STENCIL_OP(INCR,INCR));
       GPU_API_Write_DLCmd(COLOR_A(192));
       for(z=0;z<6;z++)
       {
            GPU_API_Write_DLCmd(CLEAR(0,1,1) );//clear stencil buffer
            GPU_API_Write_DLCmd(COLOR_MASK(0,0,0,0));//mask all the colors and draw one surface
            GPU_API_Write_DLCmd(STENCIL_FUNC(ALWAYS,0,255));//stencil function to increment all the values
            GPU_API_Write_DLCmd(BEGIN(EDGE_STRIP_L));
            for(i = 0;i<5;i++)
            {
                GPU_API_Write_DLCmd(points[z*5 + i]);
            }
            GPU_API_Write_DLCmd(END());
            /* set the color and draw a strip */
            GPU_API_Write_DLCmd(COLOR_MASK(1,1,1,1));
            GPU_API_Write_DLCmd(STENCIL_FUNC(EQUAL,1,255));
            //GPU_API_Write_DLCmd(STENCIL_FUNC(EQUAL,(z+1),255));
            GPU_API_Write_DLCmd(COLOR_RGB(colors[z][0],colors[z][1],colors[z][2]));
            GPU_API_Write_DLCmd(BEGIN(RECTS));
            GPU_API_Write_DLCmd(VERTEX2II(xoffset,0,0,0));
            GPU_API_Write_DLCmd(VERTEX2II(xoffset + CubeEdgeSz*2,yoffset + CubeEdgeSz*2,0,0));
            GPU_API_Write_DLCmd(END());
       }
    GPU_API_Write_DLCmd(DISPLAY());

    /* Reset the DL Buffer index, set for the next group of DL commands */
    GPU_API_Reset_DLBuffer();

    /* Do a swap */
    GPU_API_GPU_DLSwap(DLSWAP_FRAME);
    GPU_APP_ENABLE_DELAY();
}


ft_void_t    GPU_APP_Stencil()
{
    ft_int16_t PointSize = 50,DispBtwPoints = 60;

    GPU_API_Write_DLCmd(CLEAR(1,1,1)); // Clear to black
    GPU_API_Write_DLCmd(SCISSOR_XY(40, 20)); // Scissor rectangle top left at (40, 20)
    GPU_API_Write_DLCmd(STENCIL_OP(INCR, INCR) );
    GPU_API_Write_DLCmd(POINT_SIZE(PointSize * 16) );
    GPU_API_Write_DLCmd(BEGIN(POINTS) );
    GPU_API_Write_DLCmd(VERTEX2II(((DispWidth - DispBtwPoints)/2), (DispHeight/2), 0, 0) );
    GPU_API_Write_DLCmd(VERTEX2II(((DispWidth + DispBtwPoints)/2), (DispHeight/2), 0, 0) );
    GPU_API_Write_DLCmd(STENCIL_FUNC(EQUAL, 2, 255) );
    GPU_API_Write_DLCmd(COLOR_RGB(128, 0, 0) );
    GPU_API_Write_DLCmd(VERTEX2II((DispWidth/2), (DispHeight/2), 0, 0) );
    GPU_API_Write_DLCmd(DISPLAY());

    /* Reset the DL Buffer index, set for the next group of DL commands */
    GPU_API_Reset_DLBuffer();

    /* Do a swap */
    GPU_API_GPU_DLSwap(DLSWAP_FRAME);
    GPU_APP_ENABLE_DELAY();
}
/*****************************************************************************/
/* Example code to demonstrated display of point and text                    */
/*****************************************************************************/
ft_void_t    GPU_APP_FTDIString()
{
    ft_int16_t hoffset,voffset,PointSz;

    voffset = ((DispHeight - 49)/2);//49 is the max height of inbuilt font handle 31
    hoffset = ((DispWidth - 4*36)/2);
    PointSz = 20;
    hoffset += PointSz;
    GPU_HAL_Wr32(pScreen, RAM_DL + 0,  CLEAR(1, 1, 1)); // clear screen
    GPU_HAL_Wr32(pScreen, RAM_DL + 4,  BEGIN(BITMAPS)); // start drawing bitmaps

    GPU_HAL_Wr32(pScreen, RAM_DL + 8,  VERTEX2II(hoffset, voffset, 31, 'F')); // ascii F in font 31
    hoffset += 24;
    GPU_HAL_Wr32(pScreen, RAM_DL + 12, VERTEX2II(hoffset, voffset, 31, 'T')); // ascii T
    hoffset += 26;
    GPU_HAL_Wr32(pScreen, RAM_DL + 16, VERTEX2II(hoffset, voffset, 31, 'D')); // ascii D
    hoffset += 29;
    GPU_HAL_Wr32(pScreen, RAM_DL + 20, VERTEX2II(hoffset, voffset, 31, 'I')); // ascii I
    GPU_HAL_Wr32(pScreen, RAM_DL + 24, END());
    GPU_HAL_Wr32(pScreen, RAM_DL + 28, COLOR_RGB(160, 22, 22)); // change color to red
    GPU_HAL_Wr32(pScreen, RAM_DL + 32, POINT_SIZE(PointSz * 16)); // set point size
    hoffset = ((DispWidth - 4*36)/2);
    GPU_HAL_Wr32(pScreen, RAM_DL + 36, BEGIN(POINTS)); // start drawing points
    GPU_HAL_Wr32(pScreen, RAM_DL + 40, VERTEX2II(hoffset, (DispHeight/2), 0, 0)); // red point
    GPU_HAL_Wr32(pScreen, RAM_DL + 44, END());
    GPU_HAL_Wr32(pScreen, RAM_DL + 48, DISPLAY()); // display the image

    /* Do a swap */
    GPU_API_GPU_DLSwap(DLSWAP_FRAME);
    GPU_APP_ENABLE_DELAY();
}
/* Call and return example - simple graph */
ft_void_t    GPU_APP_StreetMap()
{
    GPU_API_Write_DLCmd(CLEAR_COLOR_RGB(236,232,224)); //light gray
    GPU_API_Write_DLCmd(CLEAR(1,1,1));
    GPU_API_Write_DLCmd(COLOR_RGB(170,157,136));//medium gray
    GPU_API_Write_DLCmd(LINE_WIDTH(63));
    GPU_API_Write_DLCmd(CALL(19));//draw the streets
    GPU_API_Write_DLCmd(COLOR_RGB(250,250,250));//white
    GPU_API_Write_DLCmd(LINE_WIDTH(48));
    GPU_API_Write_DLCmd(CALL(19));//draw the streets
    GPU_API_Write_DLCmd(COLOR_RGB(0,0,0));
    GPU_API_Write_DLCmd(BEGIN(BITMAPS));
    GPU_API_Write_DLCmd(VERTEX2II(240,91,27,77  ));//draw "Main st." at (240,91)
    GPU_API_Write_DLCmd(VERTEX2II(252,91,27,97    ));
    GPU_API_Write_DLCmd(VERTEX2II(260,91,27,105    ));
    GPU_API_Write_DLCmd(VERTEX2II(263,91,27,110    ));
    GPU_API_Write_DLCmd(VERTEX2II(275,91,27,115    ));
    GPU_API_Write_DLCmd(VERTEX2II(282,91,27,116    ));
    GPU_API_Write_DLCmd(VERTEX2II(286,91,27,46    ));
    GPU_API_Write_DLCmd(END());
    GPU_API_Write_DLCmd(DISPLAY());
    GPU_API_Write_DLCmd(BEGIN(LINES));
    GPU_API_Write_DLCmd(VERTEX2F(-160,-20  ));
    GPU_API_Write_DLCmd(VERTEX2F(320,4160  ));
    GPU_API_Write_DLCmd(VERTEX2F(800,-20   ));
    GPU_API_Write_DLCmd(VERTEX2F(1280,4160 ));
    GPU_API_Write_DLCmd(VERTEX2F(1920,-20  ));
    GPU_API_Write_DLCmd(VERTEX2F(2400,4160 ));
    GPU_API_Write_DLCmd(VERTEX2F(2560,-20  ));
    GPU_API_Write_DLCmd(VERTEX2F(3040,4160 ));
    GPU_API_Write_DLCmd(VERTEX2F(3200,-20  ));
    GPU_API_Write_DLCmd(VERTEX2F(3680,4160 ));
    GPU_API_Write_DLCmd(VERTEX2F(2880,-20  ));
    GPU_API_Write_DLCmd(VERTEX2F(3360,4160 ));
    GPU_API_Write_DLCmd(VERTEX2F(-20,0       ));
    GPU_API_Write_DLCmd(VERTEX2F(5440,-480 ));
    GPU_API_Write_DLCmd(VERTEX2F(-20,960   ));
    GPU_API_Write_DLCmd(VERTEX2F(5440,480  ));
    GPU_API_Write_DLCmd(VERTEX2F(-20,1920  ));
    GPU_API_Write_DLCmd(VERTEX2F(5440,1440 ));
    GPU_API_Write_DLCmd(VERTEX2F(-20,2880  ));
    GPU_API_Write_DLCmd(VERTEX2F(5440,2400 ));
    GPU_API_Write_DLCmd(END());
    GPU_API_Write_DLCmd(RETURN());

    /* Reset the DL Buffer index, set for the next group of DL commands */
    GPU_API_Reset_DLBuffer();

    /* Do a swap */
    GPU_API_GPU_DLSwap(DLSWAP_FRAME);
    GPU_APP_ENABLE_DELAY();
}

/* usage of additive blending - draw 3 Gs*/
ft_void_t    GPU_APP_AdditiveBlendText()
{

    GPU_API_Write_DLCmd(CLEAR(1, 1, 1)); // clear screen
    GPU_API_Write_DLCmd(BEGIN(BITMAPS) );
    GPU_API_Write_DLCmd(VERTEX2II(50, 30, 31, 0x47) );
    GPU_API_Write_DLCmd(COLOR_A( 128 ) );
    GPU_API_Write_DLCmd(VERTEX2II(58, 38, 31, 0x47) );
    GPU_API_Write_DLCmd(COLOR_A( 64 ) );
    GPU_API_Write_DLCmd(VERTEX2II(66, 46, 31, 0x47) );
    GPU_API_Write_DLCmd(END() );
    GPU_API_Write_DLCmd(DISPLAY() );

    /* Reset the DL Buffer index, set for the next group of DL commands */
    GPU_API_Reset_DLBuffer();

    /* Do a swap */
    GPU_API_GPU_DLSwap(DLSWAP_FRAME);
    GPU_APP_ENABLE_DELAY();
}

/* Usage of macro */
ft_void_t    GPU_APP_MacroUsage()
{
    ft_int32_t xoffset,yoffset,xflag = 1,yflag = 1,flagloop = 1;
    GPU_API_Bitmap_header_t *p_bmhdr;

    xoffset = DispWidth/3;
    yoffset = DispHeight/2;

    /* First write a valid macro instruction into macro0 */
    GPU_HAL_Wr32(pScreen, REG_MACRO_0,VERTEX2F(xoffset*16,yoffset*16));
    /* update lena face as bitmap 0 */

    p_bmhdr = &Lena_Bitmap_RawData_Header[0];
    /* Copy raw data into address 0 followed by generation of bitmap */
    GPU_HAL_WrMem(pScreen, RAM_G,&Lena_Bitmap_RawData[p_bmhdr->ArrayOffset], p_bmhdr->Stride*p_bmhdr->Height);


    GPU_API_Write_DLCmd(CLEAR(1, 1, 1)); // clear screen
    GPU_API_Write_DLCmd(BITMAP_SOURCE(RAM_G));
    GPU_API_Write_DLCmd(BITMAP_LAYOUT(p_bmhdr->Format, p_bmhdr->Stride, p_bmhdr->Height));
    GPU_API_Write_DLCmd(BITMAP_SIZE(NEAREST, BORDER, BORDER, p_bmhdr->Width, p_bmhdr->Height));
    GPU_API_Write_DLCmd(BEGIN(BITMAPS)); // start drawing bitmaps
    GPU_API_Write_DLCmd(MACRO(0)); // draw the image at (100,120)
    GPU_API_Write_DLCmd(END());
    GPU_API_Write_DLCmd(DISPLAY() );

    /* Reset the DL Buffer index, set for the next group of DL commands */
    GPU_API_Reset_DLBuffer();

    /* Do a swap */
    GPU_API_GPU_DLSwap(DLSWAP_FRAME);
        flagloop = 1000;
    while(flagloop-- > 0)
    {
        if(((xoffset + p_bmhdr->Width) >= DispWidth) || (xoffset <= 0))
        {
            xflag ^= 1;
        }
        if(((yoffset + p_bmhdr->Height) >= DispHeight) || (yoffset <= 0))
        {
            yflag ^= 1;
        }
        if(xflag)
        {
            xoffset++;
        }
        else
        {
            xoffset--;
        }
        if(yflag)
        {
            yoffset++;
        }
        else
        {
            yoffset--;
        }
        /*  update just the macro */
        GPU_HAL_Wr32(pScreen, REG_MACRO_0,VERTEX2F(xoffset*16,yoffset*16));
        GPU_APP_ENABLE_DELAY_MS(10);    //sleep for 10 ms;
    }
}


/* Additive blending of points - 1000 points */
ft_void_t    GPU_APP_AdditiveBlendPoints()
{
    ft_int32_t i,hoffset,voffset,flagloop=1,j,hdiff,vdiff,PointSz;

#define MSVC_PI (3.141592653)
    PointSz = 4;
        flagloop = 10;
    while(flagloop-- > 0)
    {
          /* Reset the DL Buffer index, set for the next group of DL commands */
        GPU_API_Reset_DLBuffer();

        GPU_API_Write_DLCmd(CLEAR(1, 1, 1)); // clear screen
        GPU_API_Write_DLCmd(COLOR_RGB(20, 91,20)); // green color for additive blending
        GPU_API_Write_DLCmd(BLEND_FUNC(SRC_ALPHA,ONE));//input is source alpha and destination is whole color
        GPU_API_Write_DLCmd(POINT_SIZE(PointSz*16));
        GPU_API_Write_DLCmd(BEGIN(POINTS));


        /* First 100 random values */
        for(i=0;i<100;i++)
        {
            hoffset = rand();
            voffset = rand();
            GPU_API_Write_DLCmd(VERTEX2F(hoffset*16,voffset*16));
        }

        /* next 480 are sine values of two cycles */
        for(i=0;i<160;i++)
        {
            /* i is x offset, y is sinwave */
            hoffset = i*3;

            //voffset = 136 + 136*sin((-(360*i/80)*MSVC_PI)/180);
            voffset = (DispHeight/2) + ((ft_int32_t)(DispHeight/2)*sin(-65536*i/(DispWidth/6))/65536);

            GPU_API_Write_DLCmd(VERTEX2F(hoffset*16,voffset*16));
            //randomvertexins[i*5] = VERTEX2F(hoffset*16,voffset*16);
            for(j=0;j<4;j++)
            {
                hdiff = rand();
                vdiff = rand();
                GPU_API_Write_DLCmd(VERTEX2F((hoffset + hdiff)*16,(voffset + vdiff)*16));
            }
        }

        GPU_API_Write_DLCmd(END());
        GPU_API_Write_DLCmd(DISPLAY() );

        /* Reset the DL Buffer index, set for the next group of DL commands */
        GPU_API_Reset_DLBuffer();

        /* Do a swap */
        GPU_API_GPU_DLSwap(DLSWAP_FRAME);
        GPU_APP_ENABLE_DELAY_MS(100);    //sleep for 100 ms

    }
}
#endif


ft_void_t GPU_APP_Screen(ft_const_char8_t *str)
{

    GPU_CoCmd_Dlstart(pScreen);
    GPU_API_Write_CoCmd(CLEAR_COLOR_RGB(255,255,255));
    GPU_API_Write_CoCmd(CLEAR(1,1,1));

    GPU_API_Write_CoCmd(COLOR_RGB(0x80,0x80,0x00));
    GPU_CoCmd_Text(pScreen,(DispWidth/2), DispHeight/2, 31, OPT_CENTERX, str);

    GPU_API_Write_CoCmd(DISPLAY());
    GPU_CoCmd_Swap(pScreen);

    /* Wait till coprocessor completes the operation */
    GPU_HAL_WaitCmdfifo_empty(pScreen);
    GPU_APP_ENABLE_DELAY_MS(2000);
}


ft_void_t GPU_APP_CoPro_Widget_Logo()
{
    GPU_CoCmd_Logo(pScreen);

    GPU_HAL_WaitLogo_Finish(pScreen);
    GPU_APP_ENABLE_DELAY();
}


/* API to demonstrate calibrate widget/functionality */
ft_void_t GPU_APP_CoPro_Widget_Calibrate()
{
    ft_uint32_t TransMatrix[6];

    /*************************************************************************/
    /* Below code demonstrates the usage of calibrate function. Calibrate    */
    /* function will wait until user presses all the three dots. Only way to*/
    /* come out of this api is to reset the coprocessor bit.                 */
    /*************************************************************************/
    {

    GPU_CoCmd_Dlstart(pScreen);

    GPU_API_Write_CoCmd(CLEAR_COLOR_RGB(64,64,64));
    GPU_API_Write_CoCmd(CLEAR(1,1,1));
    GPU_API_Write_CoCmd(COLOR_RGB(0xff,0xff,0xff));
    /* Draw number at 0,0 location */
    //GPU_API_Write_CoCmd(COLOR_A(30));
    GPU_CoCmd_Text(pScreen,(DispWidth/2), (DispHeight/2), 27, OPT_CENTER, "Please Tap on the dot");
    GPU_CoCmd_Calibrate(pScreen,0);

    /* Wait till coprocessor completes the operation */
    GPU_HAL_WaitCmdfifo_empty(pScreen);
    /* Print the configured values */
    GPU_HAL_RdMem(pScreen,REG_TOUCH_TRANSFORM_A,(ft_uint8_t *)TransMatrix,4*6);//read all the 6 coefficients
    }

}

#ifdef GPU_APP_ENABLE_APIS_SET1
/* API to demonstrate text widget */
ft_void_t GPU_APP_CoPro_Widget_Text()
{

    /*************************************************************************/
    /* Below code demonstrates the usage of text function. Text function     */
    /* draws text with either in built or externally configured text. Text   */
    /* color can be changed by colorrgb and text function supports display of*/
    /* texts on left, right, top, bottom and center respectively             */
    /*************************************************************************/
    {

    GPU_CoCmd_Dlstart(pScreen);
    GPU_API_Write_CoCmd(CLEAR_COLOR_RGB(64,64,64));
    GPU_API_Write_CoCmd(CLEAR(1,1,1));
    GPU_API_Write_CoCmd(COLOR_RGB(0xff,0xff,0xff));
    /* Draw text at 0,0 location */
    GPU_API_Write_CoCmd(COLOR_RGB(0x00,0x00,0x80));
    GPU_CoCmd_Text(pScreen,0, 0, 29, 0, "FTDI!");
    GPU_API_Write_CoCmd(COLOR_RGB(0xff,0xff,0xff));
    GPU_CoCmd_Text(pScreen,0, 40, 26, 0, "Text29 at 0,0");//text info
    /* text with centerx */
    GPU_API_Write_CoCmd(COLOR_RGB(0x80,0x00,0x00));
    GPU_CoCmd_Text(pScreen,(DispWidth/2), 50, 29, OPT_CENTERX, "FTDI!");
    GPU_API_Write_CoCmd(COLOR_RGB(0xff,0xff,0xff));
    GPU_CoCmd_Text(pScreen,((DispWidth/2) - 30), 90, 26, 0, "Text29 centerX");//text info
    /* text with centery */
    GPU_API_Write_CoCmd(COLOR_RGB(0x41,0x01,0x05));
    GPU_CoCmd_Text(pScreen,(DispWidth/5), 120, 29, OPT_CENTERY, "FTDI!");
    GPU_API_Write_CoCmd(COLOR_RGB(0xff,0xff,0xff));
    GPU_CoCmd_Text(pScreen,(DispWidth/5), 140, 26, 0, "Text29 centerY");//text info
    /* text with center */
    GPU_API_Write_CoCmd(COLOR_RGB(0x0b,0x07,0x21));
    GPU_CoCmd_Text(pScreen,(DispWidth/2), 180, 29, OPT_CENTER, "FTDI!");
    GPU_API_Write_CoCmd(COLOR_RGB(0xff,0xff,0xff));
    GPU_CoCmd_Text(pScreen,((DispWidth/2) - 50), 200, 26, 0, "Text29 center");//text info
    /* text with rightx */
    GPU_API_Write_CoCmd(COLOR_RGB(0x57,0x5e,0x1b));
    GPU_CoCmd_Text(pScreen,DispWidth, 60, 29, OPT_RIGHTX, "FTDI!");
    GPU_API_Write_CoCmd(COLOR_RGB(0xff,0xff,0xff));
    GPU_CoCmd_Text(pScreen,(DispWidth - 90), 100, 26, 0, "Text29 rightX");//text info
    GPU_API_Write_CoCmd(DISPLAY());
    GPU_CoCmd_Swap(pScreen);

    /* Wait till coprocessor completes the operation */
    GPU_HAL_WaitCmdfifo_empty(pScreen);
    GPU_APP_ENABLE_DELAY();
    }
}

/* API to demonstrate number widget */
ft_void_t GPU_APP_CoPro_Widget_Number()
{
    /*************************************************************************/
    /* Below code demonstrates the usage of number function. Number function */
    /* draws text with only 32bit decimal number, signed or unsigned can also*/
    /* be specified as input parameter. Options like centerx, centery, center*/
    /* and rightx are supported                                              */
    /*************************************************************************/
    {


    GPU_CoCmd_Dlstart(pScreen);
    GPU_API_Write_CoCmd(CLEAR_COLOR_RGB(64,64,64));
    GPU_API_Write_CoCmd(CLEAR(1,1,1));
    GPU_API_Write_CoCmd(COLOR_RGB(0xff,0xff,0xff));
    /* Draw number at 0,0 location */
    GPU_API_Write_CoCmd(COLOR_RGB(0x00,0x00,0x80));
    GPU_CoCmd_Number(pScreen,0, 0, 29, 0, 1234);
    GPU_API_Write_CoCmd(COLOR_RGB(0xff,0xff,0xff));
    GPU_CoCmd_Text(pScreen,0, 40, 26, 0, "Number29 at 0,0");//text info
    /* number with centerx */
    GPU_API_Write_CoCmd(COLOR_RGB(0x80,0x00,0x00));
    GPU_CoCmd_Number(pScreen,(DispWidth/2), 50, 29, OPT_CENTERX | OPT_SIGNED, -1234);
    GPU_API_Write_CoCmd(COLOR_RGB(0xff,0xff,0xff));
    GPU_CoCmd_Text(pScreen,((DispWidth/2) - 30), 90, 26, 0, "Number29 centerX");//text info
    /* number with centery */
    GPU_API_Write_CoCmd(COLOR_RGB(0x41,0x01,0x05));
    GPU_CoCmd_Number(pScreen,(DispWidth/5), 120, 29, OPT_CENTERY, 1234);
    GPU_API_Write_CoCmd(COLOR_RGB(0xff,0xff,0xff));
    GPU_CoCmd_Text(pScreen,(DispWidth/5), 140, 26, 0, "Number29 centerY");//text info
    /* number with center */
    GPU_API_Write_CoCmd(COLOR_RGB(0x0b,0x07,0x21));
    GPU_CoCmd_Number(pScreen,(DispWidth/2), 180, 29, OPT_CENTER | OPT_SIGNED, -1234);
    GPU_API_Write_CoCmd(COLOR_RGB(0xff,0xff,0xff));
    GPU_CoCmd_Text(pScreen,((DispWidth/2) - 50), 200, 26, 0, "Number29 center");//text info
    /* number with rightx */
    GPU_API_Write_CoCmd(COLOR_RGB(0x57,0x5e,0x1b));
    GPU_CoCmd_Number(pScreen,DispWidth, 60, 29, OPT_RIGHTX, 1234);
    GPU_API_Write_CoCmd(COLOR_RGB(0xff,0xff,0xff));
    GPU_CoCmd_Text(pScreen,(DispWidth - 100), 100, 26, 0, "Number29 rightX");//text info

    GPU_API_Write_CoCmd(DISPLAY());
    GPU_CoCmd_Swap(pScreen);

    /* Wait till coprocessor completes the operation */
    GPU_HAL_WaitCmdfifo_empty(pScreen);
    GPU_APP_ENABLE_DELAY();
    }
}
/* Main entry point */
/* API to demonstrate button functionality */
ft_void_t GPU_APP_CoPro_Widget_Button()
{

    /*************************************************************************/
    /* Below code demonstrates the usage of button function. Buttons can be  */
    /* constructed using flat or 3d effect. Button color can be changed by   */
    /* fgcolor command and text color is set by COLOR_RGB graphics command   */
    /*************************************************************************/
    {
    ft_int16_t xOffset,yOffset,bWidth,bHeight,bDistx,bDisty;

    bWidth = 60;
    bHeight = 30;
    bDistx = bWidth + ((DispWidth - 4 * bWidth)/5);
    bDisty = bHeight + 5;
    xOffset = 10;
    yOffset = (DispHeight/2 - 2*bDisty);
    /************ Construct a buttons with "ONE/TWO/THREE" text and default background *********************/
    /* Draw buttons 60x30 resolution at 10x40,10x75,10x110 */

    GPU_CoCmd_Dlstart(pScreen);
    GPU_API_Write_CoCmd(CLEAR_COLOR_RGB(64,64,64));
    GPU_API_Write_CoCmd(CLEAR(1,1,1));
    GPU_API_Write_CoCmd(COLOR_RGB(255,255,255));
    /* flat effect and default color background */
    GPU_CoCmd_FgColor(pScreen,0x0000ff);
    yOffset = (DispHeight/2 - 2*bDisty);
    GPU_CoCmd_Button(pScreen,xOffset,yOffset,bWidth,bHeight,28,OPT_FLAT,"ABC");
    yOffset += bDisty;
    GPU_CoCmd_Button(pScreen,xOffset,yOffset,bWidth,bHeight,28,OPT_FLAT,"ABC");
    yOffset += bDisty;
    GPU_CoCmd_Button(pScreen,xOffset,yOffset,bWidth,bHeight,28,OPT_FLAT,"ABC");
    GPU_CoCmd_Text(pScreen,xOffset, (yOffset + 40), 26, 0, "Flat effect");//text info
    /* 3D effect */
    xOffset += bDistx;
    yOffset = (DispHeight/2 - 2*bDisty);
    GPU_CoCmd_Button(pScreen,xOffset,yOffset,bWidth,bHeight,28,0,"ABC");
    yOffset += bDisty;
    GPU_CoCmd_Button(pScreen,xOffset,yOffset,bWidth,bHeight,28,0,"ABC");
    yOffset += bDisty;
    GPU_CoCmd_Button(pScreen,xOffset,yOffset,bWidth,bHeight,28,0,"ABC");
    GPU_CoCmd_Text(pScreen,xOffset, (yOffset + 40), 26, 0, "3D Effect");//text info
    /* 3d effect with background color */
    GPU_CoCmd_FgColor(pScreen,0xffff00);
    xOffset += bDistx;
    yOffset = (DispHeight/2 - 2*bDisty);
    GPU_CoCmd_Button(pScreen,xOffset,yOffset,bWidth,bHeight,28,0,"ABC");
    yOffset += bDisty;
    GPU_CoCmd_FgColor(pScreen,0x00ffff);
    GPU_CoCmd_Button(pScreen,xOffset,yOffset,bWidth,bHeight,28,0,"ABC");
    yOffset += bDisty;
    GPU_CoCmd_FgColor(pScreen,0xff00ff);
    GPU_CoCmd_Button(pScreen,xOffset,yOffset,bWidth,bHeight,28,0,"ABC");
    GPU_CoCmd_Text(pScreen,xOffset, (yOffset + 40), 26, 0, "3D Colour");//text info
    /* 3d effect with gradient color */
    GPU_CoCmd_FgColor(pScreen,0x101010);
    GPU_CoCmd_GradColor(pScreen,0xff0000);
    xOffset += bDistx;
    yOffset = (DispHeight/2 - 2*bDisty);
    GPU_CoCmd_Button(pScreen,xOffset,yOffset,bWidth,bHeight,28,0,"ABC");
    yOffset += bDisty;
    GPU_CoCmd_GradColor(pScreen,0x00ff00);
    GPU_CoCmd_Button(pScreen,xOffset,yOffset,bWidth,bHeight,28,0,"ABC");
    yOffset += bDisty;
    GPU_CoCmd_GradColor(pScreen,0x0000ff);
    GPU_CoCmd_Button(pScreen,xOffset,yOffset,bWidth,bHeight,28,0,"ABC");
    GPU_CoCmd_Text(pScreen,xOffset, (yOffset + 40), 26, 0, "3D Gradient");//text info
    GPU_API_Write_CoCmd(DISPLAY());
    GPU_CoCmd_Swap(pScreen);

    /* Wait till coprocessor completes the operation */
    GPU_HAL_WaitCmdfifo_empty(pScreen);
    GPU_APP_ENABLE_DELAY();
    }
}
#endif

#ifdef GPU_APP_ENABLE_APIS_SET4
/* API to demonstrate the use of transfer commands */
ft_void_t GPU_APP_CoPro_AppendCmds()
{
    ft_uint32_t AppendCmds[16];
    ft_int16_t xoffset,yoffset;
    /*************************************************************************/
    /* Below code demonstrates the usage of coprocessor append command - to append any*/
    /* mcu specific graphics commands to coprocessor generated graphics commands      */
    /*************************************************************************/

    /* Bitmap construction by MCU - display lena at 200x60 offset */
    /* Construct the bitmap data to be copied in the temp buffer then by using
       coprocessor append command copy it into graphics processor DL memory */
    xoffset = ((DispWidth - Lena_Bitmap_RawData_Header[0].Width)/2);
    yoffset = ((DispHeight/3) - Lena_Bitmap_RawData_Header[0].Height/2);

    GPU_API_Write_CoCmd( CMD_DLSTART);
    AppendCmds[0] = CLEAR_COLOR_RGB(255,0,0);
    AppendCmds[1] = CLEAR(1,1,1);
    AppendCmds[2] = COLOR_RGB(255,255,255);
    AppendCmds[3] = BEGIN(BITMAPS);
    AppendCmds[4] = BITMAP_SOURCE(0);
    AppendCmds[5] = BITMAP_LAYOUT(Lena_Bitmap_RawData_Header[0].Format,
        Lena_Bitmap_RawData_Header[0].Stride,Lena_Bitmap_RawData_Header[0].Height);
    AppendCmds[6] = BITMAP_SIZE(BILINEAR,BORDER,BORDER,
        Lena_Bitmap_RawData_Header[0].Width,Lena_Bitmap_RawData_Header[0].Height);
    AppendCmds[7] = VERTEX2F(xoffset * 16,yoffset * 16);
    AppendCmds[8] = END();

        /* Download the bitmap data*/
    GPU_HAL_WrMem(pScreen, RAM_G,(ft_uint8_t *)&Lena_Bitmap_RawData[Lena_Bitmap_RawData_Header[0].ArrayOffset],
        Lena_Bitmap_RawData_Header[0].Stride*Lena_Bitmap_RawData_Header[0].Height);

    /* Download the DL data constructed by the MCU to location 40*40*2 in sram */
    GPU_HAL_WrMem(pScreen,RAM_G + Lena_Bitmap_RawData_Header[0].Stride*Lena_Bitmap_RawData_Header[0].Height,(ft_uint8_t *)AppendCmds,9*4);

    /* Call the append api for copying the above generated data into graphics processor
       DL commands are stored at location 40*40*2 offset from the starting of the sram */
    GPU_CoCmd_Append(pScreen,RAM_G + Lena_Bitmap_RawData_Header[0].Stride*Lena_Bitmap_RawData_Header[0].Height, 9*4);
    /*  Display the text information */
    GPU_CoCmd_FgColor(pScreen,0xffff00);
    xoffset -=50;
    yoffset += 40;
    GPU_CoCmd_Text(pScreen,xoffset, yoffset, 26, 0, "Display bitmap by Append");
    GPU_API_Write_CoCmd(DISPLAY());
    GPU_CoCmd_Swap(pScreen);
    GPU_APP_ENABLE_DELAY();
}
#endif

#ifdef GPU_APP_ENABLE_APIS_SET2
/* API to demonstrate the usage of inflate command - compression done via zlib */
ft_void_t GPU_APP_CoPro_Inflate()
{
    const GPU_API_Bitmap_header_t *pBitmapHdr = (void *)0;
    ft_int16_t xoffset,yoffset;
    /***************************************************************************/
    /* Below code demonstrates the usage of inflate function                   */
    /* Download the deflated data into command buffer and in turn co-processor */
    /* inflates the deflated data and outputs at 0 location                    */
    /***************************************************************************/
    pBitmapHdr = &Lena_Bitmap_RawData_Header[0];

    xoffset = ((DispWidth - Lena_Bitmap_RawData_Header[0].Width)/2);
    yoffset = ((DispHeight - Lena_Bitmap_RawData_Header[0].Height)/2);

    /* Clear the memory at location 0 - any previous bitmap data */

    GPU_API_Write_CoCmd( CMD_MEMSET);
    GPU_API_Write_CoCmd( RAM_G);//starting address of memset
    GPU_API_Write_CoCmd( 255L);//value of memset
    GPU_API_Write_CoCmd( 1L*pBitmapHdr->Stride*pBitmapHdr->Height);//number of elements to be changed

    /* Set the display list for graphics processor */
    /* Bitmap construction by MCU - display lena at 200x90 offset */
    /* Transfer the data into co-processor memory directly word by word */
    GPU_API_Write_CoCmd( CMD_DLSTART);
    GPU_API_Write_CoCmd( CLEAR_COLOR_RGB(0,0,255));
    GPU_API_Write_CoCmd( CLEAR(1,1,1));
    GPU_API_Write_CoCmd( COLOR_RGB(255,255,255));
    GPU_API_Write_CoCmd( BEGIN(BITMAPS));
    GPU_API_Write_CoCmd( BITMAP_SOURCE(0));
    GPU_API_Write_CoCmd( BITMAP_LAYOUT(Lena_Bitmap_RawData_Header[0].Format,
        Lena_Bitmap_RawData_Header[0].Stride,Lena_Bitmap_RawData_Header[0].Height));
    GPU_API_Write_CoCmd( BITMAP_SIZE(BILINEAR,BORDER,BORDER,
        Lena_Bitmap_RawData_Header[0].Width,Lena_Bitmap_RawData_Header[0].Height));
    GPU_API_Write_CoCmd( VERTEX2F(xoffset*16,yoffset*16));
    GPU_API_Write_CoCmd( END());

    /*  Display the text information */
    GPU_API_Write_CoCmd(COLOR_A(255));
    xoffset -= 50;
    yoffset += 40;
    GPU_CoCmd_Text(pScreen,xoffset, yoffset, 26, 0, "Display bitmap by inflate");
    GPU_API_Write_CoCmd(DISPLAY());

    GPU_CoCmd_Swap(pScreen);
    /* Wait till co-processor completes the operation */
    GPU_HAL_WaitCmdfifo_empty(pScreen);

    /* Inflate the data into location 0 */
    GPU_HAL_WrCmd32(pScreen,  CMD_INFLATE);
    GPU_HAL_WrCmd32(pScreen,  RAM_G);//destination address if inflated

    /* Copy the deflated/jpeg encoded data into co-processor fifo */
    GPU_HAL_WrCmdBuf(pScreen, Lenaface40, 1L*GPU_APP_Lenaface40_SIZE);

    GPU_APP_ENABLE_DELAY_MS(2000);
}


/* API to demonstrate jpeg decode functionality */
ft_void_t GPU_APP_CoPro_Loadimage()
{
    ft_int16_t ImgW, ImgH, xoffset,yoffset;
    /*************************************************************************/
    /* Below code demonstrates the usage of loadimage function               */
    /* Download the JPEG data into command buffer and in turn co-processor   */
    /* decodes and dumps into location 0 with rgb565 format                  */
    /*************************************************************************/
    ImgW = 256;
    ImgH = 256;

    xoffset = ((DispWidth - ImgW)/2);
    yoffset = ((DispHeight - ImgH)/2);
    /* Clear the memory at location 0 - any previous bitmap data */

    GPU_API_Write_CoCmd( CMD_MEMSET);
    GPU_API_Write_CoCmd( RAM_G);//starting address of memset
    GPU_API_Write_CoCmd( 255L);//value of memset
    GPU_API_Write_CoCmd( 256L*2*256);//number of elements to be changed

    /* Set the display list for graphics processor */

    /* Bitmap construction by MCU - display Mandrill at 112x8 offset */
    /* Transfer the data into co-processor memory directly word by word */
    GPU_API_Write_CoCmd( CMD_DLSTART);
    GPU_API_Write_CoCmd( CLEAR_COLOR_RGB(0,255,255));
    GPU_API_Write_CoCmd( CLEAR(1,1,1));
    GPU_API_Write_CoCmd( COLOR_RGB(255,255,255));
    GPU_API_Write_CoCmd( BEGIN(BITMAPS));
    GPU_API_Write_CoCmd( BITMAP_SOURCE(0));
    GPU_API_Write_CoCmd( BITMAP_LAYOUT(RGB565, ImgW*2, ImgH));
    GPU_API_Write_CoCmd( BITMAP_SIZE(BILINEAR, BORDER, BORDER, ImgW, ImgH));
    GPU_API_Write_CoCmd( VERTEX2F(xoffset*16,yoffset*16));
    GPU_API_Write_CoCmd( END());

    /*  Display the text information */
    xoffset = ((DispWidth)/2);
    yoffset = ((DispHeight)/2);
    GPU_CoCmd_Text(pScreen,xoffset, yoffset, 26, OPT_CENTER, "Display bitmap by JPEG decode RGB565");
    GPU_API_Write_CoCmd(DISPLAY());
    GPU_CoCmd_Swap(pScreen);

    /* Wait till co-processor completes the operation */
    GPU_HAL_WaitCmdfifo_empty(pScreen);

    /******************* Decode JPEG output into location 0 and output colour format as RGB565 *********************/
    GPU_HAL_WrCmd32(pScreen,  CMD_LOADIMAGE);
    GPU_HAL_WrCmd32(pScreen,  RAM_G);//destination address of jpg decode
    GPU_HAL_WrCmd32(pScreen,  OPT_RGB565);//output format of the bitmap

    /* Copy the deflated/jpeg encoded data into coprocessor fifo */
    GPU_HAL_WrCmdBuf(pScreen, Mandrill256, GPU_APP_Mandrill256_SIZE);

    GPU_APP_ENABLE_DELAY_MS(2000);

    /******************** Decode JPEG output into location 0 & output as MONOCHROME ******************************/
    /* Clear the memory at location 0 - any previous bitmap data */
    xoffset = ((DispWidth - ImgW)/2);
    yoffset = ((DispHeight - ImgH)/2);


    GPU_API_Write_CoCmd( CMD_MEMSET);
    GPU_API_Write_CoCmd( RAM_G);//starting address of memset
    GPU_API_Write_CoCmd( 255L);//value of memset
    GPU_API_Write_CoCmd( 256L*2*256);//number of elements to be changed

    /* Set the display list for graphics processor */
    /* Bitmap construction by MCU - display Mandrill at 112x8 offset */
    /* Transfer the data into co-processor memory directly word by word */
    GPU_API_Write_CoCmd( CMD_DLSTART);
    GPU_API_Write_CoCmd( CLEAR_COLOR_RGB(0,0,0));
    GPU_API_Write_CoCmd( CLEAR(1,1,1));
    GPU_API_Write_CoCmd( COLOR_RGB(255,255,255));
    GPU_API_Write_CoCmd( BEGIN(BITMAPS));
    GPU_API_Write_CoCmd( BITMAP_SOURCE(0));
    GPU_API_Write_CoCmd( BITMAP_LAYOUT(L8,ImgW,ImgH));//monochrome
    GPU_API_Write_CoCmd( BITMAP_SIZE(BILINEAR,BORDER,BORDER,ImgW,ImgH));
    GPU_API_Write_CoCmd( VERTEX2F(xoffset*16,yoffset*16));
    GPU_API_Write_CoCmd( END());

    /*  Display the text information */
    xoffset = ((DispWidth)/2);
    yoffset = ((DispHeight)/2);
    GPU_CoCmd_Text(pScreen,xoffset, yoffset, 26, OPT_CENTER, "Display bitmap by JPEG decode L8");
    GPU_API_Write_CoCmd(DISPLAY());
    GPU_CoCmd_Swap(pScreen);

    /* Wait till coprocessor completes the operation */
    GPU_HAL_WaitCmdfifo_empty(pScreen);
    GPU_APP_ENABLE_DELAY();

    GPU_HAL_WrCmd32(pScreen,  CMD_LOADIMAGE);
    GPU_HAL_WrCmd32(pScreen,  RAM_G);//destination address of jpg decode
    GPU_HAL_WrCmd32(pScreen,  OPT_MONO);//output format of the bitmap - default is rgb565

    /* Copy the deflated/jpeg encoded data into co-processor fifo */
    GPU_HAL_WrCmdBuf(pScreen,Mandrill256,GPU_APP_Mandrill256_SIZE);

    GPU_APP_ENABLE_DELAY_MS(2000);
}
#endif

#ifdef GPU_APP_ENABLE_APIS_SET1
/* API to demonstrate clock widget */
ft_void_t GPU_APP_CoPro_Widget_Clock()
{

    /*************************************************************************/
    /* Below code demonstrates the usage of clock function. Clocks can be    */
    /* constructed using flat or 3d effect. Clock background and foreground  */
    /* colors can be set by gbcolor and colorrgb. Clock can be constructed   */
    /* with multiple options such as no background, no needles, no pointer.  */
    /*************************************************************************/
    {
    ft_int16_t xOffset,yOffset,cRadius,xDistBtwClocks;

    xDistBtwClocks = DispWidth/5;
    cRadius = xDistBtwClocks/2 - DispWidth/64;

    /* Download the bitmap data for lena faced clock */
    GPU_HAL_WrMem(pScreen, RAM_G,(ft_uint8_t *)&Lena_Bitmap_RawData[Lena_Bitmap_RawData_Header[0].ArrayOffset],
        Lena_Bitmap_RawData_Header[0].Stride*Lena_Bitmap_RawData_Header[0].Height);

    /* Draw clock with blue as background and read as needle color */

    GPU_CoCmd_Dlstart(pScreen);
    GPU_API_Write_CoCmd(CLEAR_COLOR_RGB(64,64,64));
    GPU_API_Write_CoCmd(CLEAR(1,1,1));
    GPU_API_Write_CoCmd(COLOR_RGB(255,255,255));
    /* flat effect and default color background */
    xOffset = xDistBtwClocks/2;
    yOffset = cRadius + 5;
    GPU_CoCmd_BgColor(pScreen, 0x0000ff);
    GPU_API_Write_CoCmd(COLOR_RGB(0xff,0x00,0x00));
    GPU_CoCmd_Clock(pScreen, xOffset,yOffset,cRadius,OPT_FLAT,30,100,5,10);
    GPU_CoCmd_Text(pScreen,xOffset, (yOffset + cRadius + 6), 26, OPT_CENTER, "Flat effect");//text info
    /* no seconds needle */
    GPU_CoCmd_BgColor(pScreen, 0x00ff00);
    GPU_API_Write_CoCmd(COLOR_RGB(0xff,0x00,0x00));
    GPU_CoCmd_FgColor(pScreen,0xff0000);
    xOffset += xDistBtwClocks;
    GPU_CoCmd_Clock(pScreen, xOffset,yOffset,cRadius,OPT_NOSECS,10,10,5,10);
    GPU_API_Write_CoCmd(COLOR_A(255));
    GPU_CoCmd_Text(pScreen,xOffset, (yOffset + cRadius + 6), 26, OPT_CENTER, "No Secs");//text info
    /* no background color */
    GPU_CoCmd_BgColor(pScreen, 0x00ffff);
    GPU_API_Write_CoCmd(COLOR_RGB(0xff,0xff,0x00));
    xOffset += xDistBtwClocks;
    GPU_CoCmd_Clock(pScreen, xOffset,yOffset,cRadius,OPT_NOBACK,10,10,5,10);
    GPU_CoCmd_Text(pScreen,xOffset, (yOffset + cRadius + 6), 26, OPT_CENTER, "No BG");//text info
    /* No ticks */
    GPU_CoCmd_BgColor(pScreen, 0xff00ff);
    GPU_API_Write_CoCmd(COLOR_RGB(0x00,0xff,0xff));
    xOffset += xDistBtwClocks;
    GPU_CoCmd_Clock(pScreen, xOffset,yOffset,cRadius,OPT_NOTICKS,10,10,5,10);
    GPU_CoCmd_Text(pScreen,xOffset, (yOffset + cRadius + 6), 26, OPT_CENTER, "No Ticks");//text info
    /* No hands */
    GPU_CoCmd_BgColor(pScreen, 0x808080);
    GPU_API_Write_CoCmd(COLOR_RGB(0x00,0xff,0x00));
    xOffset += xDistBtwClocks;
    GPU_CoCmd_Clock(pScreen, xOffset,yOffset,cRadius,OPT_NOHANDS,10,10,5,10);
    GPU_CoCmd_Text(pScreen,xOffset, (yOffset + cRadius + 6), 26, OPT_CENTER, "No Hands");//text info
    /* Bigger clock */
    yOffset += (cRadius + 10);
    cRadius = DispHeight - (2*cRadius + 5 + 10);//calculate radius based on remaining height
    cRadius = (cRadius - 5 - 20)/2;
    xOffset = cRadius + 10;
    yOffset += cRadius + 5;
    GPU_API_Write_CoCmd(COLOR_RGB(0x00,0x00,0xff));
    GPU_CoCmd_Clock(pScreen, xOffset,yOffset,cRadius,0,10,10,5,10);

    xOffset += 2 * cRadius + 10;
    /* Lena clock with no background and no ticks */
    GPU_API_Write_CoCmd(LINE_WIDTH(10*16));
    GPU_API_Write_CoCmd(COLOR_RGB(0xff,0xff,0xff));
    GPU_API_Write_CoCmd(BEGIN(RECTS));
    GPU_API_Write_CoCmd(VERTEX2F((xOffset - cRadius + 10)*16,(yOffset - cRadius + 10)*16));
    GPU_API_Write_CoCmd(VERTEX2F((xOffset + cRadius - 10)*16,(yOffset + cRadius - 10)*16));
    GPU_API_Write_CoCmd(END());
    GPU_API_Write_CoCmd(COLOR_A(0xff));
    GPU_API_Write_CoCmd(COLOR_RGB(0xff,0xff,0xff));
    GPU_API_Write_CoCmd(COLOR_MASK(0,0,0,1));
    GPU_API_Write_CoCmd(CLEAR(1,1,1));
    GPU_API_Write_CoCmd(BEGIN(RECTS));
    GPU_API_Write_CoCmd(VERTEX2F((xOffset - cRadius + 12)*16,(yOffset - cRadius + 12)*16));
    GPU_API_Write_CoCmd(VERTEX2F((xOffset + cRadius - 12)*16,(yOffset + cRadius - 12)*16));
    GPU_API_Write_CoCmd(END());
    GPU_API_Write_CoCmd(COLOR_MASK(1,1,1,1));
    GPU_API_Write_CoCmd(BLEND_FUNC(DST_ALPHA,ONE_MINUS_DST_ALPHA));
    /* Lena bitmap - scale proportionately wrt output resolution */
    GPU_CoCmd_LoadIdentity(pScreen);
    GPU_CoCmd_Scale(pScreen, 65536*2*cRadius/Lena_Bitmap_RawData_Header[0].Width,65536*2*cRadius/Lena_Bitmap_RawData_Header[0].Height);
    GPU_CoCmd_SetMatrix(pScreen );
    GPU_API_Write_CoCmd(BEGIN(BITMAPS));
    GPU_API_Write_CoCmd(BITMAP_SOURCE(0));
    GPU_API_Write_CoCmd(BITMAP_LAYOUT(Lena_Bitmap_RawData_Header[0].Format,
        Lena_Bitmap_RawData_Header[0].Stride,Lena_Bitmap_RawData_Header[0].Height));
    GPU_API_Write_CoCmd(BITMAP_SIZE(BILINEAR,BORDER,BORDER,
        2*cRadius,2*cRadius));
    GPU_API_Write_CoCmd(VERTEX2F((xOffset - cRadius)*16,(yOffset - cRadius)*16));
    GPU_API_Write_CoCmd(END());
    GPU_API_Write_CoCmd(BLEND_FUNC(SRC_ALPHA,ONE_MINUS_SRC_ALPHA));
    GPU_CoCmd_LoadIdentity(pScreen);
    GPU_CoCmd_SetMatrix(pScreen );
    GPU_API_Write_CoCmd(COLOR_RGB(0xff,0xff,0xff));
    GPU_CoCmd_Clock(pScreen, xOffset,yOffset,cRadius,OPT_NOTICKS | OPT_NOBACK,10,10,5,10);
    GPU_API_Write_CoCmd(DISPLAY());
    GPU_CoCmd_Swap(pScreen);

    /* Wait till coprocessor completes the operation */
    GPU_HAL_WaitCmdfifo_empty(pScreen);
    GPU_APP_ENABLE_DELAY();
    }
}

/* API to demonstrate gauge widget */
ft_void_t GPU_APP_CoPro_Widget_Gauge()
{
    /*************************************************************************/
    /* Below code demonstrates the usage of gauge function. Gauge can be     */
    /* constructed using flat or 3d effect. Gauge background and foreground  */
    /* colors can be set by gbcolor and colorrgb. Gauge can be constructed   */
    /* with multiple options such as no background, no minors/majors and     */
    /* no pointer.                                                           */
    /*************************************************************************/
    {
    ft_int16_t xOffset,yOffset,cRadius,xDistBtwClocks;

    xDistBtwClocks = DispWidth/5;
    cRadius = xDistBtwClocks/2 - DispWidth/64;

    /* Download the bitmap data */
    GPU_HAL_WrMem(pScreen, RAM_G,(ft_uint8_t *)&Lena_Bitmap_RawData[Lena_Bitmap_RawData_Header[0].ArrayOffset],
        Lena_Bitmap_RawData_Header[0].Stride*Lena_Bitmap_RawData_Header[0].Height);

    /* Draw gauge with blue as background and read as needle color */

    GPU_CoCmd_Dlstart(pScreen);
    GPU_API_Write_CoCmd(CLEAR_COLOR_RGB(64,64,64));
    GPU_API_Write_CoCmd(CLEAR(1,1,1));
    GPU_API_Write_CoCmd(COLOR_RGB(255,255,255));
    /* flat effect and default color background */
    xOffset = xDistBtwClocks/2;
    yOffset = cRadius + 5;
    GPU_CoCmd_BgColor(pScreen, 0x0000ff);
    GPU_API_Write_CoCmd(COLOR_RGB(0xff,0x00,0x00));
    GPU_CoCmd_Gauge(pScreen, xOffset,yOffset,cRadius,OPT_FLAT,5,4,45,100);
    GPU_CoCmd_Text(pScreen,xOffset, (yOffset + cRadius + 6), 26, OPT_CENTER, "Flat effect");//text info
    /* 3d effect */
    GPU_CoCmd_BgColor(pScreen, 0x00ff00);
    GPU_API_Write_CoCmd(COLOR_RGB(0xff,0x00,0x00));
    GPU_CoCmd_FgColor(pScreen,0xff0000);
    xOffset += xDistBtwClocks;
    GPU_CoCmd_Gauge(pScreen, xOffset,yOffset,cRadius,0,5,1,60,100);
    GPU_CoCmd_Text(pScreen,xOffset, (yOffset + cRadius + 6), 26, OPT_CENTER, "3d effect");//text info
    /* no background color */
    GPU_CoCmd_BgColor(pScreen, 0x00ffff);
    GPU_API_Write_CoCmd(COLOR_RGB(0xff,0xff,0x00));
    xOffset += xDistBtwClocks;
    GPU_CoCmd_Gauge(pScreen, xOffset,yOffset,cRadius,OPT_NOBACK,1,6,90,100);
    GPU_API_Write_CoCmd(COLOR_A(255));
    GPU_CoCmd_Text(pScreen,xOffset, (yOffset + cRadius + 6), 26, OPT_CENTER, "No BG");//text info
    /* No ticks */
    GPU_CoCmd_BgColor(pScreen, 0xff00ff);
    GPU_API_Write_CoCmd(COLOR_RGB(0x00,0xff,0xff));
    xOffset += xDistBtwClocks;
    GPU_CoCmd_Gauge(pScreen, xOffset,yOffset,cRadius,OPT_NOTICKS,5,4,20,100);
    GPU_CoCmd_Text(pScreen,xOffset, (yOffset + cRadius + 6), 26, OPT_CENTER, "No Ticks");//text info
    /* No hands */
    GPU_CoCmd_BgColor(pScreen, 0x808080);
    GPU_API_Write_CoCmd(COLOR_RGB(0x00,0xff,0x00));
    xOffset += xDistBtwClocks;
    GPU_CoCmd_Gauge(pScreen, xOffset,yOffset,cRadius,OPT_NOHANDS,5,4,55,100);
    GPU_API_Write_CoCmd(COLOR_A(255));
    GPU_CoCmd_Text(pScreen,xOffset, (yOffset + cRadius + 6), 26, OPT_CENTER, "No Hands");//text info
    /* Bigger gauge */
    yOffset += cRadius + 10;
    cRadius = DispHeight - (2*cRadius + 5 + 10);//calculate radius based on remaining height
    cRadius = (cRadius - 5 - 20)/2;
    xOffset = cRadius + 10;
    yOffset += cRadius + 5;
    GPU_CoCmd_BgColor(pScreen, 0x808000);
    GPU_API_Write_CoCmd(COLOR_RGB(0xff,0xff,0xff));
    GPU_CoCmd_Gauge(pScreen, xOffset,yOffset,cRadius,OPT_NOPOINTER,5,4,80,100);
    GPU_API_Write_CoCmd(COLOR_RGB(0xff,0x00,0x00));
    GPU_CoCmd_Gauge(pScreen, xOffset,yOffset,cRadius,OPT_NOTICKS | OPT_NOBACK,5,4,30,100);

    xOffset += 2*cRadius + 10;
    /* Lena guage with no background and no ticks */
    GPU_API_Write_CoCmd(POINT_SIZE(cRadius*16));
    GPU_API_Write_CoCmd(COLOR_RGB(0xff,0xff,0xff));
    GPU_API_Write_CoCmd(BEGIN(POINTS));
    GPU_API_Write_CoCmd(VERTEX2F(xOffset*16,yOffset*16));
    GPU_API_Write_CoCmd(END());
    GPU_API_Write_CoCmd(COLOR_A(0xff));
    GPU_API_Write_CoCmd(COLOR_RGB(0xff,0xff,0xff));
    GPU_API_Write_CoCmd(COLOR_MASK(0,0,0,1));
    GPU_API_Write_CoCmd(CLEAR(1,1,1));
    GPU_API_Write_CoCmd(BEGIN(POINTS));
    GPU_API_Write_CoCmd(POINT_SIZE((cRadius - 2)*16));
    GPU_API_Write_CoCmd(VERTEX2F(xOffset*16,yOffset*16));
    GPU_API_Write_CoCmd(END());
    GPU_API_Write_CoCmd(COLOR_MASK(1,1,1,1));
    GPU_API_Write_CoCmd(BLEND_FUNC(DST_ALPHA,ONE_MINUS_DST_ALPHA));
    /* Lena bitmap - scale proportionately wrt output resolution */
    GPU_CoCmd_LoadIdentity(pScreen);
    GPU_CoCmd_Scale(pScreen, 65536*2*cRadius/Lena_Bitmap_RawData_Header[0].Width,65536*2*cRadius/Lena_Bitmap_RawData_Header[0].Height);
    GPU_CoCmd_SetMatrix(pScreen );
    GPU_API_Write_CoCmd(BEGIN(BITMAPS));
    GPU_API_Write_CoCmd(BITMAP_SOURCE(0));
    GPU_API_Write_CoCmd(BITMAP_LAYOUT(Lena_Bitmap_RawData_Header[0].Format,
        Lena_Bitmap_RawData_Header[0].Stride,Lena_Bitmap_RawData_Header[0].Height));
    GPU_API_Write_CoCmd(BITMAP_SIZE(BILINEAR,BORDER,BORDER,
        2*cRadius,2*cRadius));
    GPU_API_Write_CoCmd(VERTEX2F((xOffset - cRadius)*16,(yOffset - cRadius)*16));
    GPU_API_Write_CoCmd(END());
    GPU_API_Write_CoCmd(BLEND_FUNC(SRC_ALPHA,ONE_MINUS_SRC_ALPHA));
    GPU_CoCmd_LoadIdentity(pScreen);
    GPU_CoCmd_SetMatrix(pScreen );

    GPU_API_Write_CoCmd(COLOR_RGB(0xff,0xff,0xff));
    GPU_CoCmd_Gauge(pScreen, xOffset,yOffset,cRadius,OPT_NOTICKS | OPT_NOBACK,5,4,30,100);
    GPU_API_Write_CoCmd(DISPLAY());
    GPU_CoCmd_Swap(pScreen);

    /* Wait till coprocessor completes the operation */
    GPU_HAL_WaitCmdfifo_empty(pScreen);
    GPU_APP_ENABLE_DELAY();
    }
}

/* API to demonstrate gradient widget */
ft_void_t GPU_APP_CoPro_Widget_Gradient()
{
    /*************************************************************************/
    /* Below code demonstrates the usage of gradient function. Gradient func */
    /* can be used to construct three effects - horizontal, vertical and     */
    /* diagonal effects.                                                      */
    /*************************************************************************/
    {
    ft_int16_t wScissor,hScissor,xOffset,yOffset;

    wScissor = ((DispWidth - 4*10)/3);
    hScissor = ((DispHeight - 3*20)/2);
    xOffset = 10;
    yOffset = 20;
    /* Draw gradient  */

    GPU_CoCmd_Dlstart(pScreen);
    GPU_API_Write_CoCmd(CLEAR_COLOR_RGB(64,64,64));
    GPU_API_Write_CoCmd(CLEAR(1,1,1));
    GPU_API_Write_CoCmd(COLOR_RGB(255,255,255));
    GPU_API_Write_CoCmd(SCISSOR_SIZE(wScissor,hScissor));
    /* Horizontal gradient effect */
    GPU_API_Write_CoCmd(SCISSOR_XY(xOffset,yOffset));//clip the display
    GPU_CoCmd_Gradient(pScreen, xOffset,yOffset,0x808080,(xOffset + wScissor),yOffset,0xffff00);
    /* Vertical gradient effect */
    xOffset += wScissor + 10;
    GPU_API_Write_CoCmd(SCISSOR_XY(xOffset,yOffset));//clip the display
    GPU_CoCmd_Gradient(pScreen, xOffset,yOffset,0xff0000,xOffset,(yOffset + hScissor),0x00ff00);
    /* diagonal gradient effect */
    xOffset += wScissor + 10;
    GPU_API_Write_CoCmd(SCISSOR_XY(xOffset,yOffset));//clip the display
    GPU_CoCmd_Gradient(pScreen, xOffset,yOffset,0x800000,(xOffset + wScissor),(yOffset + hScissor),0xffffff);
    /* Diagonal gradient with text info */
    xOffset = 10;
    yOffset += hScissor + 20;
    GPU_API_Write_CoCmd(SCISSOR_SIZE(wScissor,30));
    GPU_API_Write_CoCmd(SCISSOR_XY(xOffset,(yOffset + hScissor/2 - 15)));//clip the display
    GPU_CoCmd_Gradient(pScreen, xOffset,(yOffset + hScissor/2 - 15),0x606060,(xOffset + wScissor),(yOffset + hScissor/2 + 15),0x404080);
    GPU_CoCmd_Text(pScreen,(xOffset + wScissor/2), (yOffset + hScissor/2), 28, OPT_CENTER, "Heading 1");//text info

    /* Draw horizontal, vertical and diagonal with alpha */
    xOffset += wScissor + 10;
    GPU_API_Write_CoCmd(SCISSOR_SIZE(wScissor,hScissor));
    GPU_API_Write_CoCmd(SCISSOR_XY(xOffset,yOffset));//clip the display
    GPU_CoCmd_Gradient(pScreen, xOffset,yOffset,0x808080,(xOffset + wScissor),yOffset,0xffff00);
    wScissor -= 30; hScissor -= 30;
    GPU_API_Write_CoCmd(SCISSOR_SIZE(wScissor,hScissor));
    xOffset += 15; yOffset += 15;
    GPU_API_Write_CoCmd(SCISSOR_XY(xOffset,yOffset));//clip the display
    GPU_CoCmd_Gradient(pScreen, xOffset,yOffset,0x800000,xOffset,(yOffset + hScissor),0xffffff);
    wScissor -= 30; hScissor -= 30;
    GPU_API_Write_CoCmd(SCISSOR_SIZE(wScissor,hScissor));
    xOffset += 15; yOffset += 15;
    GPU_API_Write_CoCmd(SCISSOR_XY(xOffset,yOffset));//clip the display
    GPU_CoCmd_Gradient(pScreen, xOffset,yOffset,0x606060,(xOffset + wScissor),(yOffset + hScissor),0x404080);

    /* Display the text wrt gradient */
    wScissor = ((DispWidth - 4*10)/3);
    hScissor = ((DispHeight - 3*20)/2);
    xOffset = 10 + wScissor/2;
    yOffset = 20 + hScissor + 5;
    GPU_API_Write_CoCmd(SCISSOR_XY(0,0));//set to default values
    GPU_API_Write_CoCmd(SCISSOR_SIZE(512,512));
    GPU_CoCmd_Text(pScreen,xOffset,yOffset, 26, OPT_CENTER, "Horizontal grad");//text info
    xOffset += wScissor + 10;
    GPU_CoCmd_Text(pScreen,xOffset,yOffset, 26, OPT_CENTER, "Vertical grad");//text info
    xOffset += wScissor + 10;
    GPU_CoCmd_Text(pScreen,xOffset,yOffset, 26, OPT_CENTER, "Diagonal grad");//text info

    GPU_API_Write_CoCmd(DISPLAY());
    GPU_CoCmd_Swap(pScreen);

    /* Wait till coprocessor completes the operation */
    GPU_HAL_WaitCmdfifo_empty(pScreen);
    GPU_APP_ENABLE_DELAY();
    }
}

#define GPU_API_COPRO_WIDGET_KEYS_INTERACTIVE_TEXTSIZE (128) // xxx don't make this too big in arduino world

ft_void_t GPU_APP_CoPro_Widget_Keys_Interactive()
{
    /*************************************************************************/
    /* Below code demonstrates the usage of keys function. keys function     */
    /* draws buttons with characters given as input parameters. Keys support */
    /* Flat and 3D effects, draw at (x,y) coordinates or center of the display*/
    /* , inbuilt or custom fonts can be used for key display                 */
    /*************************************************************************/
    {
      ft_int32_t loopflag = 600;
      ft_int16_t TextFont = 29, ButtonW = 30, ButtonH = 30, yBtnDst = 5, yOffset; // xOffset; xxx
      ft_char8_t DispText[GPU_API_COPRO_WIDGET_KEYS_INTERACTIVE_TEXTSIZE], CurrChar = '|';
      ft_uint8_t CurrTag = 0,PrevTag = 0,Pendown = 1;
      ft_int32_t CurrTextIdx = 0;

       while(loopflag --)
       {


       /* Check the user input and then add the characters into array */
       CurrTag = GPU_HAL_Rd8(pScreen,REG_TOUCH_TAG);
       Pendown = ((GPU_HAL_Rd32(pScreen,REG_TOUCH_DIRECT_XY)>>31) & 0x01);

       CurrChar = CurrTag;
       if(0 == CurrTag)
       {
              CurrChar = '|';
       }

       /* check whether pwndown has happened */
       if(( 1 == Pendown) && (0 != PrevTag))
       {
              CurrTextIdx++;
              /* clear all the characters after 100 are pressed */
              if(CurrTextIdx > 24)
              {
                     CurrTextIdx = 0;
              }
       }

       GPU_CoCmd_Dlstart(pScreen);
       GPU_API_Write_CoCmd(CLEAR_COLOR_RGB(64,64,64));
       GPU_API_Write_CoCmd(CLEAR(1,1,1));
       GPU_API_Write_CoCmd(COLOR_RGB(0xff,0xff,0xff));
       /* Draw text entered by user */
       /* make sure the array is a string */
       DispText[CurrTextIdx] = CurrChar;
       DispText[CurrTextIdx + 1] = '\0';

       GPU_API_Write_CoCmd(TAG_MASK(0));
       GPU_CoCmd_Text(pScreen,DispWidth/2, 40, TextFont, OPT_CENTER, DispText);//text info
       GPU_API_Write_CoCmd(TAG_MASK(1));


       yOffset = 80 + 10;
       /* Construct a simple keyboard - note that the tags associated with the keys are the character values given in the arguments */
       GPU_CoCmd_FgColor(pScreen,0x404080);
       GPU_CoCmd_GradColor(pScreen,0x00ff00);
       GPU_CoCmd_Keys(pScreen, yBtnDst, yOffset, 10*ButtonW, ButtonH, TextFont, (OPT_CENTER | CurrTag), "qwertyuiop");
       GPU_CoCmd_GradColor(pScreen,0x00ffff);
       yOffset += ButtonH + yBtnDst;
       GPU_CoCmd_Keys(pScreen, yBtnDst, yOffset, 10*ButtonW, ButtonH, TextFont, (OPT_CENTER | CurrTag), "asdfghijkl");
       GPU_CoCmd_GradColor(pScreen,0xffff00);
       yOffset += ButtonH + yBtnDst;
       GPU_CoCmd_Keys(pScreen, yBtnDst, yOffset, 10*ButtonW, ButtonH, TextFont, (OPT_CENTER | CurrTag), "zxcvbnm");//highlight button z
       yOffset += ButtonH + yBtnDst;
       GPU_API_Write_CoCmd(TAG(' '));
       if(' ' == CurrTag)
       {
              GPU_CoCmd_Button(pScreen,yBtnDst, yOffset, 10*ButtonW, ButtonH, TextFont, OPT_CENTER | OPT_FLAT, " ");//mandatory to give '\0' at the end to make sure coprocessor understands the string end
       }
       else
       {
              GPU_CoCmd_Button(pScreen,yBtnDst, yOffset, 10*ButtonW, ButtonH, TextFont, OPT_CENTER, " ");//mandatory to give '\0' at the end to make sure coprocessor understands the string end
       }
       yOffset = 80 + 10;
       GPU_CoCmd_Keys(pScreen, 11*ButtonW, yOffset, 3*ButtonW, ButtonH, TextFont, (0 | CurrTag), "789");
       yOffset += ButtonH + yBtnDst;
       GPU_CoCmd_Keys(pScreen, 11*ButtonW, yOffset, 3*ButtonW, ButtonH, TextFont, (0 | CurrTag), "456");
       yOffset += ButtonH + yBtnDst;
       GPU_CoCmd_Keys(pScreen, 11*ButtonW, yOffset, 3*ButtonW, ButtonH, TextFont, (0 | CurrTag), "123");
       yOffset += ButtonH + yBtnDst;
       GPU_API_Write_CoCmd(COLOR_A(255));
       GPU_CoCmd_Keys(pScreen, 11*ButtonW, yOffset, 3*ButtonW, ButtonH, TextFont, (0 | CurrTag), "0.");//highlight button 0
       GPU_API_Write_CoCmd(DISPLAY());
       GPU_CoCmd_Swap(pScreen);

       /* Wait till coprocessor completes the operation */
       GPU_HAL_WaitCmdfifo_empty(pScreen);
       GPU_APP_ENABLE_DELAY_MS( 10 );//sleep for 10 ms
       PrevTag = CurrTag;
       }
    }
}

/* API to demonstrate keys widget */
ft_void_t GPU_APP_CoPro_Widget_Keys()
{
    /*************************************************************************/
    /* Below code demonstrates the usage of keys function. keys function     */
    /* draws buttons with characters given as input parameters. Keys support */
    /* Flat and 3D effects, draw at (x,y) coordinates or center of the display*/
    /* , inbuilt or custom fonts can be used for key display                 */
    /*************************************************************************/
    {
        ft_int16_t TextFont = 29,ButtonW = 30,ButtonH = 30,yBtnDst = 5,yOffset,xOffset;
#ifdef GPU_API_DISPLAY_QVGA
        TextFont = 27;
        ButtonW = 22;
        ButtonH = 22;
        yBtnDst = 3;
#endif

    GPU_CoCmd_Dlstart(pScreen);
    GPU_API_Write_CoCmd(CLEAR_COLOR_RGB(64,64,64));
    GPU_API_Write_CoCmd(CLEAR(1,1,1));
    GPU_API_Write_CoCmd(COLOR_RGB(0xff,0xff,0xff));
    /* Draw keys with flat effect */
    GPU_CoCmd_FgColor(pScreen,0x404080);
    GPU_CoCmd_Keys(pScreen, 10, 10, 4*ButtonW, 30, TextFont, OPT_FLAT, "ABCD");
    GPU_CoCmd_Text(pScreen,10, 40, 26, 0, "Flat effect");//text info
    /* Draw keys with 3d effect */
    GPU_CoCmd_FgColor(pScreen,0x800000);
    xOffset = 4*ButtonW + 20;
    GPU_CoCmd_Keys(pScreen, xOffset, 10, 4*ButtonW, 30, TextFont, 0, "ABCD");
    GPU_CoCmd_Text(pScreen,xOffset, 40, 26, 0, "3D effect");//text info
    /* Draw keys with center option */
    GPU_CoCmd_FgColor(pScreen,0xffff000);
    xOffset += 4*ButtonW + 20;
    GPU_CoCmd_Keys(pScreen, xOffset, 10, (DispWidth - 230), 30, TextFont, OPT_CENTER, "ABCD");
    GPU_CoCmd_Text(pScreen,xOffset, 40, 26, 0, "Option Center");//text info

    yOffset = 80 + 10;
    /* Construct a simple keyboard - note that the tags associated with the keys are the character values given in the arguments */
    GPU_CoCmd_FgColor(pScreen,0x404080);
    GPU_CoCmd_GradColor(pScreen,0x00ff00);
    GPU_CoCmd_Keys(pScreen, yBtnDst, yOffset, 10*ButtonW, ButtonH, TextFont, OPT_CENTER, "qwertyuiop");
    GPU_CoCmd_GradColor(pScreen,0x00ffff);
    yOffset += ButtonH + yBtnDst;
    GPU_CoCmd_Keys(pScreen, yBtnDst, yOffset, 10*ButtonW, ButtonH, TextFont, OPT_CENTER, "asdfghijkl");
    GPU_CoCmd_GradColor(pScreen,0xffff00);
    yOffset += ButtonH + yBtnDst;
    GPU_CoCmd_Keys(pScreen, yBtnDst, yOffset, 10*ButtonW, ButtonH, TextFont, (OPT_CENTER | 'z'), "zxcvbnm");//highlight button z
    yOffset += ButtonH + yBtnDst;
    GPU_CoCmd_Button(pScreen,yBtnDst, yOffset, 10*ButtonW, ButtonH, TextFont, OPT_CENTER, " ");//mandatory to give '\0' at the end to make sure coprocessor understands the string end
    yOffset = 80 + 10;
    GPU_CoCmd_Keys(pScreen, 11*ButtonW, yOffset, 3*ButtonW, ButtonH, TextFont, 0, "789");
    yOffset += ButtonH + yBtnDst;
    GPU_CoCmd_Keys(pScreen, 11*ButtonW, yOffset, 3*ButtonW, ButtonH, TextFont, 0, "456");
    yOffset += ButtonH + yBtnDst;
    GPU_CoCmd_Keys(pScreen, 11*ButtonW, yOffset, 3*ButtonW, ButtonH, TextFont, 0, "123");
    yOffset += ButtonH + yBtnDst;
    GPU_API_Write_CoCmd(COLOR_A(255));
    GPU_CoCmd_Keys(pScreen, 11*ButtonW, yOffset, 3*ButtonW, ButtonH, TextFont, (0 | '0'), "0.");//highlight button 0
    GPU_API_Write_CoCmd(DISPLAY());
    GPU_CoCmd_Swap(pScreen);

    /* Wait till coprocessor completes the operation */
    GPU_HAL_WaitCmdfifo_empty(pScreen);
    GPU_APP_ENABLE_DELAY();
    }
}
/* API to demonstrate progress bar widget */
ft_void_t GPU_APP_CoPro_Widgetrogressbar()
{
    /*************************************************************************/
    /* Below code demonstrates the usage of progress function. Progress func */
    /* draws process bar with fgcolor for the % completion and bgcolor for   */
    /* % remaining. Progress bar supports flat and 3d effets                 */
    /*************************************************************************/
    {
        ft_int16_t xOffset,yOffset,yDist = DispWidth/12,ySz = DispWidth/24;

    GPU_CoCmd_Dlstart(pScreen);
    GPU_API_Write_CoCmd(CLEAR_COLOR_RGB(64,64,64));
    GPU_API_Write_CoCmd(CLEAR(1,1,1));
    GPU_API_Write_CoCmd(COLOR_RGB(0xff,0xff,0xff));
    /* Draw progress bar with flat effect */
    GPU_API_Write_CoCmd(COLOR_RGB(0xff,0xff,0xff));
    GPU_CoCmd_BgColor(pScreen, 0x404080);
    GPU_CoCmd_Progress(pScreen, 20, 10, 120, 20, OPT_FLAT, 50, 100);//note that h/2 will be added on both sides of the progress bar
    GPU_API_Write_CoCmd(COLOR_A(255));
    GPU_CoCmd_Text(pScreen,20, 40, 26, 0, "Flat effect");//text info
    /* Draw progress bar with 3d effect */
    GPU_API_Write_CoCmd(COLOR_RGB(0x00,0xff,0x00));
    GPU_CoCmd_BgColor(pScreen, 0x800000);
    GPU_CoCmd_Progress(pScreen, 180, 10, 120, 20, 0, 75, 100);
    GPU_API_Write_CoCmd(COLOR_A(255));
    GPU_CoCmd_Text(pScreen,180, 40, 26, 0, "3D effect");//text info
    /* Draw progress bar with 3d effect and string on top */
    GPU_API_Write_CoCmd(COLOR_RGB(0xff,0x00,0x00));
    GPU_CoCmd_BgColor(pScreen, 0x000080);
    GPU_CoCmd_Progress(pScreen, 30, 60, 120, 30, 0, 19660, 65535);
    GPU_API_Write_CoCmd(COLOR_RGB(0xff,0xff,0xff));
    GPU_CoCmd_Text(pScreen,78, 68, 26, 0, "30 %");//text info

    xOffset = 20;yOffset = 120;
    GPU_API_Write_CoCmd(COLOR_RGB(0x00,0xa0,0x00));
    GPU_CoCmd_BgColor(pScreen, 0x800000);
    GPU_CoCmd_Progress(pScreen, xOffset, yOffset, 150, ySz, 0, 10, 100);
    GPU_CoCmd_BgColor(pScreen, 0x000080);
    yOffset += yDist;
    GPU_CoCmd_Progress(pScreen, xOffset, yOffset, 150, ySz, 0, 40, 100);
    GPU_CoCmd_BgColor(pScreen, 0xffff00);
    yOffset += yDist;
    GPU_CoCmd_Progress(pScreen, xOffset, yOffset, 150, ySz, 0, 70, 100);
    GPU_CoCmd_BgColor(pScreen, 0x808080);
    yOffset += yDist;
    GPU_CoCmd_Progress(pScreen, xOffset, yOffset, 150, ySz, 0, 90, 100);

    GPU_CoCmd_Text(pScreen,xOffset + 180, 80, 26, 0, "40 % TopBottom");//text info
    GPU_CoCmd_Progress(pScreen, xOffset + 180, 100, ySz, 150, 0, 40, 100);

    GPU_API_Write_CoCmd(DISPLAY());
    GPU_CoCmd_Swap(pScreen);

    /* Wait till coprocessor completes the operation */
    GPU_HAL_WaitCmdfifo_empty(pScreen);
    GPU_APP_ENABLE_DELAY();
    }
}
/* API to demonstrate scroll widget */
ft_void_t GPU_APP_CoPro_Widget_Scroll()
{
    /*************************************************************************/
    /* Below code demonstrates the usage of scroll function. Scroll function */
    /* draws scroll bar with fgcolor for inner color and current location and*/
    /* can be given by val parameter */
    /*************************************************************************/
    {
        ft_int16_t xOffset,yOffset,xDist = DispWidth/12,yDist = DispWidth/12,wSz;


    GPU_CoCmd_Dlstart(pScreen);
    GPU_API_Write_CoCmd(CLEAR_COLOR_RGB(64,64,64));
    GPU_API_Write_CoCmd(CLEAR(1,1,1));
    GPU_API_Write_CoCmd(COLOR_RGB(0xff,0xff,0xff));
    /* Draw scroll bar with flat effect */
    GPU_CoCmd_FgColor(pScreen,0xffff00);
    GPU_CoCmd_BgColor(pScreen, 0x404080);
    GPU_CoCmd_Scrollbar(pScreen, 20, 10, 120, 8, OPT_FLAT, 20, 30, 100);//note that h/2 size will be added on both sides of the progress bar
    GPU_API_Write_CoCmd(COLOR_A(255));
    GPU_CoCmd_Text(pScreen,20, 40, 26, 0, "Flat effect");//text info
    /* Draw scroll bar with 3d effect */
    GPU_CoCmd_FgColor(pScreen,0x00ff00);
    GPU_CoCmd_BgColor(pScreen, 0x800000);
    GPU_CoCmd_Scrollbar(pScreen, 180, 10, 120, 8, 0, 20, 30, 100);
    GPU_API_Write_CoCmd(COLOR_A(255));
    GPU_CoCmd_Text(pScreen,180, 40, 26, 0, "3D effect");//text info

    xOffset = 20;
    yOffset = 120;
    wSz = ((DispWidth/2) - 40);
    /* Draw horizontal scroll bars */
    GPU_CoCmd_FgColor(pScreen,0x00a000);
    GPU_CoCmd_BgColor(pScreen, 0x800000);
    GPU_CoCmd_Scrollbar(pScreen, xOffset, yOffset, wSz, 8, 0, 10, 30, 100);
    GPU_CoCmd_BgColor(pScreen, 0x000080);
    yOffset += yDist;
    GPU_CoCmd_Scrollbar(pScreen, xOffset, yOffset, wSz, 8, 0, 30, 30, 100);
    GPU_API_Write_CoCmd(COLOR_A(255));
    GPU_CoCmd_BgColor(pScreen, 0xffff00);
    yOffset += yDist;
    GPU_CoCmd_Scrollbar(pScreen, xOffset, yOffset, wSz, 8, 0, 50, 30, 100);
    GPU_CoCmd_BgColor(pScreen, 0x808080);
    yOffset += yDist;
    GPU_CoCmd_Scrollbar(pScreen, xOffset, yOffset, wSz, 8, 0, 70, 30, 100);

    xOffset = (DispWidth/2) + 40;
    yOffset = 80;
    wSz = (DispHeight - 100);
    /* draw vertical scroll bars */
    GPU_CoCmd_BgColor(pScreen, 0x800000);
    GPU_CoCmd_Scrollbar(pScreen, xOffset, yOffset, 8, wSz, 0, 10, 30, 100);
    GPU_CoCmd_BgColor(pScreen, 0x000080);
    xOffset += xDist;
    GPU_CoCmd_Scrollbar(pScreen, xOffset, yOffset, 8, wSz, 0, 30, 30, 100);
    GPU_API_Write_CoCmd(COLOR_A(255));
    GPU_CoCmd_BgColor(pScreen, 0xffff00);
    xOffset += xDist;
    GPU_CoCmd_Scrollbar(pScreen, xOffset, yOffset, 8, wSz, 0, 50, 30, 100);
    GPU_CoCmd_BgColor(pScreen, 0x808080);
    xOffset += xDist;
    GPU_CoCmd_Scrollbar(pScreen, xOffset, yOffset, 8, wSz, 0, 70, 30, 100);

    GPU_API_Write_CoCmd(DISPLAY());
    GPU_CoCmd_Swap(pScreen);

    /* Wait till coprocessor completes the operation */
    GPU_HAL_WaitCmdfifo_empty(pScreen);
    GPU_APP_ENABLE_DELAY();
    }
}
/* API to demonstrate slider widget */
ft_void_t GPU_APP_CoPro_Widget_Slider()
{
    /*************************************************************************/
    /* Below code demonstrates the usage of slider function. Slider function */
    /* draws slider bar with fgcolor for inner color and bgcolor for the knob*/
    /* , contains input parameter for position of the knob                   */
    /*************************************************************************/
    {
        ft_int16_t xOffset,yOffset,xDist = DispWidth/12,yDist = DispWidth/12,wSz;

    GPU_CoCmd_Dlstart(pScreen);
    GPU_API_Write_CoCmd(CLEAR_COLOR_RGB(64,64,64));
    GPU_API_Write_CoCmd(CLEAR(1,1,1));
    GPU_API_Write_CoCmd(COLOR_RGB(0xff,0xff,0xff));
    /* Draw scroll bar with flat effect */
    GPU_CoCmd_FgColor(pScreen,0xffff00);
    GPU_CoCmd_BgColor(pScreen, 0x000080);
    GPU_CoCmd_Slider(pScreen, 20, 10, 120, 10, OPT_FLAT, 30, 100);//note that h/2 size will be added on both sides of the progress bar
    GPU_API_Write_CoCmd(COLOR_A(255));
    GPU_CoCmd_Text(pScreen,20, 40, 26, 0, "Flat effect");//text info
    /* Draw scroll bar with 3d effect */
    GPU_CoCmd_FgColor(pScreen,0x00ff00);
    GPU_CoCmd_BgColor(pScreen, 0x800000);
    GPU_CoCmd_Slider(pScreen, 180, 10, 120, 10, 0, 50, 100);
    GPU_API_Write_CoCmd(COLOR_A(255));
    GPU_CoCmd_Text(pScreen,180, 40, 26, 0, "3D effect");//text info

    xOffset = 20;
    yOffset = 120;
    wSz = ((DispWidth/2) - 40);
    /* Draw horizontal slider bars */
    GPU_CoCmd_FgColor(pScreen,0x00a000);
    GPU_CoCmd_BgColor(pScreen, 0x800000);
    GPU_API_Write_CoCmd(COLOR_RGB(41,1,5));
    GPU_CoCmd_Slider(pScreen, xOffset, yOffset, wSz, 10, 0, 10, 100);
    GPU_API_Write_CoCmd(COLOR_RGB(11,7,65));
    GPU_CoCmd_BgColor(pScreen, 0x000080);
    yOffset += yDist;
    GPU_CoCmd_Slider(pScreen, xOffset, yOffset, wSz, 10, 0, 30, 100);
    GPU_API_Write_CoCmd(COLOR_A(255));
    GPU_CoCmd_BgColor(pScreen, 0xffff00);
    GPU_API_Write_CoCmd(COLOR_RGB(87,94,9));
    yOffset += yDist;
    GPU_CoCmd_Slider(pScreen, xOffset, yOffset, wSz, 10, 0, 50, 100);
    GPU_CoCmd_BgColor(pScreen, 0x808080);
    GPU_API_Write_CoCmd(COLOR_RGB(51,50,52));
    yOffset += yDist;
    GPU_CoCmd_Slider(pScreen, xOffset, yOffset, wSz, 10, 0, 70, 100);

    xOffset = (DispWidth/2) + 40;
    yOffset = 80;
    wSz = (DispHeight - 100);
    /* draw vertical slider bars */
    GPU_CoCmd_BgColor(pScreen, 0x800000);
    GPU_CoCmd_Slider(pScreen, xOffset, yOffset, 10, wSz, 0, 10, 100);
    GPU_CoCmd_BgColor(pScreen, 0x000080);
    xOffset += xDist;
    GPU_CoCmd_Slider(pScreen, xOffset, yOffset, 10, wSz, 0, 30, 100);
    GPU_API_Write_CoCmd(COLOR_A(255));
    GPU_CoCmd_BgColor(pScreen, 0xffff00);
    xOffset += xDist;
    GPU_CoCmd_Slider(pScreen, xOffset, yOffset, 10, wSz, 0, 50, 100);
    GPU_CoCmd_BgColor(pScreen, 0x808080);
    xOffset += xDist;
    GPU_CoCmd_Slider(pScreen, xOffset, yOffset, 10, wSz, 0, 70, 100);

    GPU_API_Write_CoCmd(DISPLAY());
    GPU_CoCmd_Swap(pScreen);

    /* Wait till coprocessor completes the operation */
    GPU_HAL_WaitCmdfifo_empty(pScreen);
    GPU_APP_ENABLE_DELAY();
    }
}
/*API to demonstrate dial widget */
ft_void_t GPU_APP_CoPro_Widget_Dial()
{
    /*************************************************************************/
    /* Below code demonstrates the usage of dial function. Dial function     */
    /* draws rounded bar with fgcolor for knob color and colorrgb for pointer*/
    /* , contains input parameter for angle of the pointer                   */
    /*************************************************************************/
    {

    GPU_CoCmd_Dlstart(pScreen);
    GPU_API_Write_CoCmd(CLEAR_COLOR_RGB(64,64,64));
    GPU_API_Write_CoCmd(CLEAR(1,1,1));
    GPU_API_Write_CoCmd(COLOR_RGB(0xff,0xff,0xff));
    /* Draw dial with flat effect */
    GPU_CoCmd_FgColor(pScreen,0xffff00);
    GPU_CoCmd_BgColor(pScreen, 0x000080);
    GPU_CoCmd_Dial(pScreen, 50, 50, 40, OPT_FLAT, 0.2*65535);//20%
    GPU_API_Write_CoCmd(COLOR_A(255));
    GPU_CoCmd_Text(pScreen,15, 90, 26, 0, "Flat effect");//text info
    /* Draw dial with 3d effect */
    GPU_CoCmd_FgColor(pScreen,0x00ff00);
    GPU_CoCmd_BgColor(pScreen, 0x800000);
    GPU_CoCmd_Dial(pScreen, 140, 50, 40, 0, 0.45*65535);//45%
    GPU_API_Write_CoCmd(COLOR_A(255));
    GPU_CoCmd_Text(pScreen,105, 90, 26, 0, "3D effect");//text info

    /* Draw increasing dials horizontally */
    GPU_CoCmd_FgColor(pScreen,0x800000);
    GPU_API_Write_CoCmd(COLOR_RGB(41,1,5));
    GPU_CoCmd_Dial(pScreen, 30, 160, 20, 0, 0.30*65535);
    GPU_CoCmd_Text(pScreen,20, 180, 26, 0, "30 %");//text info
    GPU_API_Write_CoCmd(COLOR_RGB(11,7,65));
    GPU_CoCmd_FgColor(pScreen,0x000080);
    GPU_CoCmd_Dial(pScreen, 100, 160, 40, 0, 0.45*65535);
    GPU_API_Write_CoCmd(COLOR_A(255));
    GPU_CoCmd_Text(pScreen,90, 200, 26, 0, "45 %");//text info
    GPU_CoCmd_FgColor(pScreen,0xffff00);
    GPU_API_Write_CoCmd(COLOR_RGB(87,94,9));
    GPU_CoCmd_Dial(pScreen, 210, 160, 60, 0, 0.60*65535);
    GPU_CoCmd_Text(pScreen,200, 220, 26, 0, "60 %");//text info
    GPU_CoCmd_FgColor(pScreen,0x808080);

    GPU_API_Write_CoCmd(DISPLAY());
    GPU_CoCmd_Swap(pScreen);

    /* Wait till coprocessor completes the operation */
    GPU_HAL_WaitCmdfifo_empty(pScreen);
    GPU_APP_ENABLE_DELAY();
    }
}
/* API to demonstrate toggle widget */
ft_void_t GPU_APP_CoPro_Widget_Toggle()
{
    /*************************************************************************/
    /* Below code demonstrates the usage of toggle function. Toggle function */
    /* draws line with inside knob to choose between left and right. Toggle  */
    /* inside color can be changed by bgcolor and knob color by fgcolor. Left*/
    /* right texts can be written and the color of the text can be changed by*/
    /* colorrgb graphics function                                            */
    /*************************************************************************/
    {
        ft_int16_t xOffset,yOffset,yDist = 40;

    GPU_CoCmd_Dlstart(pScreen);
    GPU_API_Write_CoCmd(CLEAR_COLOR_RGB(64,64,64));
    GPU_API_Write_CoCmd(CLEAR(1,1,1));
    GPU_API_Write_CoCmd(COLOR_RGB(0xff,0xff,0xff));
    /* Draw toggle with flat effect */
    GPU_CoCmd_FgColor(pScreen,0xffff00);
    GPU_CoCmd_BgColor(pScreen, 0x000080);

    GPU_API_Write_CoCmd(COLOR_RGB(0xff,0xff,0xff));

    GPU_CoCmd_Toggle(pScreen, 40, 10, 30, 27, OPT_FLAT, 0.5*65535,"no""\xff""yes");//circle radius will be extended on both the horizontal sides
    GPU_API_Write_CoCmd(COLOR_A(255));
    GPU_CoCmd_Text(pScreen,40, 40, 26, 0, "Flat effect");//text info
    /* Draw toggle bar with 3d effect */
    GPU_CoCmd_FgColor(pScreen,0x00ff00);
    GPU_CoCmd_BgColor(pScreen, 0x800000);
    GPU_CoCmd_Toggle(pScreen, 140, 10, 30, 27, 0, 1*65535,"stop""\xff""run");
    GPU_API_Write_CoCmd(COLOR_A(255));
    GPU_CoCmd_Text(pScreen,140, 40, 26, 0, "3D effect");//text info

    xOffset = 40;
    yOffset = 80;
    /* Draw horizontal toggle bars */
    GPU_CoCmd_BgColor(pScreen, 0x800000);
    GPU_CoCmd_FgColor(pScreen,0x410105);
    GPU_CoCmd_Toggle(pScreen, xOffset, yOffset, 30, 27, 0, 0*65535,"-ve""\xff""+ve");
    GPU_CoCmd_FgColor(pScreen,0x0b0721);
    GPU_CoCmd_BgColor(pScreen, 0x000080);
    yOffset += yDist;
    GPU_CoCmd_Toggle(pScreen, xOffset, yOffset, 30, 27, 0, 0.25*65535,"zero""\xff""one");
    GPU_CoCmd_BgColor(pScreen, 0xffff00);
    GPU_CoCmd_FgColor(pScreen,0x575e1b);
    GPU_API_Write_CoCmd(COLOR_RGB(0,0,0));
    yOffset += yDist;
    GPU_CoCmd_Toggle(pScreen, xOffset, yOffset, 30, 27, 0, 0.5*65535,"exit""\xff""init");
    GPU_CoCmd_BgColor(pScreen, 0x808080);
    GPU_CoCmd_FgColor(pScreen,0x333234);
    GPU_API_Write_CoCmd(COLOR_RGB(0xff,0xff,0xff));
    yOffset += yDist;
    GPU_CoCmd_Toggle(pScreen, xOffset, yOffset, 30, 27, 0, 0.75*65535,"off""\xff""on");

    GPU_API_Write_CoCmd(DISPLAY());
    GPU_CoCmd_Swap(pScreen);

    /* Wait till coprocessor completes the operation */
    GPU_HAL_WaitCmdfifo_empty(pScreen);
    GPU_APP_ENABLE_DELAY();
    }
}

/* API to demonstrate spinner widget */
ft_void_t GPU_APP_CoPro_Widget_Spinner()
{
    /*************************************************************************/
    /* Below code demonstrates the usage of spinner function. Spinner func   */
    /* will wait untill stop command is sent from the mcu. Spinner has option*/
    /* for displaying points in circle fashion or in a line fashion.         */
    /*************************************************************************/

    GPU_CoCmd_Dlstart(pScreen);
    GPU_API_Write_CoCmd(CLEAR_COLOR_RGB(64,64,64));
    GPU_API_Write_CoCmd(CLEAR(1,1,1));
    GPU_API_Write_CoCmd(COLOR_RGB(0xff,0xff,0xff));
    GPU_CoCmd_Text(pScreen,(DispWidth/2), 20, 27, OPT_CENTER, "Spinner circle");
    GPU_CoCmd_Text(pScreen,(DispWidth/2), 80, 27, OPT_CENTER, "Please Wait ...");
    GPU_CoCmd_Spinner(pScreen, (DispWidth/2),(DispHeight/2),0,1);//style 0 and scale 0

    /* Wait till coprocessor completes the operation */
    GPU_HAL_WaitCmdfifo_empty(pScreen);
    GPU_APP_ENABLE_DELAY();

    /**************************** spinner with style 1 and scale 1 *****************************************************/


    GPU_CoCmd_Dlstart(pScreen);
    GPU_API_Write_CoCmd(CLEAR_COLOR_RGB(64,64,64));
    GPU_API_Write_CoCmd(CLEAR(1,1,1));
    GPU_API_Write_CoCmd(COLOR_RGB(0xff,0xff,0xff));
    GPU_CoCmd_Text(pScreen,(DispWidth/2), 20, 27, OPT_CENTER, "Spinner line");
    GPU_CoCmd_Text(pScreen,(DispWidth/2), 80, 27, OPT_CENTER, "Please Wait ...");
    GPU_API_Write_CoCmd(COLOR_RGB(0x00,0x00,0x80));
    GPU_CoCmd_Spinner(pScreen, (DispWidth/2),(DispHeight/2),1,1);//style 1 and scale 1

    /* Wait till coprocessor completes the operation */
    GPU_HAL_WaitCmdfifo_empty(pScreen);
    GPU_APP_ENABLE_DELAY();

    GPU_CoCmd_Dlstart(pScreen);
    GPU_API_Write_CoCmd(CLEAR_COLOR_RGB(64,64,64));
    GPU_API_Write_CoCmd(CLEAR(1,1,1));
    GPU_API_Write_CoCmd(COLOR_RGB(0xff,0xff,0xff));
    GPU_CoCmd_Text(pScreen,(DispWidth/2), 20, 27, OPT_CENTER, "Spinner clockhand");
    GPU_CoCmd_Text(pScreen,(DispWidth/2), 80, 27, OPT_CENTER, "Please Wait ...");
    GPU_API_Write_CoCmd(COLOR_RGB(0x80,0x00,0x00));
    GPU_CoCmd_Spinner(pScreen, (DispWidth/2),((DispHeight/2) + 20),2,1);//style 2 scale 1

    /* Wait till coprocessor completes the operation */
    GPU_HAL_WaitCmdfifo_empty(pScreen);
    GPU_APP_ENABLE_DELAY();

    GPU_CoCmd_Dlstart(pScreen);
    GPU_API_Write_CoCmd(CLEAR_COLOR_RGB(64,64,64));
    GPU_API_Write_CoCmd(CLEAR(1,1,1));
    GPU_API_Write_CoCmd(COLOR_RGB(0xff,0xff,0xff));
    GPU_CoCmd_Text(pScreen,(DispWidth/2), 20, 27, OPT_CENTER, "Spinner dual dots");
    GPU_CoCmd_Text(pScreen,(DispWidth/2), 80, 27, OPT_CENTER, "Please Wait ...");
    GPU_API_Write_CoCmd(COLOR_RGB(0x80,0x00,0x00));
    GPU_CoCmd_Spinner(pScreen, (DispWidth/2),((DispHeight/2) + 20),3,1);//style 3 scale 0

    /* Wait till coprocessor completes the operation */
    GPU_HAL_WaitCmdfifo_empty(pScreen);
    GPU_APP_ENABLE_DELAY();

    /* Send the stop command */
    GPU_HAL_WrCmd32(pScreen,  CMD_STOP);
    /* Update the command buffer pointers - both read and write pointers */
    GPU_HAL_WaitCmdfifo_empty(pScreen);
    GPU_APP_ENABLE_DELAY();
}
#endif

#ifdef GPU_APP_ENABLE_APIS_SET4
/* API to demonstrate screen saver widget - playing of bitmap via macro0 */
ft_void_t GPU_APP_CoPro_Screensaver()
{
    /*************************************************************************/
    /* Below code demonstrates the usage of screensaver function. Screen     */
    /* saver modifies macro0 with the vertex2f command.                      */
    /* MCU can display any static display list at the background with the    */
    /* changing bitmap                                                       */
    /*************************************************************************/

    /* Download the bitmap data */
    GPU_HAL_WrMem(pScreen, RAM_G, &(Lena_Bitmap_RawData[Lena_Bitmap_RawData_Header[0].ArrayOffset]), \
    Lena_Bitmap_RawData_Header[0].Stride*Lena_Bitmap_RawData_Header[0].Height);

    /* Send command screen saver */

    GPU_API_Write_CoCmd( CMD_SCREENSAVER);//screen saver command will continuously update the macro0 with vertex2f command
    /* Copy the display list */
    GPU_CoCmd_Dlstart(pScreen);
    GPU_API_Write_CoCmd(CLEAR_COLOR_RGB(0,0,0x80));
    GPU_API_Write_CoCmd(CLEAR(1,1,1));
    GPU_API_Write_CoCmd(COLOR_RGB(0xff,0xff,0xff));
    /* lena bitmap */
    GPU_CoCmd_LoadIdentity(pScreen);
    GPU_CoCmd_Scale(pScreen, 3*65536,3*65536);//scale the bitmap 3 times on both sides
    GPU_CoCmd_SetMatrix(pScreen );
    GPU_API_Write_CoCmd(BEGIN(BITMAPS));
    GPU_API_Write_CoCmd(BITMAP_SOURCE(0));
    GPU_API_Write_CoCmd(BITMAP_LAYOUT(Lena_Bitmap_RawData_Header[0].Format,
        Lena_Bitmap_RawData_Header[0].Stride,Lena_Bitmap_RawData_Header[0].Height));
    GPU_API_Write_CoCmd(BITMAP_SIZE(BILINEAR,BORDER,BORDER,
        Lena_Bitmap_RawData_Header[0].Width*3,Lena_Bitmap_RawData_Header[0].Height*3));
    GPU_API_Write_CoCmd(MACRO(0));
    GPU_API_Write_CoCmd(END());
    /* Display the text */
    GPU_CoCmd_LoadIdentity(pScreen);
    GPU_CoCmd_SetMatrix(pScreen );
    GPU_CoCmd_Text(pScreen,(DispWidth/2), (DispHeight/2), 27, OPT_CENTER, "Screen Saver ...");
    GPU_CoCmd_MemSet(pScreen, (RAM_G + 3200), 0xff, (160L*2*120));
    GPU_API_Write_CoCmd(DISPLAY());
    GPU_CoCmd_Swap(pScreen);

    /* Wait till co-processor completes the operation */
    GPU_HAL_WaitCmdfifo_empty(pScreen);
    GPU_APP_ENABLE_DELAY_MS(10000);

    /* Send the stop command */
    GPU_HAL_WrCmd32(pScreen,  CMD_STOP);
    /* Update the command buffer pointers - both read and write pointers */
    GPU_HAL_WaitCmdfifo_empty(pScreen);

}
/* Sample app to demonstrate snapshot widget/functionality */
ft_void_t GPU_APP_CoPro_Snapshot()
{
    /*************************************************************************/
    /* Below code demonstrates the usage of snapshot function. Snapshot      */
    /* captures the present screen and dumps into bitmap with colour formats */
    /* argb4.                                                                */
    /*************************************************************************/

    ft_uint16_t WriteByte = 0;

    /* fadeout before switching off the pclock */
    GPU_API_fadeout();

    /* Switch off the lcd */
    GPU_HAL_Wr8(pScreen, REG_GPIO, 0x7f);
    vTaskDelay( 100 / portTICK_PERIOD_MS );//sleep for 100 ms

    /* Disable the pclock */
    GPU_HAL_Wr8(pScreen, REG_PCLK,WriteByte);
    /* Configure the resolution to 160x120 dimension */
    WriteByte = 160;
    GPU_HAL_Wr16(pScreen, REG_HSIZE,WriteByte);
    WriteByte = 120;
    GPU_HAL_Wr16(pScreen, REG_VSIZE,WriteByte);

    /* Construct screen shot for snapshot */

    GPU_CoCmd_Dlstart(pScreen);
    GPU_API_Write_CoCmd(CLEAR_COLOR_RGB(0,0,0));
    GPU_API_Write_CoCmd(CLEAR(1,1,1));
    GPU_API_Write_CoCmd(COLOR_RGB(28,20,99));
    /* captured snapshot */
    GPU_API_Write_CoCmd(BEGIN(POINTS));
    GPU_API_Write_CoCmd(COLOR_A(128));
    GPU_API_Write_CoCmd(POINT_SIZE(20*16));
    GPU_API_Write_CoCmd(VERTEX2F(0*16,0*16));
    GPU_API_Write_CoCmd(POINT_SIZE(25*16));
    GPU_API_Write_CoCmd(VERTEX2F(20*16,10*16));
    GPU_API_Write_CoCmd(POINT_SIZE(30*16));
    GPU_API_Write_CoCmd(VERTEX2F(40*16,20*16));
    GPU_API_Write_CoCmd(POINT_SIZE(35*16));
    GPU_API_Write_CoCmd(VERTEX2F(60*16,30*16));
    GPU_API_Write_CoCmd(POINT_SIZE(40*16));
    GPU_API_Write_CoCmd(VERTEX2F(80*16,40*16));
    GPU_API_Write_CoCmd(POINT_SIZE(45*16));
    GPU_API_Write_CoCmd(VERTEX2F(100*16,50*16));
    GPU_API_Write_CoCmd(POINT_SIZE(50*16));
    GPU_API_Write_CoCmd(VERTEX2F(120*16,60*16));
    GPU_API_Write_CoCmd(POINT_SIZE(55*16));
    GPU_API_Write_CoCmd(VERTEX2F(140*16,70*16));
    GPU_API_Write_CoCmd(POINT_SIZE(60*16));
    GPU_API_Write_CoCmd(VERTEX2F(160*16,80*16));
    GPU_API_Write_CoCmd(POINT_SIZE(65*16));
    GPU_API_Write_CoCmd(VERTEX2F(0*16,120*16));
    GPU_API_Write_CoCmd(POINT_SIZE(70*16));
    GPU_API_Write_CoCmd(VERTEX2F(160*16,0*16));
    GPU_API_Write_CoCmd(END());//display the bitmap at the center of the display
    GPU_API_Write_CoCmd(COLOR_A(255));
    GPU_API_Write_CoCmd(COLOR_RGB(32,32,32));
    GPU_CoCmd_Text(pScreen,80, 60, 26, OPT_CENTER, "Points");

    GPU_API_Write_CoCmd(DISPLAY());
    GPU_CoCmd_Swap(pScreen);

    /* Wait till co-processor completes the operation */
    GPU_HAL_WaitCmdfifo_empty(pScreen);

    vTaskDelay( 100 / portTICK_PERIOD_MS ); //timeout for snapshot to be performed by co-processor

    /* Take snap shot of the current screen */
    GPU_HAL_WrCmd32(pScreen, CMD_SNAPSHOT);
    GPU_HAL_WrCmd32(pScreen, 3200);//store the rgb content at location 3200

    //timeout for snapshot to be performed by co-processor

    /* Wait till coprocessor completes the operation */
    GPU_HAL_WaitCmdfifo_empty(pScreen);

    vTaskDelay( 100 / portTICK_PERIOD_MS ); //timeout for snapshot to be performed by co-processor

    /* reconfigure the resolution wrt configuration */
    WriteByte = DispWidth;
    GPU_HAL_Wr16(pScreen, REG_HSIZE,WriteByte);
    WriteByte = DispHeight;
    GPU_HAL_Wr16(pScreen, REG_VSIZE,WriteByte);

    GPU_CoCmd_Dlstart(pScreen);
    GPU_API_Write_CoCmd(CLEAR_COLOR_RGB(0xff,0xff,0xff));
    GPU_API_Write_CoCmd(CLEAR(1,1,1));
    GPU_API_Write_CoCmd(COLOR_RGB(0xff,0xff,0xff));
    /* captured snapshot */
    GPU_API_Write_CoCmd(BEGIN(BITMAPS));
    GPU_API_Write_CoCmd(BITMAP_SOURCE(3200));
    GPU_API_Write_CoCmd(BITMAP_LAYOUT(ARGB4,160*2,120));
    GPU_API_Write_CoCmd(BITMAP_SIZE(BILINEAR,BORDER,BORDER,160,120));
    GPU_API_Write_CoCmd(VERTEX2F(((DispWidth - 160)/2)*16,((DispHeight - 120)/2)*16));
    GPU_API_Write_CoCmd(END());//display the bitmap at the center of the display
    /* Display the text info */
    GPU_API_Write_CoCmd(COLOR_RGB(32,32,32));
    GPU_CoCmd_Text(pScreen,(DispWidth/2), 40, 27, OPT_CENTER, "Snap shot");

    GPU_API_Write_CoCmd(DISPLAY());
    GPU_CoCmd_Swap(pScreen);

    /* Wait till coprocessor completes the operation */
    GPU_HAL_WaitCmdfifo_empty(pScreen);

    /* reenable the pclock */
    WriteByte = DispPCLK;
    GPU_HAL_Wr8(pScreen, REG_PCLK,WriteByte);
    vTaskDelay( 60 / portTICK_PERIOD_MS ); //sleep for 60 ms

    /* Power on the LCD */
    GPU_HAL_Wr8(pScreen, REG_GPIO, 0xff);
    vTaskDelay( 200 / portTICK_PERIOD_MS ); // give some time for the lcd to switch on - hack for one particular panel

    /* set the display pwm back to 128 */
    GPU_API_fadein();

    GPU_APP_ENABLE_DELAY();
}
/* API to demonstrate sketch widget */
ft_void_t GPU_APP_CoPro_Sketch()
{
    /*************************************************************************/
    /* Below code demonstrates the usage of sketch function. Sketch function */
    /* draws line for pen movement. Skecth supports L1 and L8 output formats */
    /* Sketch draws till stop command is executed.                           */
    /*************************************************************************/

    ft_int16_t BorderSz = 40;
    ft_uint32_t MemZeroSz;
    /* Send command sketch */

    MemZeroSz = 1L*(DispWidth - 2*BorderSz)*(DispHeight - 2*BorderSz);
    GPU_CoCmd_MemZero(pScreen, RAM_G,MemZeroSz);
    GPU_CoCmd_Sketch(pScreen, BorderSz,BorderSz,(DispWidth - 2*BorderSz),(DispHeight - 2*BorderSz),0,L1);//sketch in L1 format
    /* Display the sketch */
    GPU_CoCmd_Dlstart(pScreen);
    GPU_API_Write_CoCmd(CLEAR_COLOR_RGB(0x80,0,0x00));
    GPU_API_Write_CoCmd(CLEAR(1,1,1));
    GPU_API_Write_CoCmd(COLOR_RGB(0xff,0xff,0xff));
    GPU_API_Write_CoCmd(SCISSOR_SIZE((DispWidth - 2*BorderSz),(DispHeight - 2*BorderSz)));
    GPU_API_Write_CoCmd(SCISSOR_XY(BorderSz,BorderSz));
    GPU_API_Write_CoCmd(CLEAR_COLOR_RGB(0xff,0xff,0xff));
    GPU_API_Write_CoCmd(CLEAR(1,1,1));

    GPU_API_Write_CoCmd(SCISSOR_SIZE(512,512));
    GPU_API_Write_CoCmd(SCISSOR_XY(0,0));
    GPU_API_Write_CoCmd(COLOR_RGB(0,0,0));
    /* L1 bitmap display */
    GPU_API_Write_CoCmd(BEGIN(BITMAPS));
    GPU_API_Write_CoCmd(BITMAP_SOURCE(0));
    GPU_API_Write_CoCmd(BITMAP_LAYOUT(L1,(DispWidth - 2*BorderSz)/8,(DispHeight - 2*BorderSz)));
    GPU_API_Write_CoCmd(BITMAP_SIZE(BILINEAR,BORDER,BORDER,(DispWidth - 2*BorderSz),(DispHeight - 2*BorderSz)));
    GPU_API_Write_CoCmd(VERTEX2F(BorderSz*16,BorderSz*16));
    GPU_API_Write_CoCmd(END());
    /* Display the text */
    GPU_API_Write_CoCmd(COLOR_RGB(0xff,0xff,0xff));
    GPU_CoCmd_Text(pScreen,(DispWidth/2), 20, 27, OPT_CENTER, "Sketch L1");

    GPU_API_Write_CoCmd(DISPLAY());
    GPU_CoCmd_Swap(pScreen);

    /* Wait till coprocessor completes the operation */
    GPU_HAL_WaitCmdfifo_empty(pScreen);

    GPU_APP_ENABLE_DELAY_MS(5000);

    /* Send the stop command */
    GPU_HAL_WrCmd32(pScreen,  CMD_STOP);
    /* Update the command buffer pointers - both read and write pointers */
    GPU_HAL_WaitCmdfifo_empty(pScreen);

    /* Sketch L8 format */

    /* Send command sketch */

    GPU_CoCmd_MemZero(pScreen, RAM_G,MemZeroSz);
    GPU_CoCmd_Sketch(pScreen, BorderSz,BorderSz,(DispWidth - 2*BorderSz),(DispHeight - 2*BorderSz),0,L8);//sketch in L8 format
    /* Display the sketch */
    GPU_CoCmd_Dlstart(pScreen);
    GPU_API_Write_CoCmd(CLEAR_COLOR_RGB(0x00,0,0x80));
    GPU_API_Write_CoCmd(CLEAR(1,1,1));
    GPU_API_Write_CoCmd(COLOR_RGB(0xff,0xff,0xff));
    GPU_API_Write_CoCmd(SCISSOR_SIZE((DispWidth - 2*BorderSz),(DispHeight - 2*BorderSz)));
    GPU_API_Write_CoCmd(SCISSOR_XY(BorderSz,BorderSz));
    GPU_API_Write_CoCmd(CLEAR_COLOR_RGB(0xff,0xff,0xff));
    GPU_API_Write_CoCmd(CLEAR(1,1,1));

    GPU_API_Write_CoCmd(SCISSOR_SIZE(512,512));
    GPU_API_Write_CoCmd(SCISSOR_XY(0,0));
    GPU_API_Write_CoCmd(COLOR_RGB(0,0,0));
    /* L8 bitmap display */
    GPU_API_Write_CoCmd(BEGIN(BITMAPS));
    GPU_API_Write_CoCmd(BITMAP_SOURCE(0));
    GPU_API_Write_CoCmd(BITMAP_LAYOUT(L8,(DispWidth - 2*BorderSz),(DispHeight - 2*BorderSz)));
    GPU_API_Write_CoCmd(BITMAP_SIZE(BILINEAR,BORDER,BORDER,(DispWidth - 2*BorderSz),(DispHeight - 2*BorderSz)));
    GPU_API_Write_CoCmd(VERTEX2F(BorderSz*16,BorderSz*16));
    GPU_API_Write_CoCmd(END());
    /* Display the text */
    GPU_API_Write_CoCmd(COLOR_RGB(0xff,0xff,0xff));
    GPU_CoCmd_Text(pScreen,(DispWidth/2), 20, 27, OPT_CENTER, "Sketch L8");

    GPU_API_Write_CoCmd(DISPLAY());
    GPU_CoCmd_Swap(pScreen);

    /* Wait till coprocessor completes the operation */
    GPU_HAL_WaitCmdfifo_empty(pScreen);
    GPU_APP_ENABLE_DELAY_MS(5000);

    /* Send the stop command */
    GPU_HAL_WrCmd32(pScreen,  CMD_STOP);
    /* Update the command buffer pointers - both read and write pointers */
    GPU_HAL_WaitCmdfifo_empty(pScreen);
}

/* API to demonstrate scale, rotate and translate functionality */
ft_void_t GPU_APP_CoPro_Matrix()
{
    /*************************************************************************/
    /* Below code demonstrates the usage of bitmap matrix processing apis.   */
    /* Mainly matrix apis consists if scale, rotate and translate.           */
    /* Units of translation and scale are interms of 1/65536, rotation is in */
    /* degrees and in terms of 1/65536. +ve theta is anticlock wise, and -ve  */
    /* theta is clock wise rotation                                          */
    /*************************************************************************/

    /* Lena image with 40x40 rgb565 is used as an example */

    ft_int32_t imagewidth,imagestride,imageheight,imagexoffset,imageyoffset;

    /* Download the bitmap data */
    GPU_HAL_WrMem(pScreen, RAM_G,(ft_uint8_t *)&Lena_Bitmap_RawData[Lena_Bitmap_RawData_Header[0].ArrayOffset],
        Lena_Bitmap_RawData_Header[0].Stride*Lena_Bitmap_RawData_Header[0].Height);


    GPU_CoCmd_Dlstart(pScreen);
    GPU_API_Write_CoCmd(CLEAR_COLOR_RGB(0xff,0xff,0xff));
    GPU_API_Write_CoCmd(CLEAR(1,1,1));
    GPU_API_Write_CoCmd(COLOR_RGB(32,32,32));
    GPU_CoCmd_Text(pScreen,10, 5, 16, 0, "BM with rotation");
    GPU_CoCmd_Text(pScreen,10, 20 + 40 + 5, 16, 0, "BM with scaling");
    GPU_CoCmd_Text(pScreen,10, 20 + 40 + 20 + 80 + 5, 16, 0,"BM with flip");

    imagewidth = Lena_Bitmap_RawData_Header[0].Width;
    imageheight = Lena_Bitmap_RawData_Header[0].Height;
    imagestride = Lena_Bitmap_RawData_Header[0].Stride;
    imagexoffset = 10*16;
    imageyoffset = 20*16;

    GPU_API_Write_CoCmd(COLOR_RGB(0xff,0xff,0xff));
    GPU_API_Write_CoCmd(BEGIN(BITMAPS));
    GPU_API_Write_CoCmd(BITMAP_SOURCE(0));
    GPU_API_Write_CoCmd(BITMAP_LAYOUT(Lena_Bitmap_RawData_Header[0].Format,imagestride,imageheight));
    GPU_API_Write_CoCmd(BITMAP_SIZE(BILINEAR,BORDER,BORDER,imagewidth,imageheight));
    /******************************************* Perform display of plain bitmap ************************************/
    GPU_API_Write_CoCmd(VERTEX2F(imagexoffset,imageyoffset));

    /* Perform display of plain bitmap with 45 degrees anti clock wise and the rotation is performed on top left coordinate */
    imagexoffset += (imagewidth + 10)*16;
    GPU_CoCmd_LoadIdentity(pScreen);
    GPU_CoCmd_Rotate(pScreen, (-45*65536/360));//rotate by 45 degrees anticlock wise
    GPU_CoCmd_SetMatrix(pScreen );
    GPU_API_Write_CoCmd(VERTEX2F(imagexoffset,imageyoffset));

    /* Perform display of plain bitmap with 30 degrees clock wise and the rotation is performed on top left coordinate */
    imagexoffset += (imagewidth*1.42 + 10)*16;//add the width*1.41 as diagonal is new width and extra 10 pixels
    GPU_CoCmd_LoadIdentity(pScreen);
    GPU_CoCmd_Rotate(pScreen, 30*65536/360);//rotate by 33 degrees clock wise
    GPU_CoCmd_SetMatrix(pScreen );
    GPU_API_Write_CoCmd(VERTEX2F(imagexoffset,imageyoffset));

    /* Perform display of plain bitmap with 45 degrees anti clock wise and the rotation is performed wrt centre of the bitmap */
    imagexoffset += (imagewidth*1.42 + 10)*16;//add the width*1.41 as diagonal is new width and extra 10 pixels
    GPU_CoCmd_LoadIdentity(pScreen);
    GPU_CoCmd_Translate(pScreen, 65536*imagewidth/2,65536*imageheight/2);//make the rotation coordinates at the center
    GPU_CoCmd_Rotate(pScreen, -45*65536/360);//rotate by 45 degrees anticlock wise
    GPU_CoCmd_Translate(pScreen, -65536*imagewidth/2,-65536*imageheight/2);
    GPU_CoCmd_SetMatrix(pScreen );
    GPU_API_Write_CoCmd(VERTEX2F(imagexoffset,imageyoffset));

    /* Perform display of plain bitmap with 45 degrees clock wise and the rotation is performed so that whole bitmap is viewable */
    imagexoffset += (imagewidth*1.42 + 10)*16;//add the width*1.41 as diagonal is new width and extra 10 pixels
    GPU_CoCmd_LoadIdentity(pScreen);
    GPU_CoCmd_Translate(pScreen, 65536*imagewidth/2,65536*imageheight/2);//make the rotation coordinates at the center
    GPU_CoCmd_Rotate(pScreen, 45*65536/360);//rotate by 45 degrees clock wise
    GPU_CoCmd_Translate(pScreen, -65536*imagewidth/10,-65536*imageheight/2);
    GPU_CoCmd_SetMatrix(pScreen );
    GPU_API_Write_CoCmd(BITMAP_SIZE(BILINEAR,BORDER,BORDER,imagewidth*2,imageheight*2));
    GPU_API_Write_CoCmd(VERTEX2F(imagexoffset,imageyoffset));

    /* Perform display of plain bitmap with 90 degrees anti clock wise and the rotation is performed so that whole bitmap is viewable */
    imagexoffset += (imagewidth*2 + 10)*16;
    GPU_CoCmd_LoadIdentity(pScreen);
    GPU_CoCmd_Translate(pScreen, 65536*imagewidth/2,65536*imageheight/2);//make the rotation coordinates at the center
    GPU_CoCmd_Rotate(pScreen, -90*65536/360);//rotate by 90 degrees anticlock wise
    GPU_CoCmd_Translate(pScreen, -65536*imagewidth/2,-65536*imageheight/2);
    GPU_CoCmd_SetMatrix(pScreen );
    GPU_API_Write_CoCmd(BITMAP_SIZE(BILINEAR,BORDER,BORDER,imagewidth,imageheight));
    GPU_API_Write_CoCmd(VERTEX2F(imagexoffset,imageyoffset));

    /* Perform display of plain bitmap with 180 degrees clock wise and the rotation is performed so that whole bitmap is viewable */
    imagexoffset += (imagewidth + 10)*16;
    GPU_CoCmd_LoadIdentity(pScreen);
    GPU_CoCmd_Translate(pScreen, 65536*imagewidth/2,65536*imageheight/2);//make the rotation coordinates at the center
    GPU_CoCmd_Rotate(pScreen, -180*65536/360);//rotate by 180 degrees anticlock wise
    GPU_CoCmd_Translate(pScreen, -65536*imagewidth/2,-65536*imageheight/2);
    GPU_CoCmd_SetMatrix(pScreen );
    GPU_API_Write_CoCmd(VERTEX2F(imagexoffset,imageyoffset));
    /******************************************* Perform display of bitmap with scale ************************************/
    /* Perform display of plain bitmap with scale factor of 2x2 in x & y direction */
    imagexoffset = (10)*16;
    imageyoffset += (imageheight + 20)*16;
    GPU_CoCmd_LoadIdentity(pScreen);
    GPU_CoCmd_Scale(pScreen, 2*65536,2*65536);//scale by 2x2
    GPU_CoCmd_SetMatrix(pScreen );
    GPU_API_Write_CoCmd(BITMAP_SIZE(BILINEAR,BORDER,BORDER,imagewidth*2,imageheight*2));
    GPU_API_Write_CoCmd(VERTEX2F(imagexoffset,imageyoffset));

    /* Perform display of plain bitmap with scale factor of .5x.25 in x & y direction, rotate by 45 degrees clock wise wrt top left */
    imagexoffset += (imagewidth*2 + 10)*16;
    GPU_CoCmd_LoadIdentity(pScreen);
    GPU_CoCmd_Translate(pScreen, 65536*imagewidth/2,65536*imageheight/2);//make the rotation coordinates at the center

    GPU_CoCmd_Rotate(pScreen, 45*65536/360);//rotate by 45 degrees clock wise
    GPU_CoCmd_Scale(pScreen, 65536/2,65536/4);//scale by .5x.25
    GPU_CoCmd_Translate(pScreen, -65536*imagewidth/2,-65536*imageheight/2);
    GPU_CoCmd_SetMatrix(pScreen );
    GPU_API_Write_CoCmd(VERTEX2F(imagexoffset,imageyoffset));

    /* Perform display of plain bitmap with scale factor of .5x2 in x & y direction, rotate by 75 degrees anticlock wise wrt center of the image */
    imagexoffset += (imagewidth + 10)*16;
    GPU_CoCmd_LoadIdentity(pScreen);
    GPU_CoCmd_Translate(pScreen, 65536*imagewidth/2,65536*imageheight/2);//make the rotation coordinates at the center
    GPU_CoCmd_Rotate(pScreen, -75*65536/360);//rotate by 75 degrees anticlock wise
    GPU_CoCmd_Scale(pScreen, 65536/2,2*65536);//scale by .5x2
    GPU_CoCmd_Translate(pScreen, -65536*imagewidth/2,-65536*imageheight/8);
    GPU_CoCmd_SetMatrix(pScreen );
    GPU_API_Write_CoCmd(BITMAP_SIZE(BILINEAR,BORDER,BORDER,imagewidth*5/2,imageheight*5/2));
    GPU_API_Write_CoCmd(VERTEX2F(imagexoffset,imageyoffset));
    /******************************************* Perform display of bitmap flip ************************************/
    /* perform display of plain bitmap with 1x1 and flip right */
    imagexoffset = (10)*16;
    imageyoffset += (imageheight*2 + 20)*16;
    GPU_CoCmd_LoadIdentity(pScreen);
    GPU_CoCmd_Translate(pScreen, 65536*imagewidth/2,65536*imageheight/2);//make the rotation coordinates at the center
    GPU_CoCmd_Scale(pScreen, -1*65536,1*65536);//flip right
    GPU_CoCmd_Translate(pScreen, -65536*imagewidth/2,-65536*imageheight/2);
    GPU_CoCmd_SetMatrix(pScreen );
    GPU_API_Write_CoCmd(BITMAP_SIZE(BILINEAR,BORDER,BORDER,imagewidth,imageheight));
    GPU_API_Write_CoCmd(VERTEX2F(imagexoffset,imageyoffset));

    /* Perform display of plain bitmap with 2x2 scaling, flip bottom */
    imagexoffset += (imagewidth + 10)*16;
    GPU_CoCmd_LoadIdentity(pScreen);
    GPU_CoCmd_Translate(pScreen, 65536*imagewidth/2,65536*imageheight/2);//make the rotation coordinates at the center
    GPU_CoCmd_Scale(pScreen, 2*65536,-2*65536);//flip bottom and scale by 2 on both sides
    GPU_CoCmd_Translate(pScreen, -65536*imagewidth/4,-65536*imageheight/1.42);
    GPU_CoCmd_SetMatrix(pScreen );
    GPU_API_Write_CoCmd(BITMAP_SIZE(BILINEAR,BORDER,BORDER,imagewidth*4,imageheight*4));
    GPU_API_Write_CoCmd(VERTEX2F(imagexoffset,imageyoffset));

    /* Perform display of plain bitmap with 2x1 scaling, rotation and flip right and make sure whole image is viewable */
    imagexoffset += (imagewidth*2 + 10)*16;
    GPU_CoCmd_LoadIdentity(pScreen);
    GPU_CoCmd_Translate(pScreen, 65536*imagewidth/2,65536*imageheight/2);//make the rotation coordinates at the center

    GPU_CoCmd_Rotate(pScreen, -45*65536/360);//rotate by 45 degrees anticlock wise
    GPU_CoCmd_Scale(pScreen, -2*65536,1*65536);//flip right and scale by 2 on x axis
    GPU_CoCmd_Translate(pScreen, -65536*imagewidth/2,-65536*imageheight/8);
    GPU_CoCmd_SetMatrix(pScreen );
    GPU_API_Write_CoCmd(BITMAP_SIZE(BILINEAR,BORDER,BORDER,(imagewidth*5),(imageheight*5)));
    GPU_API_Write_CoCmd(VERTEX2F(imagexoffset,imageyoffset));

    GPU_API_Write_CoCmd(END());
    GPU_API_Write_CoCmd(DISPLAY());
    GPU_CoCmd_Swap(pScreen);

    /* Wait till coprocessor completes the operation */
    GPU_HAL_WaitCmdfifo_empty(pScreen);
    GPU_APP_ENABLE_DELAY_MS(5000);
}

/* Sample app api to demonstrate track widget functionality */
ft_void_t GPU_APP_CoPro_Track(ft_void_t)
{

    /*************************************************************************/
    /* Below code demonstrates the usage of track function. Track function   */
    /* tracks the pen touch on any specific object. Track function supports  */
    /* rotary and horizontal/vertical tracks. Rotary is given by rotation    */
    /* angle and horizontal/vertical track is offset position.               */
    /*************************************************************************/
    {
    ft_int32_t LoopFlag = 0;
    ft_uint32_t TrackRegisterVal = 0;
    ft_uint16_t angleval = 0,slideval = 0,scrollval = 0;

    /* Set the tracker for 3 objects */

    GPU_CoCmd_Track(pScreen, DispWidth/2, DispHeight/2, 1,1, 10);
    GPU_CoCmd_Track(pScreen, 40, (DispHeight - 40), (DispWidth - 80),8, 11);
    GPU_CoCmd_Track(pScreen, (DispWidth - 40), 40, 8,(DispHeight - 80), 12);

    /* Wait till coprocessor completes the operation */
    GPU_HAL_WaitCmdfifo_empty(pScreen);

    LoopFlag = 600;
    /* update the background colour continuously for the colour change in any of the trackers */
    while(LoopFlag--)
    {
        ft_uint8_t tagval = 0;
        TrackRegisterVal = GPU_HAL_Rd32(pScreen, REG_TRACKER);
        tagval = TrackRegisterVal & 0xff;
        if(0 != tagval)
        {
            if(10 == tagval)
            {
                angleval = TrackRegisterVal>>16;
            }
            else if(11 == tagval)
            {
                slideval = TrackRegisterVal>>16;
            }
            else if(12 == tagval)
            {
                scrollval = TrackRegisterVal>>16;
                if((scrollval + 65535/10) > (9*65535/10))
                {
                    scrollval = (8*65535/10);
                }
                else if(scrollval < (1*65535/10))
                {
                    scrollval = 0;
                }
                else
                {
                    scrollval -= (1*65535/10);
                }
            }
        }
        /* Display a rotary dial, horizontal slider and vertical scroll */

        GPU_API_Write_CoCmd( CMD_DLSTART);

        {
            ft_int32_t tmpval0,tmpval1,tmpval2;
            ft_uint8_t angval,sldval,scrlval;

            tmpval0 = (ft_int32_t)angleval*255/65536;
            tmpval1 = (ft_int32_t)slideval*255/65536;
            tmpval2 = (ft_int32_t)scrollval*255/65536;

            angval = tmpval0&0xff;
            sldval = tmpval1&0xff;
            scrlval = tmpval2&0xff;
            GPU_API_Write_CoCmd(CLEAR_COLOR_RGB(angval,sldval,scrlval));
        }
        GPU_API_Write_CoCmd(CLEAR(1,1,1));
        GPU_API_Write_CoCmd(COLOR_RGB(0xff,0xff,0xff));

        /* Draw dial with 3d effect */
        GPU_CoCmd_FgColor(pScreen,0x00ff00);
        GPU_CoCmd_BgColor(pScreen, 0x800000);
        GPU_API_Write_CoCmd(TAG(10));
        GPU_CoCmd_Dial(pScreen, (DispWidth/2), (DispHeight/2), (DispWidth/8), 0, angleval);

        /* Draw slider with 3d effect */
        GPU_CoCmd_FgColor(pScreen,0x00a000);
        GPU_CoCmd_BgColor(pScreen, 0x800000);
        GPU_API_Write_CoCmd(TAG(11));
        GPU_CoCmd_Slider(pScreen, 40, (DispHeight - 40), (DispWidth - 80),8, 0, slideval, 65535);

        /* Draw scroll with 3d effect */
        GPU_CoCmd_FgColor(pScreen,0x00a000);
        GPU_CoCmd_BgColor(pScreen, 0x000080);
        GPU_API_Write_CoCmd(TAG(12));
        GPU_CoCmd_Scrollbar(pScreen, (DispWidth - 40), 40, 8, (DispHeight - 80), 0, scrollval, (65535*0.2), 65535);

        GPU_CoCmd_FgColor(pScreen,TAG_MASK(0));
        GPU_API_Write_CoCmd(COLOR_RGB(0xff,0xff,0xff));
        GPU_CoCmd_Text(pScreen,(DispWidth/2), ((DispHeight/2) + (DispWidth/8) + 8), 26, OPT_CENTER, "Rotary track");
        GPU_CoCmd_Text(pScreen,((DispWidth/2)), ((DispHeight - 40) + 8 + 8), 26, OPT_CENTER, "Horizontal track");
        GPU_CoCmd_Text(pScreen,(DispWidth - 40), 20, 26, OPT_CENTER, "Vertical track");

        GPU_API_Write_CoCmd(DISPLAY());
        GPU_CoCmd_Swap(pScreen);

        /* Wait till coprocessor completes the operation */
        GPU_HAL_WaitCmdfifo_empty(pScreen);

        vTaskDelay( 10 / portTICK_PERIOD_MS );//sleep for 10 ms
    }

    /* Set the tracker for 3 objects */

    GPU_CoCmd_Track(pScreen, 240, 136, 0,0, 10);
    GPU_CoCmd_Track(pScreen, 40, 232, 0,0, 11);
    GPU_CoCmd_Track(pScreen, 400, 40, 0,0, 12);

    /* Wait till coprocessor completes the operation */
    GPU_HAL_WaitCmdfifo_empty(pScreen);
    GPU_APP_ENABLE_DELAY_MS(5000);
    }
}
#endif

#ifdef GPU_APP_ENABLE_APIS_SET3
/* API to demonstrate custom font display */
ft_void_t GPU_APP_CoPro_Setfont()
{
    ft_uint8_t *pbuff;
    ft_uint8_t FontIdxTable[148];
    /*************************************************************************/
    /* Below code demonstrates the usage of setfont. Setfont function draws  */
    /* custom configured fonts on screen. Download the font table and raw    */
    /* font data in L1/L4/L8 format and display text                          */
    /*************************************************************************/

    /* Display custom font by reading from the binary file - header of 148 bytes is at the starting followed by font data of 96 characters */
    /*Roboto-BoldCondensed.ttf*/
    {
    ft_uint32_t fontaddr = (128+5*4);//header size
    ft_uint16_t blocklen;

    fontaddr = RAM_G;
    blocklen = 128+5*4;//header size

    pbuff = FontIdxTable;
    /* Copy data from starting of the array into buffer */
    //hal_memcpy((ft_uint8_t*)pbuff,(ft_uint8_t*)Roboto_BoldCondensed_12,1L*blocklen);
    memcpy((ft_uint8_t*)pbuff, (ft_uint8_t*)Roboto_BoldCondensed_12, 1L*blocklen);

    {
        ft_uint32_t *ptemp = (ft_uint32_t *)&pbuff[128+4*4],i;
        *ptemp = 1024;//download the font data at location 1024+32*8*25
        //memset(pbuff,16,32);
        for(i=0;i<32;i++)
        {
          pbuff[i] = 16;
        }
    }
    /* Modify the font data location */
    GPU_HAL_WrMem(pScreen,fontaddr,(ft_uint8_t *)pbuff,blocklen);

    /* Next download the data at location 32*8*25 - skip the first 32 characters */
    /* each character is 8x25 bytes */
    fontaddr += (1024+32*8*25);//make sure space is left at the starting of the buffer for first 32 characters - TBD manager this buffer so that this buffer can be utilized by other module

    GPU_HAL_WrMem(pScreen, fontaddr,&Roboto_BoldCondensed_12[blocklen],1L*GPU_APP_Roboto_BoldCondensed_12_SIZE);

    GPU_CoCmd_Dlstart(pScreen);
    GPU_API_Write_CoCmd(CLEAR_COLOR_RGB(0xff,0xff,0xff));//set the background to white
    GPU_API_Write_CoCmd(CLEAR(1,1,1));
    GPU_API_Write_CoCmd(COLOR_RGB(32,32,32));//black colour text

    GPU_CoCmd_Text(pScreen,(DispWidth/2), 20, 27, OPT_CENTER, "SetFont - format L4");
    GPU_API_Write_CoCmd(BITMAP_HANDLE(6));//give index table 6
    GPU_API_Write_CoCmd(BITMAP_SOURCE(1024));//make the address to 0
    GPU_API_Write_CoCmd(BITMAP_LAYOUT(L4,8,25));//stride is 8 and height is 25
    GPU_API_Write_CoCmd(BITMAP_SIZE(NEAREST,BORDER,BORDER,16,25));//width is 16 and height is 25

    GPU_CoCmd_SetFont(pScreen, 6,0);
    GPU_CoCmd_Text(pScreen,(DispWidth/2), 80,  6, OPT_CENTER, "The quick brown fox jumps");
    GPU_CoCmd_Text(pScreen,(DispWidth/2), 120, 6, OPT_CENTER, "over the lazy dog.");
    GPU_CoCmd_Text(pScreen,(DispWidth/2), 160, 6, OPT_CENTER, "1234567890");

    GPU_API_Write_CoCmd(DISPLAY());
    GPU_CoCmd_Swap(pScreen);

    /* Wait till coprocessor completes the operation */
    GPU_HAL_WaitCmdfifo_empty(pScreen);
    GPU_APP_ENABLE_DELAY_MS(4000);
    }
}
#endif


float lerp(float t, float a, float b)
{
    return (float)((1 - t) * a + t * b);
}
float smoothlerp(float t, float a, float b)
{

    float lt = 3 * t * t - 2 * t * t * t;

    return lerp(lt, a, b);
}

#ifdef GPU_APP_ENABLE_APIS_SET0
/* First draw points followed by lines to create 3d ball kind of effect */
ft_void_t GPU_APP_Ball_Stencil()
{
    ft_int16_t xball = (DispWidth/2),yball = 120,rball = (DispWidth/8);
    ft_int16_t numpoints = 6,numlines = 8,i,asize,aradius,gridsize = 20;
    ft_int32_t asmooth,loopflag = 1,dispr = (DispWidth - 10),displ = 10,dispa = 10,dispb = (DispHeight - 10),xflag = 1,yflag = 1;

    dispr -= ((dispr - displ)%gridsize);
    dispb -= ((dispb - dispa)%gridsize);
    /* write the play sound */
    GPU_HAL_Wr16(pScreen, REG_SOUND,0x50);
        loopflag = 100;
    while(loopflag-- >0 )
    {
        if(((xball + rball + 2) >= dispr) || ((xball - rball - 2) <= displ))
        {
            xflag ^= 1;
            GPU_HAL_Wr8(pScreen, REG_PLAY,1);
        }
        if(((yball + rball + 8) >= dispb) || ((yball - rball - 8) <= dispa))
        {
            yflag ^= 1;
            GPU_HAL_Wr8(pScreen, REG_PLAY,1);
        }
        if(xflag)
        {
            xball += 2;
        }
        else
        {
            xball -= 2;
        }
        if(yflag)
        {
            yball += 8 ;
        }
        else
        {
            yball -= 8;
        }


        GPU_API_Write_DLCmd(CLEAR_COLOR_RGB(128, 128, 0) );
        GPU_API_Write_DLCmd(CLEAR(1, 1, 1)); // clear screen
        GPU_API_Write_DLCmd(STENCIL_OP(INCR,INCR) );
        GPU_API_Write_DLCmd(COLOR_RGB(0, 0, 0) );
        /* draw grid */
        GPU_API_Write_DLCmd(LINE_WIDTH(16));
        GPU_API_Write_DLCmd(BEGIN(LINES));
        for(i=0;i<=((dispr - displ)/gridsize);i++)
        {
            GPU_API_Write_DLCmd(VERTEX2F((displ + i*gridsize)*16,dispa*16));
            GPU_API_Write_DLCmd(VERTEX2F((displ + i*gridsize)*16,dispb*16));
        }
        for(i=0;i<=((dispb - dispa)/gridsize);i++)
        {
            GPU_API_Write_DLCmd(VERTEX2F(displ*16,(dispa + i*gridsize)*16));
            GPU_API_Write_DLCmd(VERTEX2F(dispr*16,(dispa + i*gridsize)*16));
        }
        GPU_API_Write_DLCmd(END());
        GPU_API_Write_DLCmd(COLOR_MASK(0,0,0,0) );//mask all the colors
        GPU_API_Write_DLCmd(POINT_SIZE(rball*16) );
        GPU_API_Write_DLCmd(BEGIN(POINTS));
        GPU_API_Write_DLCmd(VERTEX2F(xball*16,yball*16));
        GPU_API_Write_DLCmd(STENCIL_OP(INCR,ZERO) );
        GPU_API_Write_DLCmd(STENCIL_FUNC(GEQUAL,1,255));
        /* one side points */

        for(i=1;i<=numpoints;i++)
        {
            asize = i*rball*2/(numpoints + 1);
            asmooth = (ft_int16_t)smoothlerp((float)((float)(asize)/(2*(float)rball)),0,2*(float)rball);

            if(asmooth > rball)
            {
                //change the offset to -ve
                ft_int32_t tempsmooth;
                tempsmooth = asmooth - rball;
                aradius = (rball*rball + tempsmooth*tempsmooth)/(2*tempsmooth);
                GPU_API_Write_DLCmd(POINT_SIZE(aradius*16) );
                GPU_API_Write_DLCmd(VERTEX2F((xball - aradius + tempsmooth)*16,yball*16));
            }
            else
            {
                ft_int32_t tempsmooth;
                tempsmooth = rball - asmooth;
                aradius = (rball*rball + tempsmooth*tempsmooth)/(2*tempsmooth);
                GPU_API_Write_DLCmd(POINT_SIZE(aradius*16) );
                GPU_API_Write_DLCmd(VERTEX2F((xball+ aradius - tempsmooth)*16,yball*16));
            }
        }



        GPU_API_Write_DLCmd(END());
        GPU_API_Write_DLCmd(BEGIN(LINES));
        /* draw lines - line should be at least radius diameter */
        for(i=1;i<=numlines;i++)
        {
            asize = (i*rball*2/numlines);
            asmooth = (ft_int16_t)smoothlerp((float)((float)(asize)/(2*(float)rball)),0,2*(float)rball);
            GPU_API_Write_DLCmd(LINE_WIDTH(asmooth * 16));
            GPU_API_Write_DLCmd(VERTEX2F((xball - rball)*16,(yball - rball )*16));
            GPU_API_Write_DLCmd(VERTEX2F((xball + rball)*16,(yball - rball )*16));
        }
        GPU_API_Write_DLCmd(END());

        GPU_API_Write_DLCmd(COLOR_MASK(1,1,1,1) );//enable all the colors
        GPU_API_Write_DLCmd(STENCIL_FUNC(ALWAYS,1,255));
        GPU_API_Write_DLCmd(STENCIL_OP(KEEP,KEEP));
        GPU_API_Write_DLCmd(COLOR_RGB(255, 255, 255) );
        GPU_API_Write_DLCmd(POINT_SIZE(rball*16) );
        GPU_API_Write_DLCmd(BEGIN(POINTS));
        GPU_API_Write_DLCmd(VERTEX2F((xball - 1)*16,(yball - 1)*16));
        GPU_API_Write_DLCmd(COLOR_RGB(0, 0, 0) );//shadow
        GPU_API_Write_DLCmd(COLOR_A(160) );
        GPU_API_Write_DLCmd(VERTEX2F((xball+16)*16,(yball+8)*16));
        GPU_API_Write_DLCmd(COLOR_A(255) );
        GPU_API_Write_DLCmd(COLOR_RGB(255, 255, 255) );
        GPU_API_Write_DLCmd(VERTEX2F(xball*16,yball*16));
        GPU_API_Write_DLCmd(COLOR_RGB(255, 0, 0) );
        GPU_API_Write_DLCmd(STENCIL_FUNC(GEQUAL,1,1));
        GPU_API_Write_DLCmd(STENCIL_OP(KEEP,KEEP));
        GPU_API_Write_DLCmd(VERTEX2F(xball*16,yball*16));

        GPU_API_Write_DLCmd(END());

        GPU_API_Write_DLCmd(DISPLAY());

        /* Reset the DL Buffer index, set for the next group of DL commands */
        GPU_API_Reset_DLBuffer();

        /* Do a swap */
        GPU_API_GPU_DLSwap(DLSWAP_FRAME);
        GPU_APP_ENABLE_DELAY_MS(10);

    }

}
#endif

#ifdef GPU_APP_ENABLE_APIS_SET4
/* API to explain the usage of touch engine */
ft_void_t GPU_APP_Touch()
{
    ft_int32_t LoopFlag = 0,wbutton,hbutton,tagval,tagoption;
    ft_char8_t StringArray[100];
    ft_uint32_t ReadWord;
    ft_int16_t xvalue,yvalue,pendown;

    /*************************************************************************/
    /* Below code demonstrates the usage of touch function. Display info     */
    /* touch raw, touch screen, touch tag, raw adc and resistance values     */
    /*************************************************************************/
    LoopFlag = 300;
    wbutton = DispWidth/8;
    hbutton = DispHeight/8;
    while(LoopFlag--)
    {

        GPU_CoCmd_Dlstart(pScreen);
        GPU_API_Write_CoCmd(CLEAR_COLOR_RGB(64,64,64));
        GPU_API_Write_CoCmd(CLEAR(1,1,1));
        GPU_API_Write_CoCmd(COLOR_RGB(0xff,0xff,0xff));
        GPU_API_Write_CoCmd(TAG_MASK(0));
        /* Draw informative text at width/2,20 location */
        StringArray[0] = '\0';
        strcat(StringArray,"Touch Raw XY (");
        ReadWord = GPU_HAL_Rd32(pScreen, REG_TOUCH_RAW_XY);
        yvalue = (ft_int16_t)(ReadWord & 0xffff);
        xvalue = (ft_int16_t)((ReadWord>>16) & 0xffff);
        GPU_HAL_Dec2ASCII(StringArray,(ft_int32_t)xvalue);
        strcat(StringArray,",");
        GPU_HAL_Dec2ASCII(StringArray,(ft_int32_t)yvalue);
        strcat(StringArray,")");
        GPU_CoCmd_Text(pScreen,DispWidth/2, 10, 26, OPT_CENTER, StringArray);

        StringArray[0] = '\0';
        strcat(StringArray,"Touch RZ (");
        ReadWord = GPU_HAL_Rd16(pScreen,REG_TOUCH_RZ);
        GPU_HAL_Dec2ASCII(StringArray,ReadWord);
        strcat(StringArray,")");
        GPU_CoCmd_Text(pScreen,DispWidth/2, 25, 26, OPT_CENTER, StringArray);

        StringArray[0] = '\0';
        strcat(StringArray,"Touch Screen XY (");
        ReadWord = GPU_HAL_Rd32(pScreen, REG_TOUCH_SCREEN_XY);
        yvalue = (ft_int16_t)(ReadWord & 0xffff);
        xvalue = (ft_int16_t)((ReadWord>>16) & 0xffff);
        GPU_HAL_Dec2ASCII(StringArray,(ft_int32_t)xvalue);
        strcat(StringArray,",");
        GPU_HAL_Dec2ASCII(StringArray,(ft_int32_t)yvalue);
        strcat(StringArray,")");
        GPU_CoCmd_Text(pScreen,DispWidth/2, 40, 26, OPT_CENTER, StringArray);

        StringArray[0] = '\0';
        strcat(StringArray,"Touch TAG (");
        ReadWord = GPU_HAL_Rd8(pScreen, REG_TOUCH_TAG);
        GPU_HAL_Dec2ASCII(StringArray,ReadWord);
        strcat(StringArray,")");
        GPU_CoCmd_Text(pScreen,DispWidth/2, 55, 26, OPT_CENTER, StringArray);
        tagval = ReadWord;
        StringArray[0] = '\0';
        strcat(StringArray,"Touch Direct XY (");
        ReadWord = GPU_HAL_Rd32(pScreen, REG_TOUCH_DIRECT_XY);
        yvalue = (ft_int16_t)(ReadWord & 0x03ff);
        xvalue = (ft_int16_t)((ReadWord>>16) & 0x03ff);
        GPU_HAL_Dec2ASCII(StringArray,(ft_int32_t)xvalue);
        strcat(StringArray,",");
        GPU_HAL_Dec2ASCII(StringArray,(ft_int32_t)yvalue);
        pendown = (ft_int16_t)((ReadWord>>31) & 0x01);
        strcat(StringArray,",");
        GPU_HAL_Dec2ASCII(StringArray,(ft_int32_t)pendown);
        strcat(StringArray,")");
        GPU_CoCmd_Text(pScreen,DispWidth/2, 70, 26, OPT_CENTER, StringArray);

        StringArray[0] = '\0';
        strcat(StringArray,"Touch Direct Z1Z2 (");
        ReadWord = GPU_HAL_Rd32(pScreen, REG_TOUCH_DIRECT_Z1Z2);
        yvalue = (ft_int16_t)(ReadWord & 0x03ff);
        xvalue = (ft_int16_t)((ReadWord>>16) & 0x03ff);
        GPU_HAL_Dec2ASCII(StringArray,(ft_int32_t)xvalue);
        strcat(StringArray,",");
        GPU_HAL_Dec2ASCII(StringArray,(ft_int32_t)yvalue);
        strcat(StringArray,")");

        GPU_CoCmd_Text(pScreen,DispWidth/2, 85, 26, OPT_CENTER, StringArray);

        GPU_CoCmd_FgColor(pScreen,0x008000);
        GPU_API_Write_CoCmd(TAG_MASK(1));
        tagoption = 0;
        if(12 == tagval)
        {
            tagoption = OPT_FLAT;
        }

        GPU_API_Write_CoCmd(TAG(12));
        GPU_CoCmd_Button(pScreen,(DispWidth/4) - (wbutton/2),(DispHeight*2/4) - (hbutton/2),wbutton,hbutton,26,tagoption,"Tag12");
        GPU_API_Write_CoCmd(TAG(13));
        tagoption = 0;
        if(13 == tagval)
        {
            tagoption = OPT_FLAT;
        }
        GPU_CoCmd_Button(pScreen,(DispWidth*3/4) - (wbutton/2),(DispHeight*3/4) - (hbutton/2),wbutton,hbutton,26,tagoption,"Tag13");

        GPU_API_Write_CoCmd(DISPLAY());
        GPU_CoCmd_Swap(pScreen);

        /* Wait till co-processor completes the operation */
        GPU_HAL_WaitCmdfifo_empty(pScreen);
        vTaskDelay( 30 / portTICK_PERIOD_MS );
    }
}

static ft_void_t GPU_APP_playmutesound()
{
    GPU_HAL_Wr16(pScreen,REG_SOUND,0x0060);
    GPU_HAL_Wr8(pScreen,REG_PLAY,0x01);
}


/* APP to demonstrate the usage of sound engine of FT800 */

ft_const_uint8_t GPU_APP_Snd_Array[5*58] =
    "Slce\0Sqrq\0Sinw\0Saww\0Triw\0Beep\0Alrm\0Warb\0Crsl\0Pp01\0Pp02\0Pp03\0Pp04\0Pp05\0Pp06\0Pp07\0Pp08\0Pp09\0Pp10\0Pp11\0Pp12\0Pp13\0Pp14\0Pp15\0Pp16\0DMF#\0DMF*\0DMF0\0DMF1\0DMF2\0DMF3\0DMF4\0DMF5\0DMF6\0DMF7\0DMF8\0DMF9\0Harp\0Xyph\0Tuba\0Glok\0Orgn\0Trmp\0Pian\0Chim\0MBox\0Bell\0Clck\0Swth\0Cowb\0Noth\0Hiht\0Kick\0Pop \0Clak\0Chak\0Mute\0uMut\0";

ft_const_uint8_t GPU_APP_Snd_TagArray[58] = {
    0x63,0x01,0x02,0x03,0x04,0x05,0x06,0x07,
    0x08,0x10,0x11,0x12,0x13,0x14,0x15,0x16,
    0x17,0x18,0x19,0x1a,0x1b,0x1c,0x1d,0x1e,
    0x1f,0x23,0x2C,0x30,0x31,0x32,0x33,0x34, // x2a -> x2C DTMF*
    0x35,0x36,0x37,0x38,0x39,0x40,0x41,0x42,
    0x43,0x44,0x45,0x46,0x47,0x48,0x49,0x50,
    0x51,0x52,0x53,0x54,0x55,0x56,0x57,0x58,
    0x60,0x61
};

#define SOUND_TRACK_TAG            100
#define SOUND_LOOP_ITERATIONS      1000

ft_void_t GPU_APP_Sound()
{
    ft_uint16_t LoopFlag = SOUND_LOOP_ITERATIONS;
    ft_uint8_t numbtnrow, numbtncol;
    ft_uint16_t wbutton, hbutton;

    ft_int8_t tagval, tagvalsnd = -1;
    ft_int8_t prevtag = -1;
    ft_uint32_t freqtrack = 0, currfreq = 0, prevfreq = 0;

    /*************************************************************************/
    /* Below code demonstrates the usage of sound function. All the supported*/
    /* sounds and respective pitches are put as part of keys/buttons, by     */
    /* choosing particular key/button the sound is played                    */
    /*************************************************************************/

    numbtnrow = 7 /*16*/;//number of rows to be created - note that mute and unmute are not played in this application
    numbtncol = 8 /*13*/;//number of columns to be created
    wbutton = (DispWidth - 40)/numbtncol;
    hbutton = DispHeight/numbtnrow;

    /* set the volume to maximum */
    GPU_HAL_Wr8(pScreen, REG_VOL_SOUND, 0xFF);
    /* set the tracker to track the slider for frequency */

    GPU_CoCmd_Track(pScreen, DispWidth - 15, 20, 8, (DispHeight - 40), SOUND_TRACK_TAG);

    while(LoopFlag--)
    {
        tagval = GPU_HAL_Rd8(pScreen, REG_TOUCH_TAG);
        freqtrack = GPU_HAL_Rd32(pScreen, REG_TRACKER);

        if( (freqtrack & 0xff) == SOUND_TRACK_TAG)
        {
            currfreq = (ft_uint32_t)(freqtrack >> 16);
            currfreq = (ft_uint32_t)(88*currfreq)/65536;
            if(currfreq > 108)
                currfreq = 108;
        }
        if((tagval > 0))
        {
            if( tagval <= 99)
            {
                tagvalsnd = tagval;
            }
            if(0x63 == tagvalsnd)
            {
                tagvalsnd = 0;
            }
            if((prevtag != tagval) || (prevfreq != currfreq))
            {
                /* Play sound wrt pitch */
                GPU_HAL_Wr16(pScreen, REG_SOUND, (((currfreq + 21) << 8) | tagvalsnd));
                GPU_HAL_Wr8(pScreen, REG_PLAY, 1);
            }
            if( tagvalsnd == 0)
                tagvalsnd = 99;
        }

        /* start a new display list for construction of screen */
        GPU_CoCmd_Dlstart(pScreen);
        GPU_API_Write_CoCmd(CLEAR_COLOR_RGB(64,64,64));
        GPU_API_Write_CoCmd(CLEAR(1,1,1));

        /* line width for the rectangles */
        GPU_API_Write_CoCmd(LINE_WIDTH(1*16));

        /* custom keys for sound input */
        /* First draw all the rectangles followed by the font */
        /* yellow colour for background colour */
        GPU_API_Write_CoCmd(COLOR_RGB(0x80,0x80,0x00));

        GPU_API_Write_CoCmd(BEGIN(RECTS));
        for(ft_uint8_t i=0; i<numbtnrow; ++i)
        {
            for(ft_uint8_t j=0; j<numbtncol; ++j)
            {
                GPU_API_Write_CoCmd(TAG(pgm_read_byte_near(&GPU_APP_Snd_TagArray[(i * numbtncol) + j ])));
                GPU_API_Write_CoCmd(VERTEX2II((j*wbutton + 2),(hbutton*i + 2),0,0));
                GPU_API_Write_CoCmd(VERTEX2II(((j*wbutton) + wbutton - 2),((hbutton*i) + hbutton - 2),0,0));
            }
        }
        GPU_API_Write_CoCmd(END());

        /* draw the highlight rectangle and text info */
        GPU_API_Write_CoCmd(COLOR_RGB(0xff,0xff,0xff));

        for(ft_uint8_t i=0; i<numbtnrow; ++i)
        {
            for(ft_uint8_t j=0; j<numbtncol; ++j)
            {
                GPU_API_Write_CoCmd((ft_uint32_t)TAG(pgm_read_byte_near(&GPU_APP_Snd_TagArray[(i * numbtncol) + j ])));
                if(tagvalsnd == pgm_read_byte_near(&GPU_APP_Snd_TagArray[(i * numbtncol) + j ]))
                {
                    /* red colour for highlight effect */
                    GPU_API_Write_CoCmd(COLOR_RGB(0x80,0x00,0x00));
                    GPU_API_Write_CoCmd(BEGIN(RECTS));
                    GPU_API_Write_CoCmd(TAG(pgm_read_byte_near(&GPU_APP_Snd_TagArray[(i * numbtncol) + j ])));
                    GPU_API_Write_CoCmd(VERTEX2II((j*wbutton + 2),(hbutton*i + 2),0,0));
                    GPU_API_Write_CoCmd(VERTEX2II(((j*wbutton) + wbutton - 2),((hbutton*i) + hbutton - 2),0,0));
                    GPU_API_Write_CoCmd(END());
                    /* reset the colour to make sure font doesn't get impacted */
                    GPU_API_Write_CoCmd(COLOR_RGB(0xff,0xff,0xff));
                }
                /* to make sure that highlight rectangle as well as font to take the same tag values */

                GPU_CoCmd_Text(pScreen, (wbutton/2) + j*wbutton,(hbutton/2) + hbutton*i, 26, OPT_CENTER, (ft_const_char8_t*)&GPU_APP_Snd_Array[((i * numbtncol) + j)*5]);
            }
        }

        /* Draw vertical slider bar for frequency control */
        GPU_API_Write_CoCmd(TAG_MASK(0));
        GPU_CoCmd_Text(pScreen,DispWidth - 20,10,26,OPT_CENTER, "Freq");
        GPU_API_Write_CoCmd(TAG_MASK(1));
        GPU_API_Write_CoCmd(TAG(100));
        GPU_CoCmd_Slider(pScreen, DispWidth - 15, 20, 8, (DispHeight - 40), 0, currfreq, 88);

        GPU_API_Write_CoCmd(DISPLAY());
        GPU_CoCmd_Swap(pScreen);

        prevtag = tagval;
        prevfreq = currfreq;

        /* Wait till coprocessor completes the operation */
        GPU_HAL_WaitCmdfifo_empty(pScreen);
        GPU_APP_ENABLE_DELAY_MS(10);
    }

    GPU_HAL_Wr16(pScreen, REG_SOUND,0);
    GPU_HAL_Wr8(pScreen, REG_PLAY,1);
}


ft_void_t GPU_APP_PowerMode()
{
    /*************************************************
    Senario1:  Transition from Active mode to Standby mode.
               Transition from Standby mode to Active Mode
    **************************************************/
    GPU_APP_Screen("Active to Standby Mode");

    //Switch FT800 from Active to Standby mode
    GPU_API_fadeout();
    GPU_APP_playmutesound();//Play mute sound to avoid pop sound
    GPU_PowerModeSwitch(pScreen,GPU_STANDBY_M);
    GPU_APP_ENABLE_DELAY();

    //Wake up from Standby first before accessing FT800 registers.
    GPU_PowerModeSwitch(pScreen,GPU_ACTIVE_M);
    GPU_API_fadein();
    GPU_APP_ENABLE_DELAY();

    /*************************************************
    Senario2:  Transition from Active mode to Sleep mode.
               Transition from Sleep mode to Active Mode
    **************************************************/
    //Switch FT800 from Active to Sleep mode
    GPU_APP_Screen("Active to Sleep Mode");
    GPU_API_fadeout();
    GPU_APP_playmutesound();//Play mute sound to avoid pop sound
    GPU_PowerModeSwitch(pScreen,GPU_SLEEP_M);
    GPU_APP_ENABLE_DELAY();

    //Wake up from Sleep
    GPU_PowerModeSwitch(pScreen,GPU_ACTIVE_M);
    GPU_API_fadein();
    GPU_APP_ENABLE_DELAY();

    /*************************************************
    Senario3:  Transition from Active mode to PowerDown mode.
               Transition from PowerDown mode to Active Mode via Standby mode.
    **************************************************/
    //Switch FT800 from Active to PowerDown mode by sending command
    GPU_APP_Screen("Active to PowerDown Mode");
    GPU_API_fadeout();
    GPU_APP_playmutesound();//Play mute sound to avoid pop sound
    GPU_PowerModeSwitch(pScreen,GPU_POWERDOWN_M);
    GPU_APP_ENABLE_DELAY();

    GPU_API_Boot_Config();
    //Need to download display list again because power down mode lost all registers and memory
    GPU_Home_Setup();
    GPU_API_fadein();
    GPU_APP_ENABLE_DELAY();
}
#endif

ft_const_uint8_t home_star_icon[] = {0x78,0x9C,0xE5,0x94,0xBF,0x4E,0xC2,0x40,0x1C,0xC7,0x7F,0x2D,0x04,0x8B,0x20,0x45,0x76,0x14,0x67,0xA3,0xF1,0x0D,0x64,0x75,0xD2,0xD5,0x09,0x27,0x17,0x13,0xE1,0x0D,0xE4,0x0D,0x78,0x04,0x98,0x5D,0x30,0x26,0x0E,0x4A,0xA2,0x3E,0x82,0x0E,0x8E,0x82,0xC1,0x38,0x62,0x51,0x0C,0x0A,0x42,0x7F,0xDE,0xB5,0x77,0xB4,0x77,0x17,0x28,0x21,0x26,0x46,0xFD,0x26,0xCD,0xE5,0xD3,0x7C,0xFB,0xBB,0xFB,0xFD,0xB9,0x02,0xCC,0xA4,0xE8,0x99,0x80,0x61,0xC4,0x8A,0x9F,0xCB,0x6F,0x31,0x3B,0xE3,0x61,0x7A,0x98,0x84,0x7C,0x37,0xF6,0xFC,0xC8,0xDD,0x45,0x00,0xDD,0xBA,0xC4,0x77,0xE6,0xEE,0x40,0xEC,0x0E,0xE6,0x91,0xF1,0xD2,0x00,0x42,0x34,0x5E,0xCE,0xE5,0x08,0x16,0xA0,0x84,0x68,0x67,0xB4,0x86,0xC3,0xD5,0x26,0x2C,0x20,0x51,0x17,0xA2,0xB8,0x03,0xB0,0xFE,0x49,0xDD,0x54,0x15,0xD8,0xEE,0x73,0x37,0x95,0x9D,0xD4,0x1A,0xB7,0xA5,0x26,0xC4,0x91,0xA9,0x0B,0x06,0xEE,0x72,0xB7,0xFB,0xC5,0x16,0x80,0xE9,0xF1,0x07,0x8D,0x3F,0x15,0x5F,0x1C,0x0B,0xFC,0x0A,0x90,0xF0,0xF3,0x09,0xA9,0x90,0xC4,0xC6,0x37,0xB0,0x93,0xBF,0xE1,0x71,0xDB,0xA9,0xD7,0x41,0xAD,0x46,0xEA,0x19,0xA9,0xD5,0xCE,0x93,0xB3,0x35,0x73,0x0A,0x69,0x59,0x91,0xC3,0x0F,0x22,0x1B,0x1D,0x91,0x13,0x3D,0x91,0x73,0x43,0xF1,0x6C,0x55,0xDA,0x3A,0x4F,0xBA,0x25,0xCE,0x4F,0x04,0xF1,0xC5,0xCF,0x71,0xDA,0x3C,0xD7,0xB9,0xB2,0x48,0xB4,0x89,0x38,0x20,0x4B,0x2A,0x95,0x0C,0xD5,0xEF,0x5B,0xAD,0x96,0x45,0x8A,0x41,0x96,0x7A,0x1F,0x60,0x0D,0x7D,0x22,0x75,0x82,0x2B,0x0F,0xFB,0xCE,0x51,0x3D,0x2E,0x3A,0x21,0xF3,0x1C,0xD9,0x38,0x86,0x2C,0xC6,0x05,0xB6,0x7B,0x9A,0x8F,0x0F,0x97,0x1B,0x72,0x6F,0x1C,0xEB,0xAE,0xFF,0xDA,0x97,0x0D,0xBA,0x43,0x32,0xCA,0x66,0x34,0x3D,0x54,0xCB,0x24,0x9B,0x43,0xF2,0x70,0x3E,0x42,0xBB,0xA0,0x95,0x11,0x37,0x46,0xE1,0x4F,0x49,0xC5,0x1B,0xFC,0x3C,0x3A,0x3E,0xD1,0x65,0x0E,0x6F,0x58,0xF8,0x9E,0x5B,0xDB,0x55,0xB6,0x41,0x34,0xCB,0xBE,0xDB,0x87,0x5F,0xA9,0xD1,0x85,0x6B,0xB3,0x17,0x9C,0x61,0x0C,0x9B,0xA2,0x5D,0x61,0x10,0xED,0x2A,0x9B,0xA2,0x5D,0x61,0x10,0xED,0x2A,0x9B,0xA2,0x5D,0x61,0x10,0xED,0x2A,0x9B,0xED,0xC9,0xFC,0xDF,0x14,0x54,0x8F,0x80,0x7A,0x06,0xF5,0x23,0xA0,0x9F,0x41,0xF3,0x10,0x30,0x4F,0x41,0xF3,0x18,0x30,0xCF,0xCA,0xFC,0xFF,0x35,0xC9,0x79,0xC9,0x89,0xFA,0x33,0xD7,0x1D,0xF6,0x5E,0x84,0x5C,0x56,0x6E,0xA7,0xDA,0x1E,0xF9,0xFA,0xAB,0xF5,0x97,0xFF,0x2F,0xED,0x89,0x7E,0x29,0x9E,0xB4,0x9F,0x74,0x1E,0x69,0xDA,0xA4,0x9F,0x81,0x94,0xEF,0x4F,0xF6,0xF9,0x0B,0xF4,0x65,0x51,0x08};

ft_void_t GPU_Home_Setup(ft_void_t)
{
    GPU_HAL_WrCmd32(pScreen, CMD_INFLATE);
    GPU_HAL_WrCmd32(pScreen, 250*1024L);
    GPU_HAL_WrCmdBuf(pScreen, home_star_icon, sizeof(home_star_icon));

    GPU_CoCmd_Dlstart(pScreen);        // start
    GPU_API_Write_CoCmd( CLEAR(1,1,1) );
    GPU_API_Write_CoCmd( COLOR_RGB(255, 255, 255) );
    GPU_API_Write_CoCmd( BITMAP_HANDLE(13) );    // handle for background stars
    GPU_API_Write_CoCmd( BITMAP_SOURCE(250*1024L) );   // Starting address in RAM_G
    GPU_API_Write_CoCmd( BITMAP_LAYOUT(L4, 16, 32) );  // format
    GPU_API_Write_CoCmd( BITMAP_SIZE(NEAREST, REPEAT, REPEAT, 512, 512) );
    GPU_API_Write_CoCmd( BITMAP_HANDLE(14) );    // handle for background stars
    GPU_API_Write_CoCmd( BITMAP_SOURCE(250*1024L) );   // Starting address in RAM_G
    GPU_API_Write_CoCmd( BITMAP_LAYOUT(L4, 16, 32) );  // format
    GPU_API_Write_CoCmd( BITMAP_SIZE(NEAREST, BORDER, BORDER, 32, 32) );
    GPU_API_Write_CoCmd( DISPLAY() );
    GPU_CoCmd_Swap(pScreen);

    GPU_HAL_WaitCmdfifo_empty(pScreen);
}

ft_void_t GPU_Info(ft_void_t)
{
    GPU_CoCmd_Dlstart(pScreen);
    GPU_API_Write_CoCmd(CLEAR(1,1,1));
    GPU_API_Write_CoCmd(COLOR_RGB(255,255,255));
    GPU_CoCmd_Text(pScreen,DispWidth/2,DispHeight/2,26,OPT_CENTERX|OPT_CENTERY, "Please tap on each dot");
    GPU_CoCmd_Calibrate(pScreen,0);
    GPU_API_Write_CoCmd(DISPLAY());
    GPU_CoCmd_Swap(pScreen);
    GPU_HAL_WaitCmdfifo_empty(pScreen);

    GPU_CoCmd_Logo(pScreen);
    GPU_HAL_WaitCmdfifo_empty(pScreen);
}

/* Nothing beyond this */
