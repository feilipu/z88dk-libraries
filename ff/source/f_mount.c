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
/* Mount/Unmount a Logical Drive                                         */
/*-----------------------------------------------------------------------*/

FRESULT f_mount (
    FATFS* fs,            /* Pointer to the filesystem object (NULL:unmount)*/
    const TCHAR* path,    /* Logical drive number to be mounted/unmounted */
    BYTE opt            /* Mode option 0:Do not mount (delayed mount), 1:Mount immediately */
)
{
    FATFS *cfs;
    int vol;
    FRESULT res;
    const TCHAR *rp = path;


    /* Get logical drive number */
    vol = get_ldnumber(&rp);
    if (vol < 0) return FR_INVALID_DRIVE;
    cfs = FatFs[vol];                    /* Pointer to fs object */

    if (cfs) {
#if FF_FS_LOCK != 0
        clear_lock(cfs);
#endif
#if FF_FS_REENTRANT                        /* Discard sync object of the current volume */
        if (!ff_del_syncobj(cfs->sobj)) return FR_INT_ERR;
#endif
        cfs->fs_type = 0;                /* Clear old fs object */
    }

    if (fs) {
        fs->fs_type = 0;                /* Clear new fs object */
#if FF_FS_REENTRANT                        /* Create sync object for the new volume */
        if (!ff_cre_syncobj((BYTE)vol, &fs->sobj)) return FR_INT_ERR;
#endif
    }
    FatFs[vol] = fs;                    /* Register new fs object */

    if (opt == 0) return FR_OK;            /* Do not mount now, it will be mounted later */

    res = find_volume(&path, &fs, 0);    /* Force mounted the volume */
    LEAVE_FF(fs, res);
}

