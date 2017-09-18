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
/* Forward Data to the Stream Directly                                   */
/*-----------------------------------------------------------------------*/
#if FF_USE_FORWARD

FRESULT f_forward (
    FIL* fp,                        /* Pointer to the file object */
    UINT (*func)(const BYTE*,UINT), /* Pointer to the streaming function */
    UINT btf,                       /* Number of bytes to forward */
    UINT* bf                        /* Pointer to number of bytes forwarded */
)
{
    FRESULT res;
    FATFS *fs;
    DWORD clst, sect;
    FSIZE_t remain;
    UINT rcnt, csect;
    BYTE *dbuf;


    *bf = 0;    /* Clear transfer byte counter */
    res = validate(&fp->obj, &fs);        /* Check validity of the file object */
    if (res != FR_OK || (res = (FRESULT)fp->err) != FR_OK) LEAVE_FF(fs, res);
    if (!(fp->flag & FA_READ)) LEAVE_FF(fs, FR_DENIED);    /* Check access mode */

    remain = fp->obj.objsize - fp->fptr;
    if (btf > remain) btf = (UINT)remain;            /* Truncate btf by remaining bytes */

    for ( ;  btf && (*func)(0, 0);                    /* Repeat until all data transferred or stream goes busy */
        fp->fptr += rcnt, *bf += rcnt, btf -= rcnt) {
        csect = (UINT)(fp->fptr / SS(fs) & (fs->csize - 1));    /* Sector offset in the cluster */
        if (fp->fptr % SS(fs) == 0) {                /* On the sector boundary? */
            if (csect == 0) {                        /* On the cluster boundary? */
                clst = (fp->fptr == 0) ?            /* On the top of the file? */
                    fp->obj.sclust : get_fat(&fp->obj, fp->clust);
                if (clst <= 1) ABORT(fs, FR_INT_ERR);
                if (clst == 0xFFFFFFFF) ABORT(fs, FR_DISK_ERR);
                fp->clust = clst;                    /* Update current cluster */
            }
        }
        sect = clst2sect(fs, fp->clust);            /* Get current data sector */
        if (sect == 0) ABORT(fs, FR_INT_ERR);
        sect += csect;
#if FF_FS_TINY
        if (move_window(fs, sect) != FR_OK) ABORT(fs, FR_DISK_ERR);    /* Move sector window to the file data */
        dbuf = fs->win;
#else
        if (fp->sect != sect) {        /* Fill sector cache with file data */
#if !FF_FS_READONLY
            if (fp->flag & FA_DIRTY) {        /* Write-back dirty sector cache */
                if (disk_write(fs->pdrv, fp->buf, fp->sect, 1) != RES_OK) ABORT(fs, FR_DISK_ERR);
                fp->flag &= (BYTE)~FA_DIRTY;
            }
#endif
            if (disk_read(fs->pdrv, fp->buf, sect, 1) != RES_OK) ABORT(fs, FR_DISK_ERR);
        }
        dbuf = fp->buf;
#endif
        fp->sect = sect;
        rcnt = SS(fs) - (UINT)fp->fptr % SS(fs);    /* Number of bytes left in the sector */
        if (rcnt > btf) rcnt = btf;                    /* Clip it by btr if needed */
        rcnt = (*func)(dbuf + ((UINT)fp->fptr % SS(fs)), rcnt);    /* Forward the file data */
        if (rcnt == 0) ABORT(fs, FR_INT_ERR);
    }

    LEAVE_FF(fs, FR_OK);
}

#endif /* FF_USE_FORWARD */

