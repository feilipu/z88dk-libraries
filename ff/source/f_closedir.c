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
/* Close Directory                                                       */
/*-----------------------------------------------------------------------*/
#if FF_FS_MINIMIZE <= 1

FRESULT f_closedir (
    DIR *dp        /* Pointer to the directory object to be closed */
)
{
    FRESULT res;
    FATFS *fs;


    res = validate(&dp->obj, &fs);    /* Check validity of the file object */
    if (res == FR_OK) {
#if FF_FS_LOCK != 0
        if (dp->obj.lockid) res = dec_lock(dp->obj.lockid);    /* Decrement sub-directory open counter */
        if (res == FR_OK) dp->obj.fs = 0;    /* Invalidate directory object */
#else
        dp->obj.fs = 0;    /* Invalidate directory object */
#endif
#if FF_FS_REENTRANT
        unlock_fs(fs, FR_OK);        /* Unlock volume */
#endif
    }
    return res;
}

#endif /* FF_FS_MINIMIZE <= 1 */

