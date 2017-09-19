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
/* Read Directory Entries in Sequence                                    */
/*-----------------------------------------------------------------------*/
#if FF_FS_MINIMIZE <= 1

FRESULT f_readdir (
    DIR* dp,            /* Pointer to the open directory object */
    FILINFO* fno        /* Pointer to file information to return */
)
{
    FRESULT res;
    FATFS *fs;
    DEF_NAMBUF


    res = validate(&dp->obj, &fs);    /* Check validity of the directory object */
    if (res == FR_OK) {
        if (!fno) {
            res = dir_sdi(dp, 0);            /* Rewind the directory object */
        } else {
            INIT_NAMBUF(fs);
            res = dir_read(dp, 0);            /* Read an item */
            if (res == FR_NO_FILE) res = FR_OK;    /* Ignore end of directory */
            if (res == FR_OK) {                /* A valid entry is found */
                get_fileinfo(dp, fno);        /* Get the object information */
                res = dir_next(dp, 0);        /* Increment index for next */
                if (res == FR_NO_FILE) res = FR_OK;    /* Ignore end of directory now */
            }
            FREE_NAMBUF();
        }
    }
    LEAVE_FF(fs, res);
}

#endif /* FF_FS_MINIMIZE <= 1 */

