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
/* Change Attribute                                                      */
/*-----------------------------------------------------------------------*/
#if FF_USE_CHMOD && !FF_FS_READONLY

FRESULT f_chmod (
    const TCHAR* path,    /* Pointer to the file path */
    BYTE attr,            /* Attribute bits */
    BYTE mask            /* Attribute mask to change */
)
{
    FRESULT res;
    DIR dj;
    FATFS *fs;
    DEF_NAMBUF


    res = find_volume(&path, &fs, FA_WRITE);    /* Get logical drive */
    if (res == FR_OK) {
        dj.obj.fs = fs;
        INIT_NAMBUF(fs);
        res = follow_path(&dj, path);    /* Follow the file path */
        if (res == FR_OK && (dj.fn[NSFLAG] & (NS_DOT | NS_NONAME))) res = FR_INVALID_NAME;    /* Check object validity */
        if (res == FR_OK) {
            mask &= AM_RDO|AM_HID|AM_SYS|AM_ARC;    /* Valid attribute mask */
#if FF_FS_EXFAT
            if (fs->fs_type == FS_EXFAT) {
                fs->dirbuf[XDIR_Attr] = (attr & mask) | (fs->dirbuf[XDIR_Attr] & (BYTE)~mask);    /* Apply attribute change */
                res = store_xdir(&dj);
            } else
#endif
            {
                dj.dir[DIR_Attr] = (attr & mask) | (dj.dir[DIR_Attr] & (BYTE)~mask);    /* Apply attribute change */
                fs->wflag = 1;
            }
            if (res == FR_OK) {
                res = sync_fs(fs);
            }
        }
        FREE_NAMBUF();
    }

    LEAVE_FF(fs, res);
}

#endif    /* FF_USE_CHMOD && !FF_FS_READONLY */

