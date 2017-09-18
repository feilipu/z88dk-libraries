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
/* Get Volume Label                                                      */
/*-----------------------------------------------------------------------*/
#if FF_USE_LABEL

FRESULT f_getlabel (
    const TCHAR* path,  /* Path name of the logical drive number */
    TCHAR* label,       /* Pointer to a buffer to store the volume label */
    DWORD* vsn          /* Pointer to a variable to store the volume serial number */
)
{
    FRESULT res;
    DIR dj;
    FATFS *fs;
    UINT si, di;
#if (FF_LFN_UNICODE && FF_USE_LFN) || FF_FS_EXFAT
    WCHAR w;
#endif

    /* Get logical drive */
    res = find_volume(&path, &fs, 0);

    /* Get volume label */
    if (res == FR_OK && label) {
        dj.obj.fs = fs; dj.obj.sclust = 0;    /* Open root directory */
        res = dir_sdi(&dj, 0);
        if (res == FR_OK) {
             res = dir_read(&dj, 1);            /* Find a volume label entry */
             if (res == FR_OK) {
#if FF_FS_EXFAT
                if (fs->fs_type == FS_EXFAT) {
                    for (si = di = 0; si < dj.dir[XDIR_NumLabel]; si++) {    /* Extract volume label from 83 entry */
                        w = ld_word(dj.dir + XDIR_Label + si * 2);
#if !FF_LFN_UNICODE        /* ANSI/OEM API */
                        w = ff_uni2oem(w, CODEPAGE);    /* Unicode -> OEM */
                        if (w == 0) w = '?';            /* Replace wrong char with '?' */
                        if (w >= 0x100) label[di++] = (char)(w >> 8);
#endif
                        label[di++] = (TCHAR)w;
                    }
                    label[di] = 0;
                } else
#endif
                {
                    si = di = 0;        /* Extract volume label from AM_VOL entry with code comversion */
                    do {
#if FF_LFN_UNICODE && FF_USE_LFN    /* Unicode API */
                        w = (si < 11) ? dj.dir[si++] : ' ';
                        if (dbc_1st((BYTE)w) && si < 11 && dbc_2nd(dj.dir[si])) {
                            w = w << 8 | dj.dir[si++];
                        }
                        label[di++] = ff_oem2uni(w, CODEPAGE);    /* OEM -> Unicode */
#else                                /* ANSI/OEM API */
                        label[di++] = dj.dir[si++];
#endif
                    } while (di < 11);
                    do {                /* Truncate trailing spaces */
                        label[di] = 0;
                        if (di == 0) break;
                    } while (label[--di] == ' ');
                }
            }
        }
        if (res == FR_NO_FILE) {    /* No label entry and return nul string */
            label[0] = 0;
            res = FR_OK;
        }
    }

    /* Get volume serial number */
    if (res == FR_OK && vsn) {
        res = move_window(fs, fs->volbase);
        if (res == FR_OK) {
            switch (fs->fs_type) {
            case FS_EXFAT:
                di = BPB_VolIDEx; break;

            case FS_FAT32:
                di = BS_VolID32; break;

            default:
                di = BS_VolID;
            }
            *vsn = ld_dword(fs->win + di);
        }
    }

    LEAVE_FF(fs, res);
}

#endif /* FF_USE_LABEL */

