/*----------------------------------------------------------------------------/
/  FatFs - Generic FAT Filesystem Module  R0.13p2                             /
/-----------------------------------------------------------------------------/
/
/ Copyright (C) 2017, ChaN, all right reserved.
/
/ FatFs module is an open source software. Redistribution and use of FatFs in
/ source and binary forms, with or without modification, are permitted provided
/ that the following condition is met:
/
/ 1. Redistributions of source code must retain the above copyright notice,
/    this condition and the following disclaimer.
/
/ This software is provided by the copyright holder and contributors "AS IS"
/ and any warranties related to this software are DISCLAIMED.
/ The copyright owner or contributors be NOT LIABLE for any damages caused
/ by use of this software.
/
/----------------------------------------------------------------------------*/

#include "ff.h"         /* FatFs Public API */
#include "ffprivate.h"  /* FatFs Private Functions */
#include "ffunicode.h"  /* FatFS Unicode */

#include "__ffstore.h"          /* extern for system storage */
#include "__ffunicodestore.h"   /* extern for LFN system storage */


/*-----------------------------------------------------------------------*/
/* Close File                                                            */
/*-----------------------------------------------------------------------*/

FRESULT f_close (
    FIL* fp        /* Pointer to the file object to be closed */
)
{
    FRESULT res;
    FATFS *fs;

#if !FF_FS_READONLY
    res = f_sync(fp);                    /* Flush cached data */
    if (res == FR_OK)
#endif
    {
        res = validate(&fp->obj, &fs);    /* Lock volume */
        if (res == FR_OK) {
#if FF_FS_LOCK != 0
            res = dec_lock(fp->obj.lockid);        /* Decrement file open counter */
            if (res == FR_OK) fp->obj.fs = 0;    /* Invalidate file object */
#else
            fp->obj.fs = 0;    /* Invalidate file object */
#endif
#if FF_FS_REENTRANT
            unlock_fs(fs, FR_OK);        /* Unlock volume */
#endif
        }
    }
    return res;
}

