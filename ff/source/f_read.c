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
/* Read File                                                             */
/*-----------------------------------------------------------------------*/

FRESULT f_read (
    FIL* fp,     /* Pointer to the file object */
    void* buff,    /* Pointer to data buffer */
    UINT btr,    /* Number of bytes to read */
    UINT* br    /* Pointer to number of bytes read */
)
{
    FRESULT res;
    FATFS *fs;
    DWORD clst, sect;
    FSIZE_t remain;
    UINT rcnt, cc, csect;
    BYTE *rbuff = (BYTE*)buff;


    *br = 0;    /* Clear read byte counter */
    res = validate(&fp->obj, &fs);                /* Check validity of the file object */
    if (res != FR_OK || (res = (FRESULT)fp->err) != FR_OK) LEAVE_FF(fs, res);    /* Check validity */
    if (!(fp->flag & FA_READ)) LEAVE_FF(fs, FR_DENIED); /* Check access mode */
    remain = fp->obj.objsize - fp->fptr;
    if (btr > remain) btr = (UINT)remain;        /* Truncate btr by remaining bytes */

    for ( ;  btr;                                /* Repeat until all data read */
        btr -= rcnt, *br += rcnt, rbuff += rcnt, fp->fptr += rcnt) {
        if (fp->fptr % SS(fs) == 0) {            /* On the sector boundary? */
            csect = (UINT)(fp->fptr / SS(fs) & (fs->csize - 1));    /* Sector offset in the cluster */
            if (csect == 0) {                    /* On the cluster boundary? */
                if (fp->fptr == 0) {            /* On the top of the file? */
                    clst = fp->obj.sclust;        /* Follow cluster chain from the origin */
                } else {                        /* Middle or end of the file */
#if FF_USE_FASTSEEK
                    if (fp->cltbl) {
                        clst = clmt_clust(fp, fp->fptr);    /* Get cluster# from the CLMT */
                    } else
#endif
                    {
                        clst = get_fat(&fp->obj, fp->clust);    /* Follow cluster chain on the FAT */
                    }
                }
                if (clst < 2) ABORT(fs, FR_INT_ERR);
                if (clst == 0xFFFFFFFF) ABORT(fs, FR_DISK_ERR);
                fp->clust = clst;                /* Update current cluster */
            }
            sect = clst2sect(fs, fp->clust);    /* Get current sector */
            if (sect == 0) ABORT(fs, FR_INT_ERR);
            sect += csect;
            cc = btr / SS(fs);                    /* When remaining bytes >= sector size, */
            if (cc > 0) {                        /* Read maximum contiguous sectors directly */
                if (csect + cc > fs->csize) {    /* Clip at cluster boundary */
                    cc = fs->csize - csect;
                }
                if (disk_read(fs->pdrv, rbuff, sect, cc) != RES_OK) ABORT(fs, FR_DISK_ERR);
#if !FF_FS_READONLY && FF_FS_MINIMIZE <= 2        /* Replace one of the read sectors with cached data if it contains a dirty sector */
#if FF_FS_TINY
                if (fs->wflag && fs->winsect - sect < cc) {
                    MEMCPY(rbuff + ((fs->winsect - sect) * SS(fs)), fs->win, SS(fs));
                }
#else
                if ((fp->flag & FA_DIRTY) && fp->sect - sect < cc) {
                    MEMCPY(rbuff + ((fp->sect - sect) * SS(fs)), fp->buf, SS(fs));
                }
#endif
#endif
                rcnt = SS(fs) * cc;                /* Number of bytes transferred */
                continue;
            }
#if !FF_FS_TINY
            if (fp->sect != sect) {            /* Load data sector if not in cache */
#if !FF_FS_READONLY
                if (fp->flag & FA_DIRTY) {        /* Write-back dirty sector cache */
                    if (disk_write(fs->pdrv, fp->buf, fp->sect, 1) != RES_OK) ABORT(fs, FR_DISK_ERR);
                    fp->flag &= (BYTE)~FA_DIRTY;
                }
#endif
                if (disk_read(fs->pdrv, fp->buf, sect, 1) != RES_OK)    ABORT(fs, FR_DISK_ERR);    /* Fill sector cache */
            }
#endif
            fp->sect = sect;
        }
        rcnt = SS(fs) - (UINT)fp->fptr % SS(fs);    /* Number of bytes left in the sector */
        if (rcnt > btr) rcnt = btr;                    /* Clip it by btr if needed */
#if FF_FS_TINY
        if (move_window(fs, fp->sect) != FR_OK) ABORT(fs, FR_DISK_ERR);    /* Move sector window */
        MEMCPY(rbuff, fs->win + fp->fptr % SS(fs), rcnt);    /* Extract partial sector */
#else
        MEMCPY(rbuff, fp->buf + fp->fptr % SS(fs), rcnt);    /* Extract partial sector */
#endif
    }

    LEAVE_FF(fs, FR_OK);
}

