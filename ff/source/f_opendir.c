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

