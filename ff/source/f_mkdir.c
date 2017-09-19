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
/* Create a Directory                                                    */
/*-----------------------------------------------------------------------*/
#if FF_FS_MINIMIZE == 0
#if !FF_FS_READONLY

FRESULT f_mkdir (
    const TCHAR* path        /* Pointer to the directory path */
)
{
    FRESULT res;
    DIR dj;
    FATFS *fs;
    BYTE *dir;
    DWORD dcl, pcl, tm;
    DEF_NAMBUF


    /* Get logical drive */
    res = find_volume(&path, &fs, FA_WRITE);
    if (res == FR_OK) {
        dj.obj.fs = fs;
        INIT_NAMBUF(fs);
        res = follow_path(&dj, path);            /* Follow the file path */
        if (res == FR_OK) res = FR_EXIST;        /* Any object with same name is already existing */
        if (FF_FS_RPATH && res == FR_NO_FILE && (dj.fn[NSFLAG] & NS_DOT)) {
            res = FR_INVALID_NAME;
        }
        if (res == FR_NO_FILE) {                /* Can create a new directory */
            dcl = create_chain(&dj.obj, 0);        /* Allocate a cluster for the new directory table */
            dj.obj.objsize = (DWORD)fs->csize * SS(fs);
            res = FR_OK;
            if (dcl == 0) res = FR_DENIED;        /* No space to allocate a new cluster */
            if (dcl == 1) res = FR_INT_ERR;
            if (dcl == 0xFFFFFFFF) res = FR_DISK_ERR;
            if (res == FR_OK) res = sync_window(fs);    /* Flush FAT */
            tm = GET_FATTIME();
            if (res == FR_OK) {                    /* Initialize the new directory table */
                res = dir_clear(fs, dcl);        /* Clean up the new table */
                if (res == FR_OK && (!FF_FS_EXFAT || fs->fs_type != FS_EXFAT)) {    /* Create dot entries (FAT only) */
                    dir = fs->win;
                    MEMSET(dir + DIR_Name, ' ', 11);    /* Create "." entry */
                    dir[DIR_Name] = '.';
                    dir[DIR_Attr] = AM_DIR;
                    st_dword(dir + DIR_ModTime, tm);
                    st_clust(fs, dir, dcl);
                    MEMCPY(dir + SZDIRE, dir, SZDIRE); /* Create ".." entry */
                    dir[SZDIRE + 1] = '.'; pcl = dj.obj.sclust;
                    st_clust(fs, dir + SZDIRE, pcl);
                    fs->wflag = 1;
                }
            }
            if (res == FR_OK) {
                res = dir_register(&dj);    /* Register the object to the directoy */
            }
            if (res == FR_OK) {
#if FF_FS_EXFAT
                if (fs->fs_type == FS_EXFAT) {    /* Initialize directory entry block */
                    st_dword(fs->dirbuf + XDIR_ModTime, tm);    /* Created time */
                    st_dword(fs->dirbuf + XDIR_FstClus, dcl);    /* Table start cluster */
                    st_dword(fs->dirbuf + XDIR_FileSize, (DWORD)dj.obj.objsize);    /* File size needs to be valid */
                    st_dword(fs->dirbuf + XDIR_ValidFileSize, (DWORD)dj.obj.objsize);
                    fs->dirbuf[XDIR_GenFlags] = 3;                /* Initialize the object flag */
                    fs->dirbuf[XDIR_Attr] = AM_DIR;                /* Attribute */
                    res = store_xdir(&dj);
                } else
#endif
                {
                    dir = dj.dir;
                    st_dword(dir + DIR_ModTime, tm);    /* Created time */
                    st_clust(fs, dir, dcl);                /* Table start cluster */
                    dir[DIR_Attr] = AM_DIR;                /* Attribute */
                    fs->wflag = 1;
                }
                if (res == FR_OK) {
                    res = sync_fs(fs);
                }
            } else {
                remove_chain(&dj.obj, dcl, 0);        /* Could not register, remove cluster chain */
            }
        }
        FREE_NAMBUF();
    }

    LEAVE_FF(fs, res);
}

#endif /* !FF_FS_READONLY */
#endif /* FF_FS_MINIMIZE == 0 */

