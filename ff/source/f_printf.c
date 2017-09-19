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

#include "__ffstore.h"          /* extern for system storage */
#include "__ffunicodestore.h"   /* extern for LFN system storage */


/*-----------------------------------------------------------------------*/
/* Put a Formatted String to the File                                    */
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

int f_printf (
    FIL* fp,            /* Pointer to the file object */
    const TCHAR* fmt,    /* Pointer to the format string */
    ...                    /* Optional arguments... */
)
{
    va_list arp;
    putbuff pb;
    BYTE f, r;
    UINT i, j, w;
    DWORD v;
    TCHAR c, d, str[32], *p;


    putc_init(&pb, fp);

    va_start(arp, fmt);

    for (;;) {
        c = *fmt++;
        if (c == 0) break;            /* End of string */
        if (c != '%') {                /* Non escape character */
            putc_bfd(&pb, c);
            continue;
        }
        w = f = 0;
        c = *fmt++;
        if (c == '0') {                /* Flag: '0' padding */
            f = 1; c = *fmt++;
        } else {
            if (c == '-') {            /* Flag: left justified */
                f = 2; c = *fmt++;
            }
        }
        while (IsDigit(c)) {        /* Precision */
            w = w * 10 + c - '0';
            c = *fmt++;
        }
        if (c == 'l' || c == 'L') {    /* Prefix: Size is long int */
            f |= 4; c = *fmt++;
        }
        if (!c) break;
        d = c;
        if (IsLower(d)) d -= 0x20;
        switch (d) {                /* Type is... */
        case 'S' :                    /* String */
            p = va_arg(arp, TCHAR*);
            for (j = 0; p[j]; j++) ;
            if (!(f & 2)) {                        /* Right pad */
                while (j++ < w) putc_bfd(&pb, ' ');
            }
            while (*p) putc_bfd(&pb, *p++);        /* String body */
            while (j++ < w) putc_bfd(&pb, ' ');    /* Left pad */
            continue;

        case 'C' :                    /* Character */
            putc_bfd(&pb, (TCHAR)va_arg(arp, int)); continue;

        case 'B' :                    /* Binary */
            r = 2; break;

        case 'O' :                    /* Octal */
            r = 8; break;

        case 'D' :                    /* Signed decimal */
        case 'U' :                    /* Unsigned decimal */
            r = 10; break;

        case 'X' :                    /* Hexdecimal */
            r = 16; break;

        default:                    /* Unknown type (pass-through) */
            putc_bfd(&pb, c); continue;
        }

        /* Get an argument and put it in numeral */
        v = (f & 4) ? (DWORD)va_arg(arp, long) : ((d == 'D') ? (DWORD)(long)va_arg(arp, int) : (DWORD)va_arg(arp, unsigned int));
        if (d == 'D' && (v & 0x80000000)) {
            v = 0 - v;
            f |= 8;
        }
        i = 0;
        do {
            d = (TCHAR)(v % r); v /= r;
            if (d > 9) d += (c == 'x') ? 0x27 : 0x07;
            str[i++] = d + '0';
        } while (v && i < sizeof str / sizeof *str);
        if (f & 8) str[i++] = '-';
        j = i; d = (f & 1) ? '0' : ' ';
        if (!(f & 2)) {
            while (j++ < w) putc_bfd(&pb, d);    /* Right pad */
        }
        do {
            putc_bfd(&pb, str[--i]);            /* Number body */
        } while (i);
        while (j++ < w) putc_bfd(&pb, d);        /* Left pad */
    }

    va_end(arp);

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

