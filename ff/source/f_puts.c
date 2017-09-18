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
/* Put a String to the File                                              */
/*-----------------------------------------------------------------------*/
#if FF_USE_STRFUNC
#if !FF_FS_READONLY

#include <stdarg.h>

typedef struct {
    FIL *fp;        /* Ptr to the writing file */
    int idx, nchr;    /* Write index of buf[] (-1:error), number of chars written */
    BYTE buf[64];    /* Write buffer */
} putbuff;

static
void putc_bfd (        /* Buffered write with code conversion */
    putbuff* pb,
    TCHAR c
);

static
int putc_flush (        /* Flush left characters in the buffer */
    putbuff* pb
);

static
void putc_init (        /* Initialize write buffer */
    putbuff* pb,
    FIL* fp
);


int f_puts (
    const TCHAR* str,    /* Pointer to the string to be output */
    FIL* fp                /* Pointer to the file object */
)
{
    putbuff pb;

    putc_init(&pb, fp);
    while (*str) putc_bfd(&pb, *str++);        /* Put the string */
    return putc_flush(&pb);
}


static
void putc_bfd (        /* Buffered write with code conversion */
    putbuff* pb,
    TCHAR c
)
{
    UINT bw;
    int i;


    if (FF_USE_STRFUNC == 2 && c == '\n') {     /* LF -> CRLF conversion */
        putc_bfd(pb, '\r');
    }

    i = pb->idx;        /* Write index of pb->buf[] */
    if (i < 0) return;

#if FF_LFN_UNICODE && FF_USE_LFN    /* Unicode API */
#if FF_STRF_ENCODE == 3            /* Write a character in UTF-8 */
    if (c < 0x80) {                /* 7-bit */
        pb->buf[i++] = (BYTE)c;
    } else {
        if (c < 0x800) {        /* 11-bit */
            pb->buf[i++] = (BYTE)(0xC0 | c >> 6);
        } else {                /* 16-bit */
            pb->buf[i++] = (BYTE)(0xE0 | c >> 12);
            pb->buf[i++] = (BYTE)(0x80 | (c >> 6 & 0x3F));
        }
        pb->buf[i++] = (BYTE)(0x80 | (c & 0x3F));
    }
#elif FF_STRF_ENCODE == 2            /* Write a character in UTF-16BE */
    pb->buf[i++] = (BYTE)(c >> 8);
    pb->buf[i++] = (BYTE)c;
#elif FF_STRF_ENCODE == 1            /* Write a character in UTF-16LE */
    pb->buf[i++] = (BYTE)c;
    pb->buf[i++] = (BYTE)(c >> 8);
#else                            /* Write a character in ANSI/OEM */
    c = ff_uni2oem(c, CODEPAGE);    /* Unicode -> OEM */
    if (!c) c = '?';
    if (c >= 0x100)
        pb->buf[i++] = (BYTE)(c >> 8);
    pb->buf[i++] = (BYTE)c;
#endif
#else                            /* ANSI/OEM API: Write a character without conversion */
    pb->buf[i++] = (BYTE)c;
#endif

    if (i >= (int)(sizeof pb->buf) - 3) {    /* Write buffered characters to the file */
        f_write(pb->fp, pb->buf, (UINT)i, &bw);
        i = (bw == (UINT)i) ? 0 : -1;
    }
    pb->idx = i;
    pb->nchr++;
}


static
int putc_flush (        /* Flush left characters in the buffer */
    putbuff* pb
)
{
    UINT nw;

    if (   pb->idx >= 0    /* Flush buffered characters to the file */
        && f_write(pb->fp, pb->buf, (UINT)pb->idx, &nw) == FR_OK
        && (UINT)pb->idx == nw) return pb->nchr;
    return EOF;
}


static
void putc_init (        /* Initialize write buffer */
    putbuff* pb,
    FIL* fp
)
{
    pb->fp = fp;
    pb->nchr = pb->idx = 0;
}

#endif /* !FF_FS_READONLY */
#endif /* FF_USE_STRFUNC */

