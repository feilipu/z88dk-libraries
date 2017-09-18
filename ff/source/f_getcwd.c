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
#if FF_FS_RPATH >= 2

FRESULT f_getcwd (
    TCHAR* buff,    /* Pointer to the directory path */
    UINT len        /* Size of path */
)
{
    FRESULT res;
    DIR dj;
    FATFS *fs;
    UINT i, n;
    DWORD ccl;
    TCHAR *tp;
    FILINFO fno;
    DEF_NAMBUF


    *buff = 0;
    /* Get logical drive */
    res = find_volume((const TCHAR**)&buff, &fs, 0);    /* Get current volume */
    if (res == FR_OK) {
        dj.obj.fs = fs;
        INIT_NAMBUF(fs);
        i = len;            /* Bottom of buffer (directory stack base) */
        if (!FF_FS_EXFAT || fs->fs_type != FS_EXFAT) {    /* (Cannot do getcwd on exFAT and returns root path) */
            dj.obj.sclust = fs->cdir;                /* Start to follow upper directory from current directory */
            while ((ccl = dj.obj.sclust) != 0) {    /* Repeat while current directory is a sub-directory */
                res = dir_sdi(&dj, 1 * SZDIRE);    /* Get parent directory */
                if (res != FR_OK) break;
                res = move_window(fs, dj.sect);
                if (res != FR_OK) break;
                dj.obj.sclust = ld_clust(fs, dj.dir);    /* Goto parent directory */
                res = dir_sdi(&dj, 0);
                if (res != FR_OK) break;
                do {                            /* Find the entry links to the child directory */
                    res = dir_read(&dj, 0);
                    if (res != FR_OK) break;
                    if (ccl == ld_clust(fs, dj.dir)) break;    /* Found the entry */
                    res = dir_next(&dj, 0);
                } while (res == FR_OK);
                if (res == FR_NO_FILE) res = FR_INT_ERR;/* It cannot be 'not found'. */
                if (res != FR_OK) break;
                get_fileinfo(&dj, &fno);        /* Get the directory name and push it to the buffer */
                for (n = 0; fno.fname[n]; n++) ;
                if (i < n + 3) {
                    res = FR_NOT_ENOUGH_CORE; break;
                }
                while (n) buff[--i] = fno.fname[--n];
                buff[--i] = '/';
            }
        }
        tp = buff;
        if (res == FR_OK) {
#if FF_VOLUMES >= 2
            *tp++ = '0' + CurrVol;            /* Put drive number */
            *tp++ = ':';
#endif
            if (i == len) {                    /* Root-directory */
                *tp++ = '/';
            } else {                        /* Sub-directroy */
                do        /* Add stacked path str */
                    *tp++ = buff[i++];
                while (i < len);
            }
        }
        *tp = 0;
        FREE_NAMBUF();
    }

    LEAVE_FF(fs, res);
}

#endif /* FF_FS_RPATH >= 2 */

