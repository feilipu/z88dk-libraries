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
/* Open or Create a File                                                 */
/*-----------------------------------------------------------------------*/

FRESULT f_open (
    FIL* fp,            /* Pointer to the blank file object */
    const TCHAR* path,    /* Pointer to the file name */
    BYTE mode            /* Access mode and file open mode flags */
)
{
    FRESULT res;
    DIR dj;
    FATFS *fs;
#if !FF_FS_READONLY
    DWORD dw, cl, bcs, clst, sc;
    FSIZE_t ofs;
#endif
    DEF_NAMBUF


    if (!fp) return FR_INVALID_OBJECT;

    /* Get logical drive */
    mode &= FF_FS_READONLY ? FA_READ : FA_READ | FA_WRITE | FA_CREATE_ALWAYS | FA_CREATE_NEW | FA_OPEN_ALWAYS | FA_OPEN_APPEND | FA_SEEKEND;
    res = find_volume(&path, &fs, mode);
    if (res == FR_OK) {
        dj.obj.fs = fs;
        INIT_NAMBUF(fs);
        res = follow_path(&dj, path);    /* Follow the file path */
#if !FF_FS_READONLY    /* Read/Write configuration */
        if (res == FR_OK) {
            if (dj.fn[NSFLAG] & NS_NONAME) {    /* Origin directory itself? */
                res = FR_INVALID_NAME;
            }
#if FF_FS_LOCK != 0
            else {
                res = chk_lock(&dj, (mode & ~FA_READ) ? 1 : 0);        /* Check if the file can be used */
            }
#endif
        }
        /* Create or Open a file */
        if (mode & (FA_CREATE_ALWAYS | FA_OPEN_ALWAYS | FA_CREATE_NEW)) {
            if (res != FR_OK) {                    /* No file, create new */
                if (res == FR_NO_FILE) {        /* There is no file to open, create a new entry */
#if FF_FS_LOCK != 0
                    res = enq_lock() ? dir_register(&dj) : FR_TOO_MANY_OPEN_FILES;
#else
                    res = dir_register(&dj);
#endif
                }
                mode |= FA_CREATE_ALWAYS;        /* File is created */
            }
            else {                                /* Any object with the same name is already existing */
                if (dj.obj.attr & (AM_RDO | AM_DIR)) {    /* Cannot overwrite it (R/O or DIR) */
                    res = FR_DENIED;
                } else {
                    if (mode & FA_CREATE_NEW) res = FR_EXIST;    /* Cannot create as new file */
                }
            }
            if (res == FR_OK && (mode & FA_CREATE_ALWAYS)) {    /* Truncate the file if overwrite mode */
#if FF_FS_EXFAT
                if (fs->fs_type == FS_EXFAT) {
                    /* Get current allocation info */
                    fp->obj.fs = fs;
                    fp->obj.sclust = cl = ld_dword(fs->dirbuf + XDIR_FstClus);
                    fp->obj.objsize = ld_qword(fs->dirbuf + XDIR_FileSize);
                    fp->obj.stat = fs->dirbuf[XDIR_GenFlags] & 2;
                    fp->obj.n_frag = 0;
                    /* Set directory entry block initial state */
                    MEMSET(fs->dirbuf + 2, 0, 30);        /* Clear 85 entry except for NumSec */
                    MEMSET(fs->dirbuf + 38, 0, 26);    /* Clear C0 entry except for NumName and NameHash */
                    fs->dirbuf[XDIR_Attr] = AM_ARC;
                    st_dword(fs->dirbuf + XDIR_CrtTime, GET_FATTIME());
                    fs->dirbuf[XDIR_GenFlags] = 1;
                    res = store_xdir(&dj);
                    if (res == FR_OK && cl != 0) {        /* Remove the cluster chain if exist */
                        res = remove_chain(&fp->obj, cl, 0);
                        fs->last_clst = cl - 1;            /* Reuse the cluster hole */
                    }
                } else
#endif
                {
                    /* Set directory entry initial state */
                    cl = ld_clust(fs, dj.dir);            /* Get current cluster chain */
                    st_dword(dj.dir + DIR_CrtTime, GET_FATTIME());    /* Set created time */
                    dj.dir[DIR_Attr] = AM_ARC;            /* Reset attribute */
                    st_clust(fs, dj.dir, 0);            /* Reset file allocation info */
                    st_dword(dj.dir + DIR_FileSize, 0);
                    fs->wflag = 1;
                    if (cl != 0) {                        /* Remove the cluster chain if exist */
                        dw = fs->winsect;
                        res = remove_chain(&dj.obj, cl, 0);
                        if (res == FR_OK) {
                            res = move_window(fs, dw);
                            fs->last_clst = cl - 1;        /* Reuse the cluster hole */
                        }
                    }
                }
            }
        }
        else {    /* Open an existing file */
            if (res == FR_OK) {                    /* Is the object exsiting? */
                if (dj.obj.attr & AM_DIR) {        /* File open against a directory */
                    res = FR_NO_FILE;
                } else {
                    if ((mode & FA_WRITE) && (dj.obj.attr & AM_RDO)) { /* Write mode open against R/O file */
                        res = FR_DENIED;
                    }
                }
            }
        }
        if (res == FR_OK) {
            if (mode & FA_CREATE_ALWAYS) mode |= FA_MODIFIED;    /* Set file change flag if created or overwritten */
            fp->dir_sect = fs->winsect;            /* Pointer to the directory entry */
            fp->dir_ptr = dj.dir;
#if FF_FS_LOCK != 0
            fp->obj.lockid = inc_lock(&dj, (mode & ~FA_READ) ? 1 : 0);    /* Lock the file for this session */
            if (!fp->obj.lockid) res = FR_INT_ERR;
#endif
        }
#else        /* R/O configuration */
        if (res == FR_OK) {
            if (dj.fn[NSFLAG] & NS_NONAME) {    /* Is it origin directory itself? */
                res = FR_INVALID_NAME;
            } else {
                if (dj.obj.attr & AM_DIR) {        /* Is it a directory? */
                    res = FR_NO_FILE;
                }
            }
        }
#endif

        if (res == FR_OK) {
#if FF_FS_EXFAT
            if (fs->fs_type == FS_EXFAT) {
                fp->obj.c_scl = dj.obj.sclust;                            /* Get containing directory info */
                fp->obj.c_size = ((DWORD)dj.obj.objsize & 0xFFFFFF00) | dj.obj.stat;
                fp->obj.c_ofs = dj.blk_ofs;
                fp->obj.sclust = ld_dword(fs->dirbuf + XDIR_FstClus);    /* Get object allocation info */
                fp->obj.objsize = ld_qword(fs->dirbuf + XDIR_FileSize);
                fp->obj.stat = fs->dirbuf[XDIR_GenFlags] & 2;
                fp->obj.n_frag = 0;
            } else
#endif
            {
                fp->obj.sclust = ld_clust(fs, dj.dir);                    /* Get object allocation info */
                fp->obj.objsize = ld_dword(dj.dir + DIR_FileSize);
            }
#if FF_USE_FASTSEEK
            fp->cltbl = 0;            /* Disable fast seek mode */
#endif
            fp->obj.fs = fs;         /* Validate the file object */
            fp->obj.id = fs->id;
            fp->flag = mode;        /* Set file access mode */
            fp->err = 0;            /* Clear error flag */
            fp->sect = 0;            /* Invalidate current data sector */
            fp->fptr = 0;            /* Set file pointer top of the file */
#if !FF_FS_READONLY
#if !FF_FS_TINY
            MEMSET(fp->buf, 0, FF_MAX_SS);    /* Clear sector buffer */
#endif
            if ((mode & FA_SEEKEND) && fp->obj.objsize > 0) {    /* Seek to end of file if FA_OPEN_APPEND is specified */
                fp->fptr = fp->obj.objsize;            /* Offset to seek */
                bcs = (DWORD)fs->csize * SS(fs);    /* Cluster size in byte */
                clst = fp->obj.sclust;                /* Follow the cluster chain */
                for (ofs = fp->obj.objsize; res == FR_OK && ofs > bcs; ofs -= bcs) {
                    clst = get_fat(&fp->obj, clst);
                    if (clst <= 1) res = FR_INT_ERR;
                    if (clst == 0xFFFFFFFF) res = FR_DISK_ERR;
                }
                fp->clust = clst;
                if (res == FR_OK && ofs % SS(fs)) {    /* Fill sector buffer if not on the sector boundary */
                    if ((sc = clst2sect(fs, clst)) == 0) {
                        res = FR_INT_ERR;
                    } else {
                        fp->sect = sc + (DWORD)(ofs / SS(fs));
#if !FF_FS_TINY
                        if (disk_read(fs->pdrv, fp->buf, fp->sect, 1) != RES_OK) res = FR_DISK_ERR;
#endif
                    }
                }
            }
#endif
        }

        FREE_NAMBUF();
    }

    if (res != FR_OK) fp->obj.fs = 0;    /* Invalidate file object on error */

    LEAVE_FF(fs, res);
}

