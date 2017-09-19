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
/* Seek File Read/Write Pointer                                          */
/*-----------------------------------------------------------------------*/
#if FF_FS_MINIMIZE <= 2

FRESULT f_lseek (
    FIL* fp,        /* Pointer to the file object */
    FSIZE_t ofs        /* File pointer from top of file */
)
{
    FRESULT res;
    FATFS *fs;
    DWORD clst, bcs, nsect;
    FSIZE_t ifptr;
#if FF_USE_FASTSEEK
    DWORD cl, pcl, ncl, tcl, dsc, tlen, ulen, *tbl;
#endif

    res = validate(&fp->obj, &fs);        /* Check validity of the file object */
    if (res == FR_OK) res = (FRESULT)fp->err;
#if FF_FS_EXFAT && !FF_FS_READONLY
    if (res == FR_OK && fs->fs_type == FS_EXFAT) {
        res = fill_last_frag(&fp->obj, fp->clust, 0xFFFFFFFF);    /* Fill last fragment on the FAT if needed */
    }
#endif
    if (res != FR_OK) LEAVE_FF(fs, res);

#if FF_USE_FASTSEEK
    if (fp->cltbl) {    /* Fast seek */
        if (ofs == CREATE_LINKMAP) {    /* Create CLMT */
            tbl = fp->cltbl;
            tlen = *tbl++; ulen = 2;    /* Given table size and required table size */
            cl = fp->obj.sclust;        /* Origin of the chain */
            if (cl != 0) {
                do {
                    /* Get a fragment */
                    tcl = cl; ncl = 0; ulen += 2;    /* Top, length and used items */
                    do {
                        pcl = cl; ncl++;
                        cl = get_fat(&fp->obj, cl);
                        if (cl <= 1) ABORT(fs, FR_INT_ERR);
                        if (cl == 0xFFFFFFFF) ABORT(fs, FR_DISK_ERR);
                    } while (cl == pcl + 1);
                    if (ulen <= tlen) {        /* Store the length and top of the fragment */
                        *tbl++ = ncl; *tbl++ = tcl;
                    }
                } while (cl < fs->n_fatent);    /* Repeat until end of chain */
            }
            *fp->cltbl = ulen;    /* Number of items used */
            if (ulen <= tlen) {
                *tbl = 0;        /* Terminate table */
            } else {
                res = FR_NOT_ENOUGH_CORE;    /* Given table size is smaller than required */
            }
        } else {                        /* Fast seek */
            if (ofs > fp->obj.objsize) ofs = fp->obj.objsize;    /* Clip offset at the file size */
            fp->fptr = ofs;                /* Set file pointer */
            if (ofs > 0) {
                fp->clust = clmt_clust(fp, ofs - 1);
                dsc = clst2sect(fs, fp->clust);
                if (dsc == 0) ABORT(fs, FR_INT_ERR);
                dsc += (DWORD)((ofs - 1) / SS(fs)) & (fs->csize - 1);
                if (fp->fptr % SS(fs) && dsc != fp->sect) {    /* Refill sector cache if needed */
#if !FF_FS_TINY
#if !FF_FS_READONLY
                    if (fp->flag & FA_DIRTY) {        /* Write-back dirty sector cache */
                        if (disk_write(fs->pdrv, fp->buf, fp->sect, 1) != RES_OK) ABORT(fs, FR_DISK_ERR);
                        fp->flag &= (BYTE)~FA_DIRTY;
                    }
#endif
                    if (disk_read(fs->pdrv, fp->buf, dsc, 1) != RES_OK) ABORT(fs, FR_DISK_ERR);    /* Load current sector */
#endif
                    fp->sect = dsc;
                }
            }
        }
    } else
#endif

    /* Normal Seek */
    {
#if FF_FS_EXFAT
        if (fs->fs_type != FS_EXFAT && ofs >= 0x100000000) ofs = 0xFFFFFFFF;    /* Clip at 4 GiB - 1 if at FATxx */
#endif
        if (ofs > fp->obj.objsize && (FF_FS_READONLY || !(fp->flag & FA_WRITE))) {    /* In read-only mode, clip offset with the file size */
            ofs = fp->obj.objsize;
        }
        ifptr = fp->fptr;
        fp->fptr = nsect = 0;
        if (ofs > 0) {
            bcs = (DWORD)fs->csize * SS(fs);    /* Cluster size (byte) */
            if (ifptr > 0 &&
                (ofs - 1) / bcs >= (ifptr - 1) / bcs) {    /* When seek to same or following cluster, */
                fp->fptr = (ifptr - 1) & ~(FSIZE_t)(bcs - 1);    /* start from the current cluster */
                ofs -= fp->fptr;
                clst = fp->clust;
            } else {                                    /* When seek to back cluster, */
                clst = fp->obj.sclust;                    /* start from the first cluster */
#if !FF_FS_READONLY
                if (clst == 0) {                        /* If no cluster chain, create a new chain */
                    clst = create_chain(&fp->obj, 0);
                    if (clst == 1) ABORT(fs, FR_INT_ERR);
                    if (clst == 0xFFFFFFFF) ABORT(fs, FR_DISK_ERR);
                    fp->obj.sclust = clst;
                }
#endif
                fp->clust = clst;
            }
            if (clst != 0) {
                while (ofs > bcs) {                        /* Cluster following loop */
                    ofs -= bcs; fp->fptr += bcs;
#if !FF_FS_READONLY
                    if (fp->flag & FA_WRITE) {            /* Check if in write mode or not */
                        if (FF_FS_EXFAT && fp->fptr > fp->obj.objsize) {    /* No FAT chain object needs correct objsize to generate FAT value */
                            fp->obj.objsize = fp->fptr;
                            fp->flag |= FA_MODIFIED;
                        }
                        clst = create_chain(&fp->obj, clst);    /* Follow chain with forceed stretch */
                        if (clst == 0) {                /* Clip file size in case of disk full */
                            ofs = 0; break;
                        }
                    } else
#endif
                    {
                        clst = get_fat(&fp->obj, clst);    /* Follow cluster chain if not in write mode */
                    }
                    if (clst == 0xFFFFFFFF) ABORT(fs, FR_DISK_ERR);
                    if (clst <= 1 || clst >= fs->n_fatent) ABORT(fs, FR_INT_ERR);
                    fp->clust = clst;
                }
                fp->fptr += ofs;
                if (ofs % SS(fs)) {
                    nsect = clst2sect(fs, clst);    /* Current sector */
                    if (nsect == 0) ABORT(fs, FR_INT_ERR);
                    nsect += (DWORD)(ofs / SS(fs));
                }
            }
        }
        if (!FF_FS_READONLY && fp->fptr > fp->obj.objsize) {    /* Set file change flag if the file size is extended */
            fp->obj.objsize = fp->fptr;
            fp->flag |= FA_MODIFIED;
        }
        if (fp->fptr % SS(fs) && nsect != fp->sect) {    /* Fill sector cache if needed */
#if !FF_FS_TINY
#if !FF_FS_READONLY
            if (fp->flag & FA_DIRTY) {            /* Write-back dirty sector cache */
                if (disk_write(fs->pdrv, fp->buf, fp->sect, 1) != RES_OK) ABORT(fs, FR_DISK_ERR);
                fp->flag &= (BYTE)~FA_DIRTY;
            }
#endif
            if (disk_read(fs->pdrv, fp->buf, nsect, 1) != RES_OK) ABORT(fs, FR_DISK_ERR);    /* Fill sector cache */
#endif
            fp->sect = nsect;
        }
    }

    LEAVE_FF(fs, res);
}

#endif /* FF_FS_MINIMIZE <= 2 */

