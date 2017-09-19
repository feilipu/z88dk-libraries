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
/* Truncate File                                                         */
/*-----------------------------------------------------------------------*/
#if FF_FS_MINIMIZE == 0
#if !FF_FS_READONLY

FRESULT f_truncate (
    FIL* fp        /* Pointer to the file object */
)
{
    FRESULT res;
    FATFS *fs;
    DWORD ncl;


    res = validate(&fp->obj, &fs);    /* Check validity of the file object */
    if (res != FR_OK || (res = (FRESULT)fp->err) != FR_OK) LEAVE_FF(fs, res);
    if (!(fp->flag & FA_WRITE)) LEAVE_FF(fs, FR_DENIED);    /* Check access mode */

    if (fp->fptr < fp->obj.objsize) {    /* Process when fptr is not on the eof */
        if (fp->fptr == 0) {    /* When set file size to zero, remove entire cluster chain */
            res = remove_chain(&fp->obj, fp->obj.sclust, 0);
            fp->obj.sclust = 0;
        } else {                /* When truncate a part of the file, remove remaining clusters */
            ncl = get_fat(&fp->obj, fp->clust);
            res = FR_OK;
            if (ncl == 0xFFFFFFFF) res = FR_DISK_ERR;
            if (ncl == 1) res = FR_INT_ERR;
            if (res == FR_OK && ncl < fs->n_fatent) {
                res = remove_chain(&fp->obj, ncl, fp->clust);
            }
        }
        fp->obj.objsize = fp->fptr;    /* Set file size to current read/write point */
        fp->flag |= FA_MODIFIED;
#if !FF_FS_TINY
        if (res == FR_OK && (fp->flag & FA_DIRTY)) {
            if (disk_write(fs->pdrv, fp->buf, fp->sect, 1) != RES_OK) {
                res = FR_DISK_ERR;
            } else {
                fp->flag &= (BYTE)~FA_DIRTY;
            }
        }
#endif
        if (res != FR_OK) ABORT(fs, res);
    }

    LEAVE_FF(fs, res);
}

#endif /* !FF_FS_READONLY */
#endif /* FF_FS_MINIMIZE == 0 */

