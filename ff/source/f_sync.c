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
/* Synchronize the File                                                  */
/*-----------------------------------------------------------------------*/
#if !FF_FS_READONLY

FRESULT f_sync (
    FIL* fp        /* Pointer to the file object */
)
{
    FRESULT res;
    FATFS *fs;
    DWORD tm;
    BYTE *dir;


    res = validate(&fp->obj, &fs);    /* Check validity of the file object */
    if (res == FR_OK) {
        if (fp->flag & FA_MODIFIED) {    /* Is there any change to the file? */
#if !FF_FS_TINY
            if (fp->flag & FA_DIRTY) {    /* Write-back cached data if needed */
                if (disk_write(fs->pdrv, fp->buf, fp->sect, 1) != RES_OK) LEAVE_FF(fs, FR_DISK_ERR);
                fp->flag &= (BYTE)~FA_DIRTY;
            }
#endif
            /* Update the directory entry */
            tm = GET_FATTIME();                /* Modified time */
#if FF_FS_EXFAT
            if (fs->fs_type == FS_EXFAT) {
                res = fill_first_frag(&fp->obj);    /* Fill first fragment on the FAT if needed */
                if (res == FR_OK) {
                    res = fill_last_frag(&fp->obj, fp->clust, 0xFFFFFFFF);    /* Fill last fragment on the FAT if needed */
                }
                if (res == FR_OK) {
                    DIR dj;
                    DEF_NAMBUF

                    INIT_NAMBUF(fs);
                    res = load_obj_xdir(&dj, &fp->obj);    /* Load directory entry block */
                    if (res == FR_OK) {
                        fs->dirbuf[XDIR_Attr] |= AM_ARC;                /* Set archive attribute to indicate that the file has been changed */
                        fs->dirbuf[XDIR_GenFlags] = fp->obj.stat | 1;    /* Update file allocation information */
                        st_dword(fs->dirbuf + XDIR_FstClus, fp->obj.sclust);
                        st_qword(fs->dirbuf + XDIR_FileSize, fp->obj.objsize);
                        st_qword(fs->dirbuf + XDIR_ValidFileSize, fp->obj.objsize);
                        st_dword(fs->dirbuf + XDIR_ModTime, tm);        /* Update modified time */
                        fs->dirbuf[XDIR_ModTime10] = 0;
                        st_dword(fs->dirbuf + XDIR_AccTime, 0);
                        res = store_xdir(&dj);    /* Restore it to the directory */
                        if (res == FR_OK) {
                            res = sync_fs(fs);
                            fp->flag &= (BYTE)~FA_MODIFIED;
                        }
                    }
                    FREE_NAMBUF();
                }
            } else
#endif
            {
                res = move_window(fs, fp->dir_sect);
                if (res == FR_OK) {
                    dir = fp->dir_ptr;
                    dir[DIR_Attr] |= AM_ARC;                        /* Set archive attribute to indicate that the file has been changed */
                    st_clust(fp->obj.fs, dir, fp->obj.sclust);        /* Update file allocation information  */
                    st_dword(dir + DIR_FileSize, (DWORD)fp->obj.objsize);    /* Update file size */
                    st_dword(dir + DIR_ModTime, tm);                /* Update modified time */
                    st_word(dir + DIR_LstAccDate, 0);
                    fs->wflag = 1;
                    res = sync_fs(fs);                    /* Restore it to the directory */
                    fp->flag &= (BYTE)~FA_MODIFIED;
                }
            }
        }
    }

    LEAVE_FF(fs, res);
}

#endif /* !FF_FS_READONLY */

