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

#if FF_DEFINED != 87030     /* Revision ID */
#error Wrong include file (ff.h).
#endif

/* Remark: Variables defined here without initial value shall be guaranteed
/  zero/null at start-up. If not, the linker option or start-up routine is
/  not compliance with C standard. */

/*-----------------------------------------------------------------------*/
/* File/Volume controls                                                  */
/*-----------------------------------------------------------------------*/

#if FF_VOLUMES < 1 || FF_VOLUMES > 10
#error Wrong FF_VOLUMES setting
#endif
FATFS *FatFs[FF_VOLUMES];   /* Pointer to the filesystem objects (logical drives) */
WORD Fsid;                  /* File system mount ID */

#if FF_FS_RPATH != 0 && FF_VOLUMES >= 2
BYTE CurrVol;               /* Current drive */
#endif

#if FF_FS_LOCK != 0
FILESEM Files[FF_FS_LOCK];  /* Open object lock semaphores */
#endif



/*-----------------------------------------------------------------------*/
/* LFN/Directory working buffer                                          */
/*-----------------------------------------------------------------------*/

#if FF_USE_LFN != 0                  /* LFN configurations */

const BYTE LfnOfs[] = {1,3,5,7,9,14,16,18,20,22,24,28,30};    /* FAT: Offset of LFN characters in the directory entry */

#if FF_USE_LFN == 1                 /* LFN enabled with static working buffer */
#if FF_FS_EXFAT
BYTE DirBuf[MAXDIRB(FF_MAX_LFN)];   /* Directory entry block scratchpad buffer */
#endif
WCHAR LfnBuf[FF_MAX_LFN + 1];       /* LFN working buffer */
#endif

#endif



