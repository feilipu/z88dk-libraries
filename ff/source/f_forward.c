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
/* Forward Data to the Stream Directly                                   */
/*-----------------------------------------------------------------------*/
#if FF_USE_FORWARD

FRESULT f_forward (
    FIL* fp,                        /* Pointer to the file object */
    UINT (*func)(const BYTE*,UINT), /* Pointer to the streaming function */
    UINT btf,                       /* Number of bytes to forward */
    UINT* bf                        /* Pointer to number of bytes forwarded */
)
{
    FRESULT res;
    FATFS *fs;
    DWORD clst, sect;
    FSIZE_t remain;
    UINT rcnt, csect;
    BYTE *dbuf;


    *bf = 0;    /* Clear transfer byte counter */
    res = validate(&fp->obj, &fs);        /* Check validity of the file object */
    if (res != FR_OK || (res = (FRESULT)fp->err) != FR_OK) LEAVE_FF(fs, res);
    if (!(fp->flag & FA_READ)) LEAVE_FF(fs, FR_DENIED);    /* Check access mode */

    remain = fp->obj.objsize - fp->fptr;
    if (btf > remain) btf = (UINT)remain;            /* Truncate btf by remaining bytes */

    for ( ;  btf && (*func)(0, 0);                    /* Repeat until all data transferred or stream goes busy */
        fp->fptr += rcnt, *bf += rcnt, btf -= rcnt) {
        csect = (UINT)(fp->fptr / SS(fs) & (fs->csize - 1));    /* Sector offset in the cluster */
        if (fp->fptr % SS(fs) == 0) {                /* On the sector boundary? */
            if (csect == 0) {                        /* On the cluster boundary? */
                clst = (fp->fptr == 0) ?            /* On the top of the file? */
                    fp->obj.sclust : get_fat(&fp->obj, fp->clust);
                if (clst <= 1) ABORT(fs, FR_INT_ERR);
                if (clst == 0xFFFFFFFF) ABORT(fs, FR_DISK_ERR);
                fp->clust = clst;                    /* Update current cluster */
            }
        }
        sect = clst2sect(fs, fp->clust);            /* Get current data sector */
        if (sect == 0) ABORT(fs, FR_INT_ERR);
        sect += csect;
#if FF_FS_TINY
        if (move_window(fs, sect) != FR_OK) ABORT(fs, FR_DISK_ERR);    /* Move sector window to the file data */
        dbuf = fs->win;
#else
        if (fp->sect != sect) {        /* Fill sector cache with file data */
#if !FF_FS_READONLY
            if (fp->flag & FA_DIRTY) {        /* Write-back dirty sector cache */
                if (disk_write(fs->pdrv, fp->buf, fp->sect, 1) != RES_OK) ABORT(fs, FR_DISK_ERR);
                fp->flag &= (BYTE)~FA_DIRTY;
            }
#endif
            if (disk_read(fs->pdrv, fp->buf, sect, 1) != RES_OK) ABORT(fs, FR_DISK_ERR);
        }
        dbuf = fp->buf;
#endif
        fp->sect = sect;
        rcnt = SS(fs) - (UINT)fp->fptr % SS(fs);    /* Number of bytes left in the sector */
        if (rcnt > btf) rcnt = btf;                    /* Clip it by btr if needed */
        rcnt = (*func)(dbuf + ((UINT)fp->fptr % SS(fs)), rcnt);    /* Forward the file data */
        if (rcnt == 0) ABORT(fs, FR_INT_ERR);
    }

    LEAVE_FF(fs, FR_OK);
}

#endif /* FF_USE_FORWARD */

