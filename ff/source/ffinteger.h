/*-------------------------------------------*/
/* Integer type definitions for FatFs module */
/*-------------------------------------------*/

#ifndef FF_INTEGER
#define FF_INTEGER

#ifndef __DISKIO_H__

/* These types MUST be 16-bit or 32-bit */
typedef int                 INT;
typedef unsigned int        UINT;

/* This type MUST be 8-bit */
typedef unsigned char       BYTE;

/* These types MUST be 16-bit */
typedef short               SHORT;
typedef unsigned short      WORD;
typedef unsigned short      WCHAR;

/* These types MUST be 32-bit */
typedef long                LONG;
typedef unsigned long       DWORD;

#ifndef __SCCZ80
/* This type MUST be 64-bit (Remove this for ANSI C (C89) compatibility) */
typedef unsigned long long  QWORD;
#endif

#endif  /* __DISKIO_H__ */

#endif  /* FF_INTEGER */
