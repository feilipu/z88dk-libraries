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
/* Get File Status                                                       */
/*-----------------------------------------------------------------------*/
#if FF_FS_MINIMIZE == 0

FRESULT f_stat (
    const TCHAR* path,    /* Pointer to the file path */
    FILINFO* fno        /* Pointer to file information to return */
)
{
    FRESULT res;
    DIR dj;
    DEF_NAMBUF


    /* Get logical drive */
    res = find_volume(&path, &dj.obj.fs, 0);
    if (res == FR_OK) {
        INIT_NAMBUF(dj.obj.fs);
        res = follow_path(&dj, path);    /* Follow the file path */
        if (res == FR_OK) {                /* Follow completed */
            if (dj.fn[NSFLAG] & NS_NONAME) {    /* It is origin directory */
                res = FR_INVALID_NAME;
            } else {                            /* Found an object */
                if (fno) get_fileinfo(&dj, fno);
            }
        }
        FREE_NAMBUF();
    }

    LEAVE_FF(dj.obj.fs, res);
}

#endif /* FF_FS_MINIMIZE == 0 */

