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


/*-----------------------------------------------------------------------*/
/* File/Volume controls                                                  */
/*-----------------------------------------------------------------------*/

#if FF_VOLUMES < 1 || FF_VOLUMES > 10
#error Wrong FF_VOLUMES setting
#endif
extern FATFS *FatFs[];      /* Pointer to the filesystem objects (logical drives) */
extern WORD Fsid;		    /* File system mount ID */

#if FF_FS_RPATH != 0 && FF_VOLUMES >= 2
extern BYTE CurrVol;	    /* Current drive */
#endif

#if FF_FS_LOCK != 0
extern FILESEM Files[];     /* Open object lock semaphores */
#endif


/*-----------------------------------------------------------------------*/
/* LFN/Directory working buffer                                          */
/*-----------------------------------------------------------------------*/

#if FF_USE_LFN == 1         /* LFN enabled with static working buffer */
#if FF_FS_EXFAT
extern BYTE DirBuf[];       /* Directory entry block scratchpad buffer */
#endif
extern WCHAR LfnBuf[];      /* LFN working buffer */
#endif


/*------------------------------------------------------------------------*/
/* Code Conversion Tables                                                 */
/*------------------------------------------------------------------------*/

#if FF_USE_LFN != 0                         /* LFN configurations */

#if FF_CODE_PAGE == 0       /* Run-time code page configuration */
#define CODEPAGE CodePage
extern WORD CodePage;       /* Current code page */
extern const BYTE *ExCvt, *DbcTbl;    /* Pointer to current SBCS up-case table and DBCS code range table below */
extern const BYTE Ct437[];
extern const BYTE Ct720[];
extern const BYTE Ct737[];
extern const BYTE Ct771[];
extern const BYTE Ct775[];
extern const BYTE Ct850[];
extern const BYTE Ct852[];
extern const BYTE Ct855[];
extern const BYTE Ct857[];
extern const BYTE Ct860[];
extern const BYTE Ct861[];
extern const BYTE Ct862[];
extern const BYTE Ct863[];
extern const BYTE Ct864[];
extern const BYTE Ct865[];
extern const BYTE Ct866[];
extern const BYTE Ct869[];
extern const BYTE Dc932[];
extern const BYTE Dc936[];
extern const BYTE Dc949[];
extern const BYTE Dc950[];

extern const WORD cp_code[];
extern const WCHAR *const cp_table[];

#elif FF_CODE_PAGE < 900    /* static code page configuration (SBCS) */
#define CODEPAGE FF_CODE_PAGE
extern const BYTE ExCvt[];

#else                       /* static code page configuration (DBCS) */
#define CODEPAGE FF_CODE_PAGE
extern const BYTE DbcTbl[];

#endif
#endif


/*-----------------------------------------------------------------------*/
/* Delete a File/Directory                                               */
/*-----------------------------------------------------------------------*/
#if FF_FS_MINIMIZE == 0
#if !FF_FS_READONLY


FRESULT f_unlink (
    const TCHAR* path        /* Pointer to the file or directory path */
)
{
    FRESULT res;
    DIR dj, sdj;
    DWORD dclst = 0;
    FATFS *fs;
#if FF_FS_EXFAT
    FFOBJID obj;
#endif
    DEF_NAMBUF


    /* Get logical drive */
    res = find_volume(&path, &fs, FA_WRITE);
    if (res == FR_OK) {
        dj.obj.fs = fs;
        INIT_NAMBUF(fs);
        res = follow_path(&dj, path);        /* Follow the file path */
        if (FF_FS_RPATH && res == FR_OK && (dj.fn[NSFLAG] & NS_DOT)) {
            res = FR_INVALID_NAME;            /* Cannot remove dot entry */
        }
#if FF_FS_LOCK != 0
        if (res == FR_OK) res = chk_lock(&dj, 2);    /* Check if it is an open object */
#endif
        if (res == FR_OK) {                    /* The object is accessible */
            if (dj.fn[NSFLAG] & NS_NONAME) {
                res = FR_INVALID_NAME;        /* Cannot remove the origin directory */
            } else {
                if (dj.obj.attr & AM_RDO) {
                    res = FR_DENIED;        /* Cannot remove R/O object */
                }
            }
            if (res == FR_OK) {
#if FF_FS_EXFAT
                obj.fs = fs;
                if (fs->fs_type == FS_EXFAT) {
                    obj.sclust = dclst = ld_dword(fs->dirbuf + XDIR_FstClus);
                    obj.objsize = ld_qword(fs->dirbuf + XDIR_FileSize);
                    obj.stat = fs->dirbuf[XDIR_GenFlags] & 2;
                } else
#endif
                {
                    dclst = ld_clust(fs, dj.dir);
                }
                if (dj.obj.attr & AM_DIR) {            /* Is it a sub-directory? */
#if FF_FS_RPATH != 0
                    if (dclst == fs->cdir) {             /* Is it the current directory? */
                        res = FR_DENIED;
                    } else
#endif
                    {
                        sdj.obj.fs = fs;                /* Open the sub-directory */
                        sdj.obj.sclust = dclst;
#if FF_FS_EXFAT
                        if (fs->fs_type == FS_EXFAT) {
                            sdj.obj.objsize = obj.objsize;
                            sdj.obj.stat = obj.stat;
                        }
#endif
                        res = dir_sdi(&sdj, 0);
                        if (res == FR_OK) {
                            res = dir_read(&sdj, 0);            /* Read an item */
                            if (res == FR_OK) res = FR_DENIED;    /* Not empty? */
                            if (res == FR_NO_FILE) res = FR_OK;    /* Empty? */
                        }
                    }
                }
            }
            if (res == FR_OK) {
                res = dir_remove(&dj);            /* Remove the directory entry */
                if (res == FR_OK && dclst) {    /* Remove the cluster chain if exist */
#if FF_FS_EXFAT
                    res = remove_chain(&obj, dclst, 0);
#else
                    res = remove_chain(&dj.obj, dclst, 0);
#endif
                }
                if (res == FR_OK) res = sync_fs(fs);
            }
        }
        FREE_NAMBUF();
    }

    LEAVE_FF(fs, res);
}

#endif /* !FF_FS_READONLY */
#endif /* FF_FS_MINIMIZE == 0 */

