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

