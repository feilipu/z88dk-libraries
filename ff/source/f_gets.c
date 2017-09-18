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
/* Get a String from the File                                            */
/*-----------------------------------------------------------------------*/
#if FF_USE_STRFUNC

TCHAR* f_gets (
    TCHAR* buff,    /* Pointer to the string buffer to read */
    int len,        /* Size of string buffer (characters) */
    FIL* fp            /* Pointer to the file object */
)
{
    int n = 0;
    TCHAR c, *p = buff;
    BYTE s[2];
    UINT rc;


    while (n < len - 1) {    /* Read characters until buffer gets filled */
#if FF_LFN_UNICODE && FF_USE_LFN    /* Unicode API */
#if FF_STRF_ENCODE == 3        /* Read a character in UTF-8 */
        f_read(fp, s, 1, &rc);
        if (rc != 1) break;
        c = s[0];
        if (c >= 0x80) {
            if (c < 0xC0) continue;    /* Skip stray trailer */
            if (c < 0xE0) {            /* Two-byte sequence (0x80-0x7FF) */
                f_read(fp, s, 1, &rc);
                if (rc != 1) break;
                c = (c & 0x1F) << 6 | (s[0] & 0x3F);
                if (c < 0x80) c = '?';    /* Reject invalid code range */
            } else {
                if (c < 0xF0) {        /* Three-byte sequence (0x800-0xFFFF) */
                    f_read(fp, s, 2, &rc);
                    if (rc != 2) break;
                    c = c << 12 | (s[0] & 0x3F) << 6 | (s[1] & 0x3F);
                    if (c < 0x800) c = '?';    /* Reject invalid code range */
                } else {            /* Reject four-byte sequence */
                    c = '?';
                }
            }
        }
#elif FF_STRF_ENCODE == 2        /* Read a character in UTF-16BE */
        f_read(fp, s, 2, &rc);
        if (rc != 2) break;
        c = s[1] + (s[0] << 8);
#elif FF_STRF_ENCODE == 1        /* Read a character in UTF-16LE */
        f_read(fp, s, 2, &rc);
        if (rc != 2) break;
        c = s[0] + (s[1] << 8);
#else                            /* Read a character in ANSI/OEM */
        f_read(fp, s, 1, &rc);
        if (rc != 1) break;
        c = s[0];
        if (dbc_1st((BYTE)c)) {
            f_read(fp, s, 1, &rc);
            if (rc != 1) break;
            c = (c << 8) + s[0];
        }
        c = ff_oem2uni(c, CODEPAGE);    /* OEM -> Unicode */
        if (!c) c = '?';
#endif
#else                        /* ANSI/OEM API: Read a character without conversion */
        f_read(fp, s, 1, &rc);
        if (rc != 1) break;
        c = s[0];
#endif
        if (FF_USE_STRFUNC == 2 && c == '\r') continue;    /* Strip '\r' */
        *p++ = c;
        n++;
        if (c == '\n') break;        /* Break on EOL */
    }
    *p = 0;
    return n ? buff : 0;            /* When no data read (eof or error), return with error. */
}

#endif /* FF_USE_STRFUNC */

