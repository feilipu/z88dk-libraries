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
/* Create a Directory Object                                             */
/*-----------------------------------------------------------------------*/
#if FF_FS_MINIMIZE <= 1

FRESULT f_opendir (
    DIR* dp,            /* Pointer to directory object to create */
    const TCHAR* path    /* Pointer to the directory path */
)
{
    FRESULT res;
    FATFS *fs;
    DEF_NAMBUF


    if (!dp) return FR_INVALID_OBJECT;

    /* Get logical drive */
    res = find_volume(&path, &fs, 0);
    if (res == FR_OK) {
        dp->obj.fs = fs;
        INIT_NAMBUF(fs);
        res = follow_path(dp, path);            /* Follow the path to the directory */
        if (res == FR_OK) {                        /* Follow completed */
            if (!(dp->fn[NSFLAG] & NS_NONAME)) {    /* It is not the origin directory itself */
                if (dp->obj.attr & AM_DIR) {        /* This object is a sub-directory */
#if FF_FS_EXFAT
                    if (fs->fs_type == FS_EXFAT) {
                        dp->obj.c_scl = dp->obj.sclust;                            /* Get containing directory inforamation */
                        dp->obj.c_size = ((DWORD)dp->obj.objsize & 0xFFFFFF00) | dp->obj.stat;
                        dp->obj.c_ofs = dp->blk_ofs;
                        dp->obj.sclust = ld_dword(fs->dirbuf + XDIR_FstClus);    /* Get object allocation info */
                        dp->obj.objsize = ld_qword(fs->dirbuf + XDIR_FileSize);
                        dp->obj.stat = fs->dirbuf[XDIR_GenFlags] & 2;
                    } else
#endif
                    {
                        dp->obj.sclust = ld_clust(fs, dp->dir);    /* Get object allocation info */
                    }
                } else {                        /* This object is a file */
                    res = FR_NO_PATH;
                }
            }
            if (res == FR_OK) {
                dp->obj.id = fs->id;
                res = dir_sdi(dp, 0);            /* Rewind directory */
#if FF_FS_LOCK != 0
                if (res == FR_OK) {
                    if (dp->obj.sclust != 0) {
                        dp->obj.lockid = inc_lock(dp, 0);    /* Lock the sub directory */
                        if (!dp->obj.lockid) res = FR_TOO_MANY_OPEN_FILES;
                    } else {
                        dp->obj.lockid = 0;    /* Root directory need not to be locked */
                    }
                }
#endif
            }
        }
        FREE_NAMBUF();
        if (res == FR_NO_FILE) res = FR_NO_PATH;
    }
    if (res != FR_OK) dp->obj.fs = 0;        /* Invalidate the directory object if function faild */

    LEAVE_FF(fs, res);
}

#endif /* FF_FS_MINIMIZE <= 1 */

