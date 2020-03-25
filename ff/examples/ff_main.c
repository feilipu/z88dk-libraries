/*----------------------------------------------------------------------*/
/* Foolproof FatFs sample project for Z80              (C)ChaN, 2014    */
/*----------------------------------------------------------------------*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "ffconf.h"            /* FatFs configuration.
                                  Use same file as you used to compile library"*/
#if __YAZ180
#include <lib/yaz180/ff.h>     /* Declarations of FatFs API */
#elif __RC2014
#include <lib/rc2014/ff.h>     /* Declarations of FatFs API */
#else
#warning - no FatFs library available
#endif

// zcc +yaz180 -subtype=app -v --list -m -SO3 -clib=sdcc_iy -llib/yaz180/ff --max-allocs-per-node100000 ff_main.c -o ff_main -create-app

// zcc +rc2014 -subtype=basic_dcio -v --list -m -SO3 -clib=sdcc_iy -llib/rc2014/ff --max-allocs-per-node100000 ff_main.c -o ff_main -create-app

// doke &h2704, &h2900 (Look for __Start in ff_main.map)
// doke &h8224, &h9000 (rc2014, subtype basic_dcio NASCOM Basic)

static FATFS FatFs;        /* FatFs work area needed for each volume */
static FIL Fil;            /* File object needed for each open file */

char buffer[64];

int main (void)
{
    UINT bw;
    UINT br;
    
    FRESULT res;

    printf("Mounting...");
    res = f_mount(&FatFs, "", 1);                 /* Give a work area to the default drive */
    if(res != FR_OK) printf("f_mount error #%u\r\n", res);

    if (f_open(&Fil, "afile.txt", FA_OPEN_ALWAYS | FA_WRITE | FA_READ | FA_OPEN_EXISTING) == FR_OK)
    {    /* Create a file */

        res = f_write(&Fil, "It works!\r\n", 11, &bw);  /* Write data to the file */
        if(res != FR_OK) printf("f_write error #%u\r\n", res);
        
        br = 0;
        res = f_read(&Fil, &buffer, 64, &br);             /* Read data from the file */
        if(res != FR_OK) printf("f_read error #%u\r\n", res);

        res = f_close(&Fil);                              /* Close the file */
        if(res != FR_OK) printf("f_close error #%u\r\n", res);        

        printf("\r\n%u bytes: %s\r\n", br, buffer);
        if(br > 0) printf("It works!\r\n");
    }
    else
    {
        printf("f_open error #%u\r\n", res);
    }
    return 0;
}

