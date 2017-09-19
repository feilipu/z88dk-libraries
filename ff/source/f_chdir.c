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
/* Change Current Directory or Current Drive, Get Current Directory      */
/*-----------------------------------------------------------------------*/
#if FF_FS_RPATH >= 1

FRESULT f_chdir (
    const TCHAR* path    /* Pointer to the directory path */
)
{
    FRESULT res;
    DIR dj;
    FATFS *fs;
    DEF_NAMBUF

    /* Get logical drive */
    res = find_volume(&path, &fs, 0);
    if (res == FR_OK) {
        dj.obj.fs = fs;
        INIT_NAMBUF(fs);
        res = follow_path(&dj, path);        /* Follow the path */
        if (res == FR_OK) {                    /* Follow completed */
            if (dj.fn[NSFLAG] & NS_NONAME) {
                fs->cdir = dj.obj.sclust;    /* It is the start directory itself */
#if FF_FS_EXFAT
                if (fs->fs_type == FS_EXFAT) {
                    fs->cdc_scl = dj.obj.c_scl;
                    fs->cdc_size = dj.obj.c_size;
                    fs->cdc_ofs = dj.obj.c_ofs;
                }
#endif
            } else {
                if (dj.obj.attr & AM_DIR) {    /* It is a sub-directory */
#if FF_FS_EXFAT
                    if (fs->fs_type == FS_EXFAT) {
                        fs->cdir = ld_dword(fs->dirbuf + XDIR_FstClus);        /* Sub-directory cluster */
                        fs->cdc_scl = dj.obj.sclust;                        /* Save containing directory information */
                        fs->cdc_size = ((DWORD)dj.obj.objsize & 0xFFFFFF00) | dj.obj.stat;
                        fs->cdc_ofs = dj.blk_ofs;
                    } else
#endif
                    {
                        fs->cdir = ld_clust(fs, dj.dir);                    /* Sub-directory cluster */
                    }
                } else {
                    res = FR_NO_PATH;        /* Reached but a file */
                }
            }
        }
        FREE_NAMBUF();
        if (res == FR_NO_FILE) res = FR_NO_PATH;
    }

    LEAVE_FF(fs, res);
}

#endif /* FF_FS_RPATH >= 1 */

