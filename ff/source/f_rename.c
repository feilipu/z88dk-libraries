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
/* Rename a File/Directory                                               */
/*-----------------------------------------------------------------------*/
#if FF_FS_MINIMIZE == 0
#if !FF_FS_READONLY

FRESULT f_rename (
    const TCHAR* path_old,    /* Pointer to the object name to be renamed */
    const TCHAR* path_new    /* Pointer to the new name */
)
{
    FRESULT res;
    DIR djo, djn;
    FATFS *fs;
    BYTE buf[FF_FS_EXFAT ? SZDIRE * 2 : SZDIRE], *dir;
    DWORD dw;
    DEF_NAMBUF


    get_ldnumber(&path_new);                        /* Snip the drive number of new name off */
    res = find_volume(&path_old, &fs, FA_WRITE);    /* Get logical drive of the old object */
    if (res == FR_OK) {
        djo.obj.fs = fs;
        INIT_NAMBUF(fs);
        res = follow_path(&djo, path_old);        /* Check old object */
        if (res == FR_OK && (djo.fn[NSFLAG] & (NS_DOT | NS_NONAME))) res = FR_INVALID_NAME;    /* Check validity of name */
#if FF_FS_LOCK != 0
        if (res == FR_OK) {
            res = chk_lock(&djo, 2);
        }
#endif
        if (res == FR_OK) {                        /* Object to be renamed is found */
#if FF_FS_EXFAT
            if (fs->fs_type == FS_EXFAT) {    /* At exFAT volume */
                BYTE nf, nn;
                WORD nh;

                MEMCPY(buf, fs->dirbuf, SZDIRE * 2);    /* Save 85+C0 entry of old object */
                MEMCPY(&djn, &djo, sizeof djo);
                res = follow_path(&djn, path_new);        /* Make sure if new object name is not in use */
                if (res == FR_OK) {                        /* Is new name already in use by any other object? */
                    res = (djn.obj.sclust == djo.obj.sclust && djn.dptr == djo.dptr) ? FR_NO_FILE : FR_EXIST;
                }
                if (res == FR_NO_FILE) {                 /* It is a valid path and no name collision */
                    res = dir_register(&djn);            /* Register the new entry */
                    if (res == FR_OK) {
                        nf = fs->dirbuf[XDIR_NumSec]; nn = fs->dirbuf[XDIR_NumName];
                        nh = ld_word(fs->dirbuf + XDIR_NameHash);
                        MEMCPY(fs->dirbuf, buf, SZDIRE * 2);    /* Restore 85+C0 entry */
                        fs->dirbuf[XDIR_NumSec] = nf; fs->dirbuf[XDIR_NumName] = nn;
                        st_word(fs->dirbuf + XDIR_NameHash, nh);
                        if (!(fs->dirbuf[XDIR_Attr] & AM_DIR)) fs->dirbuf[XDIR_Attr] |= AM_ARC;    /* Set archive attribute if it is a file */
/* Start of critical section where an interruption can cause a cross-link */
                        res = store_xdir(&djn);
                    }
                }
            } else
#endif
            {    /* At FAT/FAT32 volume */
                MEMCPY(buf, djo.dir, SZDIRE);            /* Save directory entry of the object */
                MEMCPY(&djn, &djo, sizeof (DIR));        /* Duplicate the directory object */
                res = follow_path(&djn, path_new);        /* Make sure if new object name is not in use */
                if (res == FR_OK) {                        /* Is new name already in use by any other object? */
                    res = (djn.obj.sclust == djo.obj.sclust && djn.dptr == djo.dptr) ? FR_NO_FILE : FR_EXIST;
                }
                if (res == FR_NO_FILE) {                 /* It is a valid path and no name collision */
                    res = dir_register(&djn);            /* Register the new entry */
                    if (res == FR_OK) {
                        dir = djn.dir;                    /* Copy directory entry of the object except name */
                        MEMCPY(dir + 13, buf + 13, SZDIRE - 13);
                        dir[DIR_Attr] = buf[DIR_Attr];
                        if (!(dir[DIR_Attr] & AM_DIR)) dir[DIR_Attr] |= AM_ARC;    /* Set archive attribute if it is a file */
                        fs->wflag = 1;
                        if ((dir[DIR_Attr] & AM_DIR) && djo.obj.sclust != djn.obj.sclust) {    /* Update .. entry in the sub-directory if needed */
                            dw = clst2sect(fs, ld_clust(fs, dir));
                            if (dw == 0) {
                                res = FR_INT_ERR;
                            } else {
/* Start of critical section where an interruption can cause a cross-link */
                                res = move_window(fs, dw);
                                dir = fs->win + SZDIRE * 1;    /* Ptr to .. entry */
                                if (res == FR_OK && dir[1] == '.') {
                                    st_clust(fs, dir, djn.obj.sclust);
                                    fs->wflag = 1;
                                }
                            }
                        }
                    }
                }
            }
            if (res == FR_OK) {
                res = dir_remove(&djo);        /* Remove old entry */
                if (res == FR_OK) {
                    res = sync_fs(fs);
                }
            }
/* End of the critical section */
        }
        FREE_NAMBUF();
    }

    LEAVE_FF(fs, res);
}

#endif /* !FF_FS_READONLY */
#endif /* FF_FS_MINIMIZE == 0 */

