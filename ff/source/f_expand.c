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
/* Allocate a Contiguous Blocks to the File                              */
/*-----------------------------------------------------------------------*/
#if FF_USE_EXPAND && !FF_FS_READONLY

FRESULT f_expand (
    FIL* fp,        /* Pointer to the file object */
    FSIZE_t fsz,    /* File size to be expanded to */
    BYTE opt        /* Operation mode 0:Find and prepare or 1:Find and allocate */
)
{
    FRESULT res;
    FATFS *fs;
    DWORD n, clst, stcl, scl, ncl, tcl, lclst;


    res = validate(&fp->obj, &fs);        /* Check validity of the file object */
    if (res != FR_OK || (res = (FRESULT)fp->err) != FR_OK) LEAVE_FF(fs, res);
    if (fsz == 0 || fp->obj.objsize != 0 || !(fp->flag & FA_WRITE)) LEAVE_FF(fs, FR_DENIED);
#if FF_FS_EXFAT
    if (fs->fs_type != FS_EXFAT && fsz >= 0x100000000) LEAVE_FF(fs, FR_DENIED);    /* Check if in size limit */
#endif
    n = (DWORD)fs->csize * SS(fs);    /* Cluster size */
    tcl = (DWORD)(fsz / n) + ((fsz & (n - 1)) ? 1 : 0);    /* Number of clusters required */
    stcl = fs->last_clst; lclst = 0;
    if (stcl < 2 || stcl >= fs->n_fatent) stcl = 2;

#if FF_FS_EXFAT
    if (fs->fs_type == FS_EXFAT) {
        scl = find_bitmap(fs, stcl, tcl);            /* Find a contiguous cluster block */
        if (scl == 0) res = FR_DENIED;                /* No contiguous cluster block was found */
        if (scl == 0xFFFFFFFF) res = FR_DISK_ERR;
        if (res == FR_OK) {    /* A contiguous free area is found */
            if (opt) {        /* Allocate it now */
                res = change_bitmap(fs, scl, tcl, 1);    /* Mark the cluster block 'in use' */
                lclst = scl + tcl - 1;
            } else {        /* Set it as suggested point for next allocation */
                lclst = scl - 1;
            }
        }
    } else
#endif
    {
        scl = clst = stcl; ncl = 0;
        for (;;) {    /* Find a contiguous cluster block */
            n = get_fat(&fp->obj, clst);
            if (++clst >= fs->n_fatent) clst = 2;
            if (n == 1) { res = FR_INT_ERR; break; }
            if (n == 0xFFFFFFFF) { res = FR_DISK_ERR; break; }
            if (n == 0) {    /* Is it a free cluster? */
                if (++ncl == tcl) break;    /* Break if a contiguous cluster block is found */
            } else {
                scl = clst; ncl = 0;        /* Not a free cluster */
            }
            if (clst == stcl) { res = FR_DENIED; break; }    /* No contiguous cluster? */
        }
        if (res == FR_OK) {    /* A contiguous free area is found */
            if (opt) {        /* Allocate it now */
                for (clst = scl, n = tcl; n; clst++, n--) {    /* Create a cluster chain on the FAT */
                    res = put_fat(fs, clst, (n == 1) ? 0xFFFFFFFF : clst + 1);
                    if (res != FR_OK) break;
                    lclst = clst;
                }
            } else {        /* Set it as suggested point for next allocation */
                lclst = scl - 1;
            }
        }
    }

    if (res == FR_OK) {
        fs->last_clst = lclst;        /* Set suggested start cluster to start next */
        if (opt) {    /* Is it allocated now? */
            fp->obj.sclust = scl;        /* Update object allocation information */
            fp->obj.objsize = fsz;
            if (FF_FS_EXFAT) fp->obj.stat = 2;    /* Set status 'contiguous chain' */
            fp->flag |= FA_MODIFIED;
            if (fs->free_clst <= fs->n_fatent - 2) {    /* Update FSINFO */
                fs->free_clst -= tcl;
                fs->fsi_flag |= 1;
            }
        }
    }

    LEAVE_FF(fs, res);
}

#endif /* FF_USE_EXPAND && !FF_FS_READONLY */

