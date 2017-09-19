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
/* Write File                                                            */
/*-----------------------------------------------------------------------*/
#if !FF_FS_READONLY

FRESULT f_write (
    FIL* fp,            /* Pointer to the file object */
    const void* buff,    /* Pointer to the data to be written */
    UINT btw,            /* Number of bytes to write */
    UINT* bw            /* Pointer to number of bytes written */
)
{
    FRESULT res;
    FATFS *fs;
    DWORD clst, sect;
    UINT wcnt, cc, csect;
    const BYTE *wbuff = (const BYTE*)buff;


    *bw = 0;    /* Clear write byte counter */
    res = validate(&fp->obj, &fs);            /* Check validity of the file object */
    if (res != FR_OK || (res = (FRESULT)fp->err) != FR_OK) LEAVE_FF(fs, res);    /* Check validity */
    if (!(fp->flag & FA_WRITE)) LEAVE_FF(fs, FR_DENIED);    /* Check access mode */

    /* Check fptr wrap-around (file size cannot reach 4 GiB at FAT volume) */
    if ((!FF_FS_EXFAT || fs->fs_type != FS_EXFAT) && (DWORD)(fp->fptr + btw) < (DWORD)fp->fptr) {
        btw = (UINT)(0xFFFFFFFF - (DWORD)fp->fptr);
    }

    for ( ;  btw;                            /* Repeat until all data written */
        btw -= wcnt, *bw += wcnt, wbuff += wcnt, fp->fptr += wcnt, fp->obj.objsize = (fp->fptr > fp->obj.objsize) ? fp->fptr : fp->obj.objsize) {
        if (fp->fptr % SS(fs) == 0) {        /* On the sector boundary? */
            csect = (UINT)(fp->fptr / SS(fs)) & (fs->csize - 1);    /* Sector offset in the cluster */
            if (csect == 0) {                /* On the cluster boundary? */
                if (fp->fptr == 0) {        /* On the top of the file? */
                    clst = fp->obj.sclust;    /* Follow from the origin */
                    if (clst == 0) {        /* If no cluster is allocated, */
                        clst = create_chain(&fp->obj, 0);    /* create a new cluster chain */
                    }
                } else {                    /* On the middle or end of the file */
#if FF_USE_FASTSEEK
                    if (fp->cltbl) {
                        clst = clmt_clust(fp, fp->fptr);    /* Get cluster# from the CLMT */
                    } else
#endif
                    {
                        clst = create_chain(&fp->obj, fp->clust);    /* Follow or stretch cluster chain on the FAT */
                    }
                }
                if (clst == 0) break;        /* Could not allocate a new cluster (disk full) */
                if (clst == 1) ABORT(fs, FR_INT_ERR);
                if (clst == 0xFFFFFFFF) ABORT(fs, FR_DISK_ERR);
                fp->clust = clst;            /* Update current cluster */
                if (fp->obj.sclust == 0) fp->obj.sclust = clst;    /* Set start cluster if the first write */
            }
#if FF_FS_TINY
            if (fs->winsect == fp->sect && sync_window(fs) != FR_OK) ABORT(fs, FR_DISK_ERR);    /* Write-back sector cache */
#else
            if (fp->flag & FA_DIRTY) {        /* Write-back sector cache */
                if (disk_write(fs->pdrv, fp->buf, fp->sect, 1) != RES_OK) ABORT(fs, FR_DISK_ERR);
                fp->flag &= (BYTE)~FA_DIRTY;
            }
#endif
            sect = clst2sect(fs, fp->clust);    /* Get current sector */
            if (sect == 0) ABORT(fs, FR_INT_ERR);
            sect += csect;
            cc = btw / SS(fs);                /* When remaining bytes >= sector size, */
            if (cc > 0) {                    /* Write maximum contiguous sectors directly */
                if (csect + cc > fs->csize) {    /* Clip at cluster boundary */
                    cc = fs->csize - csect;
                }
                if (disk_write(fs->pdrv, wbuff, sect, cc) != RES_OK) ABORT(fs, FR_DISK_ERR);
#if FF_FS_MINIMIZE <= 2
#if FF_FS_TINY
                if (fs->winsect - sect < cc) {    /* Refill sector cache if it gets invalidated by the direct write */
                    MEMCPY(fs->win, wbuff + ((fs->winsect - sect) * SS(fs)), SS(fs));
                    fs->wflag = 0;
                }
#else
                if (fp->sect - sect < cc) { /* Refill sector cache if it gets invalidated by the direct write */
                    MEMCPY(fp->buf, wbuff + ((fp->sect - sect) * SS(fs)), SS(fs));
                    fp->flag &= (BYTE)~FA_DIRTY;
                }
#endif
#endif
                wcnt = SS(fs) * cc;        /* Number of bytes transferred */
                continue;
            }
#if FF_FS_TINY
            if (fp->fptr >= fp->obj.objsize) {    /* Avoid silly cache filling on the growing edge */
                if (sync_window(fs) != FR_OK) ABORT(fs, FR_DISK_ERR);
                fs->winsect = sect;
            }
#else
            if (fp->sect != sect &&         /* Fill sector cache with file data */
                fp->fptr < fp->obj.objsize &&
                disk_read(fs->pdrv, fp->buf, sect, 1) != RES_OK) {
                    ABORT(fs, FR_DISK_ERR);
            }
#endif
            fp->sect = sect;
        }
        wcnt = SS(fs) - (UINT)fp->fptr % SS(fs);    /* Number of bytes left in the sector */
        if (wcnt > btw) wcnt = btw;                    /* Clip it by btw if needed */
#if FF_FS_TINY
        if (move_window(fs, fp->sect) != FR_OK) ABORT(fs, FR_DISK_ERR);    /* Move sector window */
        MEMCPY(fs->win + fp->fptr % SS(fs), wbuff, wcnt);    /* Fit data to the sector */
        fs->wflag = 1;
#else
        MEMCPY(fp->buf + fp->fptr % SS(fs), wbuff, wcnt);    /* Fit data to the sector */
        fp->flag |= FA_DIRTY;
#endif
    }

    fp->flag |= FA_MODIFIED;                /* Set file change flag */

    LEAVE_FF(fs, FR_OK);
}

#endif /* !FF_FS_READONLY */

