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
/* Find First File                                                       */
/*-----------------------------------------------------------------------*/
#if FF_FS_MINIMIZE <= 1
#if FF_USE_FIND

FRESULT f_findfirst (
    DIR* dp,                /* Pointer to the blank directory object */
    FILINFO* fno,            /* Pointer to the file information structure */
    const TCHAR* path,        /* Pointer to the directory to open */
    const TCHAR* pattern    /* Pointer to the matching pattern */
)
{
    FRESULT res;


    dp->pat = pattern;        /* Save pointer to pattern string */
    res = f_opendir(dp, path);        /* Open the target directory */
    if (res == FR_OK) {
        res = f_findnext(dp, fno);    /* Find the first item */
    }
    return res;
}

#endif /* FF_FS_MINIMIZE <= 1 */
#endif    /* FF_USE_FIND */

