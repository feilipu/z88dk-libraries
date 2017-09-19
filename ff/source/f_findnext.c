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
/* Find Next File                                                        */
/*-----------------------------------------------------------------------*/
#if FF_FS_MINIMIZE <= 1
#if FF_USE_FIND

FRESULT f_findnext (
    DIR* dp,        /* Pointer to the open directory object */
    FILINFO* fno    /* Pointer to the file information structure */
)
{
    FRESULT res;


    for (;;) {
        res = f_readdir(dp, fno);        /* Get a directory item */
        if (res != FR_OK || !fno || !fno->fname[0]) break;    /* Terminate if any error or end of directory */
        if (pattern_matching(dp->pat, fno->fname, 0, 0)) break;        /* Test for the file name */
#if FF_USE_LFN && FF_USE_FIND == 2
        if (pattern_matching(dp->pat, fno->altname, 0, 0)) break;    /* Test for alternative name if exist */
#endif
    }
    return res;
}


#endif /* FF_FS_MINIMIZE <= 1 */
#endif    /* FF_USE_FIND */

