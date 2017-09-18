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
extern FATFS *FatFs[FF_VOLUMES];	/* Pointer to the filesystem objects (logical drives) */
extern WORD Fsid;					/* File system mount ID */

#if FF_FS_RPATH != 0 && FF_VOLUMES >= 2
extern BYTE CurrVol;				/* Current drive */
#endif

#if FF_FS_LOCK != 0
extern FILESEM Files[FF_FS_LOCK];	/* Open object lock semaphores */
#endif


/*-----------------------------------------------------------------------*/
/* LFN/Directory working buffer                                          */
/*-----------------------------------------------------------------------*/

#if FF_USE_LFN != 0                         /* LFN configurations */

extern const BYTE LfnOfs[] = {1,3,5,7,9,14,16,18,20,22,24,28,30};    /* FAT: Offset of LFN characters in the directory entry */

#if FF_USE_LFN == 1                         /* LFN enabled with static working buffer */
#if FF_FS_EXFAT
extern BYTE DirBuf[MAXDIRB(FF_MAX_LFN)];    /* Directory entry block scratchpad buffer */
#endif
extern WCHAR LfnBuf[FF_MAX_LFN + 1];        /* LFN working buffer */
#endif

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




/*-----------------------------------------------------------------------*/
/* Open or Create a File                                                 */
/*-----------------------------------------------------------------------*/

FRESULT f_open (
    FIL* fp,            /* Pointer to the blank file object */
    const TCHAR* path,    /* Pointer to the file name */
    BYTE mode            /* Access mode and file open mode flags */
)
{
    FRESULT res;
    DIR dj;
    FATFS *fs;
#if !FF_FS_READONLY
    DWORD dw, cl, bcs, clst, sc;
    FSIZE_t ofs;
#endif
    DEF_NAMBUF


    if (!fp) return FR_INVALID_OBJECT;

    /* Get logical drive */
    mode &= FF_FS_READONLY ? FA_READ : FA_READ | FA_WRITE | FA_CREATE_ALWAYS | FA_CREATE_NEW | FA_OPEN_ALWAYS | FA_OPEN_APPEND | FA_SEEKEND;
    res = find_volume(&path, &fs, mode);
    if (res == FR_OK) {
        dj.obj.fs = fs;
        INIT_NAMBUF(fs);
        res = follow_path(&dj, path);    /* Follow the file path */
#if !FF_FS_READONLY    /* Read/Write configuration */
        if (res == FR_OK) {
            if (dj.fn[NSFLAG] & NS_NONAME) {    /* Origin directory itself? */
                res = FR_INVALID_NAME;
            }
#if FF_FS_LOCK != 0
            else {
                res = chk_lock(&dj, (mode & ~FA_READ) ? 1 : 0);        /* Check if the file can be used */
            }
#endif
        }
        /* Create or Open a file */
        if (mode & (FA_CREATE_ALWAYS | FA_OPEN_ALWAYS | FA_CREATE_NEW)) {
            if (res != FR_OK) {                    /* No file, create new */
                if (res == FR_NO_FILE) {        /* There is no file to open, create a new entry */
#if FF_FS_LOCK != 0
                    res = enq_lock() ? dir_register(&dj) : FR_TOO_MANY_OPEN_FILES;
#else
                    res = dir_register(&dj);
#endif
                }
                mode |= FA_CREATE_ALWAYS;        /* File is created */
            }
            else {                                /* Any object with the same name is already existing */
                if (dj.obj.attr & (AM_RDO | AM_DIR)) {    /* Cannot overwrite it (R/O or DIR) */
                    res = FR_DENIED;
                } else {
                    if (mode & FA_CREATE_NEW) res = FR_EXIST;    /* Cannot create as new file */
                }
            }
            if (res == FR_OK && (mode & FA_CREATE_ALWAYS)) {    /* Truncate the file if overwrite mode */
#if FF_FS_EXFAT
                if (fs->fs_type == FS_EXFAT) {
                    /* Get current allocation info */
                    fp->obj.fs = fs;
                    fp->obj.sclust = cl = ld_dword(fs->dirbuf + XDIR_FstClus);
                    fp->obj.objsize = ld_qword(fs->dirbuf + XDIR_FileSize);
                    fp->obj.stat = fs->dirbuf[XDIR_GenFlags] & 2;
                    fp->obj.n_frag = 0;
                    /* Set directory entry block initial state */
                    MEMSET(fs->dirbuf + 2, 0, 30);        /* Clear 85 entry except for NumSec */
                    MEMSET(fs->dirbuf + 38, 0, 26);    /* Clear C0 entry except for NumName and NameHash */
                    fs->dirbuf[XDIR_Attr] = AM_ARC;
                    st_dword(fs->dirbuf + XDIR_CrtTime, GET_FATTIME());
                    fs->dirbuf[XDIR_GenFlags] = 1;
                    res = store_xdir(&dj);
                    if (res == FR_OK && cl != 0) {        /* Remove the cluster chain if exist */
                        res = remove_chain(&fp->obj, cl, 0);
                        fs->last_clst = cl - 1;            /* Reuse the cluster hole */
                    }
                } else
#endif
                {
                    /* Set directory entry initial state */
                    cl = ld_clust(fs, dj.dir);            /* Get current cluster chain */
                    st_dword(dj.dir + DIR_CrtTime, GET_FATTIME());    /* Set created time */
                    dj.dir[DIR_Attr] = AM_ARC;            /* Reset attribute */
                    st_clust(fs, dj.dir, 0);            /* Reset file allocation info */
                    st_dword(dj.dir + DIR_FileSize, 0);
                    fs->wflag = 1;
                    if (cl != 0) {                        /* Remove the cluster chain if exist */
                        dw = fs->winsect;
                        res = remove_chain(&dj.obj, cl, 0);
                        if (res == FR_OK) {
                            res = move_window(fs, dw);
                            fs->last_clst = cl - 1;        /* Reuse the cluster hole */
                        }
                    }
                }
            }
        }
        else {    /* Open an existing file */
            if (res == FR_OK) {                    /* Is the object exsiting? */
                if (dj.obj.attr & AM_DIR) {        /* File open against a directory */
                    res = FR_NO_FILE;
                } else {
                    if ((mode & FA_WRITE) && (dj.obj.attr & AM_RDO)) { /* Write mode open against R/O file */
                        res = FR_DENIED;
                    }
                }
            }
        }
        if (res == FR_OK) {
            if (mode & FA_CREATE_ALWAYS) mode |= FA_MODIFIED;    /* Set file change flag if created or overwritten */
            fp->dir_sect = fs->winsect;            /* Pointer to the directory entry */
            fp->dir_ptr = dj.dir;
#if FF_FS_LOCK != 0
            fp->obj.lockid = inc_lock(&dj, (mode & ~FA_READ) ? 1 : 0);    /* Lock the file for this session */
            if (!fp->obj.lockid) res = FR_INT_ERR;
#endif
        }
#else        /* R/O configuration */
        if (res == FR_OK) {
            if (dj.fn[NSFLAG] & NS_NONAME) {    /* Is it origin directory itself? */
                res = FR_INVALID_NAME;
            } else {
                if (dj.obj.attr & AM_DIR) {        /* Is it a directory? */
                    res = FR_NO_FILE;
                }
            }
        }
#endif

        if (res == FR_OK) {
#if FF_FS_EXFAT
            if (fs->fs_type == FS_EXFAT) {
                fp->obj.c_scl = dj.obj.sclust;                            /* Get containing directory info */
                fp->obj.c_size = ((DWORD)dj.obj.objsize & 0xFFFFFF00) | dj.obj.stat;
                fp->obj.c_ofs = dj.blk_ofs;
                fp->obj.sclust = ld_dword(fs->dirbuf + XDIR_FstClus);    /* Get object allocation info */
                fp->obj.objsize = ld_qword(fs->dirbuf + XDIR_FileSize);
                fp->obj.stat = fs->dirbuf[XDIR_GenFlags] & 2;
                fp->obj.n_frag = 0;
            } else
#endif
            {
                fp->obj.sclust = ld_clust(fs, dj.dir);                    /* Get object allocation info */
                fp->obj.objsize = ld_dword(dj.dir + DIR_FileSize);
            }
#if FF_USE_FASTSEEK
            fp->cltbl = 0;            /* Disable fast seek mode */
#endif
            fp->obj.fs = fs;         /* Validate the file object */
            fp->obj.id = fs->id;
            fp->flag = mode;        /* Set file access mode */
            fp->err = 0;            /* Clear error flag */
            fp->sect = 0;            /* Invalidate current data sector */
            fp->fptr = 0;            /* Set file pointer top of the file */
#if !FF_FS_READONLY
#if !FF_FS_TINY
            MEMSET(fp->buf, 0, FF_MAX_SS);    /* Clear sector buffer */
#endif
            if ((mode & FA_SEEKEND) && fp->obj.objsize > 0) {    /* Seek to end of file if FA_OPEN_APPEND is specified */
                fp->fptr = fp->obj.objsize;            /* Offset to seek */
                bcs = (DWORD)fs->csize * SS(fs);    /* Cluster size in byte */
                clst = fp->obj.sclust;                /* Follow the cluster chain */
                for (ofs = fp->obj.objsize; res == FR_OK && ofs > bcs; ofs -= bcs) {
                    clst = get_fat(&fp->obj, clst);
                    if (clst <= 1) res = FR_INT_ERR;
                    if (clst == 0xFFFFFFFF) res = FR_DISK_ERR;
                }
                fp->clust = clst;
                if (res == FR_OK && ofs % SS(fs)) {    /* Fill sector buffer if not on the sector boundary */
                    if ((sc = clst2sect(fs, clst)) == 0) {
                        res = FR_INT_ERR;
                    } else {
                        fp->sect = sc + (DWORD)(ofs / SS(fs));
#if !FF_FS_TINY
                        if (disk_read(fs->pdrv, fp->buf, fp->sect, 1) != RES_OK) res = FR_DISK_ERR;
#endif
                    }
                }
            }
#endif
        }

        FREE_NAMBUF();
    }

    if (res != FR_OK) fp->obj.fs = 0;    /* Invalidate file object on error */

    LEAVE_FF(fs, res);
}




/*-----------------------------------------------------------------------*/
/* Read File                                                             */
/*-----------------------------------------------------------------------*/

FRESULT f_read (
    FIL* fp,     /* Pointer to the file object */
    void* buff,    /* Pointer to data buffer */
    UINT btr,    /* Number of bytes to read */
    UINT* br    /* Pointer to number of bytes read */
)
{
    FRESULT res;
    FATFS *fs;
    DWORD clst, sect;
    FSIZE_t remain;
    UINT rcnt, cc, csect;
    BYTE *rbuff = (BYTE*)buff;


    *br = 0;    /* Clear read byte counter */
    res = validate(&fp->obj, &fs);                /* Check validity of the file object */
    if (res != FR_OK || (res = (FRESULT)fp->err) != FR_OK) LEAVE_FF(fs, res);    /* Check validity */
    if (!(fp->flag & FA_READ)) LEAVE_FF(fs, FR_DENIED); /* Check access mode */
    remain = fp->obj.objsize - fp->fptr;
    if (btr > remain) btr = (UINT)remain;        /* Truncate btr by remaining bytes */

    for ( ;  btr;                                /* Repeat until all data read */
        btr -= rcnt, *br += rcnt, rbuff += rcnt, fp->fptr += rcnt) {
        if (fp->fptr % SS(fs) == 0) {            /* On the sector boundary? */
            csect = (UINT)(fp->fptr / SS(fs) & (fs->csize - 1));    /* Sector offset in the cluster */
            if (csect == 0) {                    /* On the cluster boundary? */
                if (fp->fptr == 0) {            /* On the top of the file? */
                    clst = fp->obj.sclust;        /* Follow cluster chain from the origin */
                } else {                        /* Middle or end of the file */
#if FF_USE_FASTSEEK
                    if (fp->cltbl) {
                        clst = clmt_clust(fp, fp->fptr);    /* Get cluster# from the CLMT */
                    } else
#endif
                    {
                        clst = get_fat(&fp->obj, fp->clust);    /* Follow cluster chain on the FAT */
                    }
                }
                if (clst < 2) ABORT(fs, FR_INT_ERR);
                if (clst == 0xFFFFFFFF) ABORT(fs, FR_DISK_ERR);
                fp->clust = clst;                /* Update current cluster */
            }
            sect = clst2sect(fs, fp->clust);    /* Get current sector */
            if (sect == 0) ABORT(fs, FR_INT_ERR);
            sect += csect;
            cc = btr / SS(fs);                    /* When remaining bytes >= sector size, */
            if (cc > 0) {                        /* Read maximum contiguous sectors directly */
                if (csect + cc > fs->csize) {    /* Clip at cluster boundary */
                    cc = fs->csize - csect;
                }
                if (disk_read(fs->pdrv, rbuff, sect, cc) != RES_OK) ABORT(fs, FR_DISK_ERR);
#if !FF_FS_READONLY && FF_FS_MINIMIZE <= 2        /* Replace one of the read sectors with cached data if it contains a dirty sector */
#if FF_FS_TINY
                if (fs->wflag && fs->winsect - sect < cc) {
                    MEMCPY(rbuff + ((fs->winsect - sect) * SS(fs)), fs->win, SS(fs));
                }
#else
                if ((fp->flag & FA_DIRTY) && fp->sect - sect < cc) {
                    MEMCPY(rbuff + ((fp->sect - sect) * SS(fs)), fp->buf, SS(fs));
                }
#endif
#endif
                rcnt = SS(fs) * cc;                /* Number of bytes transferred */
                continue;
            }
#if !FF_FS_TINY
            if (fp->sect != sect) {            /* Load data sector if not in cache */
#if !FF_FS_READONLY
                if (fp->flag & FA_DIRTY) {        /* Write-back dirty sector cache */
                    if (disk_write(fs->pdrv, fp->buf, fp->sect, 1) != RES_OK) ABORT(fs, FR_DISK_ERR);
                    fp->flag &= (BYTE)~FA_DIRTY;
                }
#endif
                if (disk_read(fs->pdrv, fp->buf, sect, 1) != RES_OK)    ABORT(fs, FR_DISK_ERR);    /* Fill sector cache */
            }
#endif
            fp->sect = sect;
        }
        rcnt = SS(fs) - (UINT)fp->fptr % SS(fs);    /* Number of bytes left in the sector */
        if (rcnt > btr) rcnt = btr;                    /* Clip it by btr if needed */
#if FF_FS_TINY
        if (move_window(fs, fp->sect) != FR_OK) ABORT(fs, FR_DISK_ERR);    /* Move sector window */
        MEMCPY(rbuff, fs->win + fp->fptr % SS(fs), rcnt);    /* Extract partial sector */
#else
        MEMCPY(rbuff, fp->buf + fp->fptr % SS(fs), rcnt);    /* Extract partial sector */
#endif
    }

    LEAVE_FF(fs, FR_OK);
}




#if !FF_FS_READONLY
/*-----------------------------------------------------------------------*/
/* Write File                                                            */
/*-----------------------------------------------------------------------*/

FRESULT f_write (
    FIL* fp,            /* Pointer to the file object */
    const void* buff,    /* Pointer to the data to be written */
    UINT btw,            /* Number of bytes to write */
    UINT* bw            /* Pointer to number of bytes written */
)
{
    FRESULT res;
    FATFS *fs;
    DWORD clst, sect;
    UINT wcnt, cc, csect;
    const BYTE *wbuff = (const BYTE*)buff;


    *bw = 0;    /* Clear write byte counter */
    res = validate(&fp->obj, &fs);            /* Check validity of the file object */
    if (res != FR_OK || (res = (FRESULT)fp->err) != FR_OK) LEAVE_FF(fs, res);    /* Check validity */
    if (!(fp->flag & FA_WRITE)) LEAVE_FF(fs, FR_DENIED);    /* Check access mode */

    /* Check fptr wrap-around (file size cannot reach 4 GiB at FAT volume) */
    if ((!FF_FS_EXFAT || fs->fs_type != FS_EXFAT) && (DWORD)(fp->fptr + btw) < (DWORD)fp->fptr) {
        btw = (UINT)(0xFFFFFFFF - (DWORD)fp->fptr);
    }

    for ( ;  btw;                            /* Repeat until all data written */
        btw -= wcnt, *bw += wcnt, wbuff += wcnt, fp->fptr += wcnt, fp->obj.objsize = (fp->fptr > fp->obj.objsize) ? fp->fptr : fp->obj.objsize) {
        if (fp->fptr % SS(fs) == 0) {        /* On the sector boundary? */
            csect = (UINT)(fp->fptr / SS(fs)) & (fs->csize - 1);    /* Sector offset in the cluster */
            if (csect == 0) {                /* On the cluster boundary? */
                if (fp->fptr == 0) {        /* On the top of the file? */
                    clst = fp->obj.sclust;    /* Follow from the origin */
                    if (clst == 0) {        /* If no cluster is allocated, */
                        clst = create_chain(&fp->obj, 0);    /* create a new cluster chain */
                    }
                } else {                    /* On the middle or end of the file */
#if FF_USE_FASTSEEK
                    if (fp->cltbl) {
                        clst = clmt_clust(fp, fp->fptr);    /* Get cluster# from the CLMT */
                    } else
#endif
                    {
                        clst = create_chain(&fp->obj, fp->clust);    /* Follow or stretch cluster chain on the FAT */
                    }
                }
                if (clst == 0) break;        /* Could not allocate a new cluster (disk full) */
                if (clst == 1) ABORT(fs, FR_INT_ERR);
                if (clst == 0xFFFFFFFF) ABORT(fs, FR_DISK_ERR);
                fp->clust = clst;            /* Update current cluster */
                if (fp->obj.sclust == 0) fp->obj.sclust = clst;    /* Set start cluster if the first write */
            }
#if FF_FS_TINY
            if (fs->winsect == fp->sect && sync_window(fs) != FR_OK) ABORT(fs, FR_DISK_ERR);    /* Write-back sector cache */
#else
            if (fp->flag & FA_DIRTY) {        /* Write-back sector cache */
                if (disk_write(fs->pdrv, fp->buf, fp->sect, 1) != RES_OK) ABORT(fs, FR_DISK_ERR);
                fp->flag &= (BYTE)~FA_DIRTY;
            }
#endif
            sect = clst2sect(fs, fp->clust);    /* Get current sector */
            if (sect == 0) ABORT(fs, FR_INT_ERR);
            sect += csect;
            cc = btw / SS(fs);                /* When remaining bytes >= sector size, */
            if (cc > 0) {                    /* Write maximum contiguous sectors directly */
                if (csect + cc > fs->csize) {    /* Clip at cluster boundary */
                    cc = fs->csize - csect;
                }
                if (disk_write(fs->pdrv, wbuff, sect, cc) != RES_OK) ABORT(fs, FR_DISK_ERR);
#if FF_FS_MINIMIZE <= 2
#if FF_FS_TINY
                if (fs->winsect - sect < cc) {    /* Refill sector cache if it gets invalidated by the direct write */
                    MEMCPY(fs->win, wbuff + ((fs->winsect - sect) * SS(fs)), SS(fs));
                    fs->wflag = 0;
                }
#else
                if (fp->sect - sect < cc) { /* Refill sector cache if it gets invalidated by the direct write */
                    MEMCPY(fp->buf, wbuff + ((fp->sect - sect) * SS(fs)), SS(fs));
                    fp->flag &= (BYTE)~FA_DIRTY;
                }
#endif
#endif
                wcnt = SS(fs) * cc;        /* Number of bytes transferred */
                continue;
            }
#if FF_FS_TINY
            if (fp->fptr >= fp->obj.objsize) {    /* Avoid silly cache filling on the growing edge */
                if (sync_window(fs) != FR_OK) ABORT(fs, FR_DISK_ERR);
                fs->winsect = sect;
            }
#else
            if (fp->sect != sect &&         /* Fill sector cache with file data */
                fp->fptr < fp->obj.objsize &&
                disk_read(fs->pdrv, fp->buf, sect, 1) != RES_OK) {
                    ABORT(fs, FR_DISK_ERR);
            }
#endif
            fp->sect = sect;
        }
        wcnt = SS(fs) - (UINT)fp->fptr % SS(fs);    /* Number of bytes left in the sector */
        if (wcnt > btw) wcnt = btw;                    /* Clip it by btw if needed */
#if FF_FS_TINY
        if (move_window(fs, fp->sect) != FR_OK) ABORT(fs, FR_DISK_ERR);    /* Move sector window */
        MEMCPY(fs->win + fp->fptr % SS(fs), wbuff, wcnt);    /* Fit data to the sector */
        fs->wflag = 1;
#else
        MEMCPY(fp->buf + fp->fptr % SS(fs), wbuff, wcnt);    /* Fit data to the sector */
        fp->flag |= FA_DIRTY;
#endif
    }

    fp->flag |= FA_MODIFIED;                /* Set file change flag */

    LEAVE_FF(fs, FR_OK);
}




/*-----------------------------------------------------------------------*/
/* Synchronize the File                                                  */
/*-----------------------------------------------------------------------*/

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




/*-----------------------------------------------------------------------*/
/* Close File                                                            */
/*-----------------------------------------------------------------------*/

FRESULT f_close (
    FIL* fp        /* Pointer to the file object to be closed */
)
{
    FRESULT res;
    FATFS *fs;

#if !FF_FS_READONLY
    res = f_sync(fp);                    /* Flush cached data */
    if (res == FR_OK)
#endif
    {
        res = validate(&fp->obj, &fs);    /* Lock volume */
        if (res == FR_OK) {
#if FF_FS_LOCK != 0
            res = dec_lock(fp->obj.lockid);        /* Decrement file open counter */
            if (res == FR_OK) fp->obj.fs = 0;    /* Invalidate file object */
#else
            fp->obj.fs = 0;    /* Invalidate file object */
#endif
#if FF_FS_REENTRANT
            unlock_fs(fs, FR_OK);        /* Unlock volume */
#endif
        }
    }
    return res;
}




#if FF_FS_RPATH >= 1
/*-----------------------------------------------------------------------*/
/* Change Current Directory or Current Drive, Get Current Directory      */
/*-----------------------------------------------------------------------*/

#if FF_VOLUMES >= 2
FRESULT f_chdrive (
    const TCHAR* path        /* Drive number */
)
{
    int vol;


    /* Get logical drive number */
    vol = get_ldnumber(&path);
    if (vol < 0) return FR_INVALID_DRIVE;

    CurrVol = (BYTE)vol;    /* Set it as current volume */

    return FR_OK;
}
#endif


FRESULT f_chdir (
    const TCHAR* path    /* Pointer to the directory path */
)
{
    FRESULT res;
    DIR dj;
    FATFS *fs;
    DEF_NAMBUF

    /* Get logical drive */
    res = find_volume(&path, &fs, 0);
    if (res == FR_OK) {
        dj.obj.fs = fs;
        INIT_NAMBUF(fs);
        res = follow_path(&dj, path);        /* Follow the path */
        if (res == FR_OK) {                    /* Follow completed */
            if (dj.fn[NSFLAG] & NS_NONAME) {
                fs->cdir = dj.obj.sclust;    /* It is the start directory itself */
#if FF_FS_EXFAT
                if (fs->fs_type == FS_EXFAT) {
                    fs->cdc_scl = dj.obj.c_scl;
                    fs->cdc_size = dj.obj.c_size;
                    fs->cdc_ofs = dj.obj.c_ofs;
                }
#endif
            } else {
                if (dj.obj.attr & AM_DIR) {    /* It is a sub-directory */
#if FF_FS_EXFAT
                    if (fs->fs_type == FS_EXFAT) {
                        fs->cdir = ld_dword(fs->dirbuf + XDIR_FstClus);        /* Sub-directory cluster */
                        fs->cdc_scl = dj.obj.sclust;                        /* Save containing directory information */
                        fs->cdc_size = ((DWORD)dj.obj.objsize & 0xFFFFFF00) | dj.obj.stat;
                        fs->cdc_ofs = dj.blk_ofs;
                    } else
#endif
                    {
                        fs->cdir = ld_clust(fs, dj.dir);                    /* Sub-directory cluster */
                    }
                } else {
                    res = FR_NO_PATH;        /* Reached but a file */
                }
            }
        }
        FREE_NAMBUF();
        if (res == FR_NO_FILE) res = FR_NO_PATH;
    }

    LEAVE_FF(fs, res);
}


#if FF_FS_RPATH >= 2
FRESULT f_getcwd (
    TCHAR* buff,    /* Pointer to the directory path */
    UINT len        /* Size of path */
)
{
    FRESULT res;
    DIR dj;
    FATFS *fs;
    UINT i, n;
    DWORD ccl;
    TCHAR *tp;
    FILINFO fno;
    DEF_NAMBUF


    *buff = 0;
    /* Get logical drive */
    res = find_volume((const TCHAR**)&buff, &fs, 0);    /* Get current volume */
    if (res == FR_OK) {
        dj.obj.fs = fs;
        INIT_NAMBUF(fs);
        i = len;            /* Bottom of buffer (directory stack base) */
        if (!FF_FS_EXFAT || fs->fs_type != FS_EXFAT) {    /* (Cannot do getcwd on exFAT and returns root path) */
            dj.obj.sclust = fs->cdir;                /* Start to follow upper directory from current directory */
            while ((ccl = dj.obj.sclust) != 0) {    /* Repeat while current directory is a sub-directory */
                res = dir_sdi(&dj, 1 * SZDIRE);    /* Get parent directory */
                if (res != FR_OK) break;
                res = move_window(fs, dj.sect);
                if (res != FR_OK) break;
                dj.obj.sclust = ld_clust(fs, dj.dir);    /* Goto parent directory */
                res = dir_sdi(&dj, 0);
                if (res != FR_OK) break;
                do {                            /* Find the entry links to the child directory */
                    res = dir_read(&dj, 0);
                    if (res != FR_OK) break;
                    if (ccl == ld_clust(fs, dj.dir)) break;    /* Found the entry */
                    res = dir_next(&dj, 0);
                } while (res == FR_OK);
                if (res == FR_NO_FILE) res = FR_INT_ERR;/* It cannot be 'not found'. */
                if (res != FR_OK) break;
                get_fileinfo(&dj, &fno);        /* Get the directory name and push it to the buffer */
                for (n = 0; fno.fname[n]; n++) ;
                if (i < n + 3) {
                    res = FR_NOT_ENOUGH_CORE; break;
                }
                while (n) buff[--i] = fno.fname[--n];
                buff[--i] = '/';
            }
        }
        tp = buff;
        if (res == FR_OK) {
#if FF_VOLUMES >= 2
            *tp++ = '0' + CurrVol;            /* Put drive number */
            *tp++ = ':';
#endif
            if (i == len) {                    /* Root-directory */
                *tp++ = '/';
            } else {                        /* Sub-directroy */
                do        /* Add stacked path str */
                    *tp++ = buff[i++];
                while (i < len);
            }
        }
        *tp = 0;
        FREE_NAMBUF();
    }

    LEAVE_FF(fs, res);
}

#endif /* FF_FS_RPATH >= 2 */
#endif /* FF_FS_RPATH >= 1 */



#if FF_FS_MINIMIZE <= 2
/*-----------------------------------------------------------------------*/
/* Seek File Read/Write Pointer                                          */
/*-----------------------------------------------------------------------*/

FRESULT f_lseek (
    FIL* fp,        /* Pointer to the file object */
    FSIZE_t ofs        /* File pointer from top of file */
)
{
    FRESULT res;
    FATFS *fs;
    DWORD clst, bcs, nsect;
    FSIZE_t ifptr;
#if FF_USE_FASTSEEK
    DWORD cl, pcl, ncl, tcl, dsc, tlen, ulen, *tbl;
#endif

    res = validate(&fp->obj, &fs);        /* Check validity of the file object */
    if (res == FR_OK) res = (FRESULT)fp->err;
#if FF_FS_EXFAT && !FF_FS_READONLY
    if (res == FR_OK && fs->fs_type == FS_EXFAT) {
        res = fill_last_frag(&fp->obj, fp->clust, 0xFFFFFFFF);    /* Fill last fragment on the FAT if needed */
    }
#endif
    if (res != FR_OK) LEAVE_FF(fs, res);

#if FF_USE_FASTSEEK
    if (fp->cltbl) {    /* Fast seek */
        if (ofs == CREATE_LINKMAP) {    /* Create CLMT */
            tbl = fp->cltbl;
            tlen = *tbl++; ulen = 2;    /* Given table size and required table size */
            cl = fp->obj.sclust;        /* Origin of the chain */
            if (cl != 0) {
                do {
                    /* Get a fragment */
                    tcl = cl; ncl = 0; ulen += 2;    /* Top, length and used items */
                    do {
                        pcl = cl; ncl++;
                        cl = get_fat(&fp->obj, cl);
                        if (cl <= 1) ABORT(fs, FR_INT_ERR);
                        if (cl == 0xFFFFFFFF) ABORT(fs, FR_DISK_ERR);
                    } while (cl == pcl + 1);
                    if (ulen <= tlen) {        /* Store the length and top of the fragment */
                        *tbl++ = ncl; *tbl++ = tcl;
                    }
                } while (cl < fs->n_fatent);    /* Repeat until end of chain */
            }
            *fp->cltbl = ulen;    /* Number of items used */
            if (ulen <= tlen) {
                *tbl = 0;        /* Terminate table */
            } else {
                res = FR_NOT_ENOUGH_CORE;    /* Given table size is smaller than required */
            }
        } else {                        /* Fast seek */
            if (ofs > fp->obj.objsize) ofs = fp->obj.objsize;    /* Clip offset at the file size */
            fp->fptr = ofs;                /* Set file pointer */
            if (ofs > 0) {
                fp->clust = clmt_clust(fp, ofs - 1);
                dsc = clst2sect(fs, fp->clust);
                if (dsc == 0) ABORT(fs, FR_INT_ERR);
                dsc += (DWORD)((ofs - 1) / SS(fs)) & (fs->csize - 1);
                if (fp->fptr % SS(fs) && dsc != fp->sect) {    /* Refill sector cache if needed */
#if !FF_FS_TINY
#if !FF_FS_READONLY
                    if (fp->flag & FA_DIRTY) {        /* Write-back dirty sector cache */
                        if (disk_write(fs->pdrv, fp->buf, fp->sect, 1) != RES_OK) ABORT(fs, FR_DISK_ERR);
                        fp->flag &= (BYTE)~FA_DIRTY;
                    }
#endif
                    if (disk_read(fs->pdrv, fp->buf, dsc, 1) != RES_OK) ABORT(fs, FR_DISK_ERR);    /* Load current sector */
#endif
                    fp->sect = dsc;
                }
            }
        }
    } else
#endif

    /* Normal Seek */
    {
#if FF_FS_EXFAT
        if (fs->fs_type != FS_EXFAT && ofs >= 0x100000000) ofs = 0xFFFFFFFF;    /* Clip at 4 GiB - 1 if at FATxx */
#endif
        if (ofs > fp->obj.objsize && (FF_FS_READONLY || !(fp->flag & FA_WRITE))) {    /* In read-only mode, clip offset with the file size */
            ofs = fp->obj.objsize;
        }
        ifptr = fp->fptr;
        fp->fptr = nsect = 0;
        if (ofs > 0) {
            bcs = (DWORD)fs->csize * SS(fs);    /* Cluster size (byte) */
            if (ifptr > 0 &&
                (ofs - 1) / bcs >= (ifptr - 1) / bcs) {    /* When seek to same or following cluster, */
                fp->fptr = (ifptr - 1) & ~(FSIZE_t)(bcs - 1);    /* start from the current cluster */
                ofs -= fp->fptr;
                clst = fp->clust;
            } else {                                    /* When seek to back cluster, */
                clst = fp->obj.sclust;                    /* start from the first cluster */
#if !FF_FS_READONLY
                if (clst == 0) {                        /* If no cluster chain, create a new chain */
                    clst = create_chain(&fp->obj, 0);
                    if (clst == 1) ABORT(fs, FR_INT_ERR);
                    if (clst == 0xFFFFFFFF) ABORT(fs, FR_DISK_ERR);
                    fp->obj.sclust = clst;
                }
#endif
                fp->clust = clst;
            }
            if (clst != 0) {
                while (ofs > bcs) {                        /* Cluster following loop */
                    ofs -= bcs; fp->fptr += bcs;
#if !FF_FS_READONLY
                    if (fp->flag & FA_WRITE) {            /* Check if in write mode or not */
                        if (FF_FS_EXFAT && fp->fptr > fp->obj.objsize) {    /* No FAT chain object needs correct objsize to generate FAT value */
                            fp->obj.objsize = fp->fptr;
                            fp->flag |= FA_MODIFIED;
                        }
                        clst = create_chain(&fp->obj, clst);    /* Follow chain with forceed stretch */
                        if (clst == 0) {                /* Clip file size in case of disk full */
                            ofs = 0; break;
                        }
                    } else
#endif
                    {
                        clst = get_fat(&fp->obj, clst);    /* Follow cluster chain if not in write mode */
                    }
                    if (clst == 0xFFFFFFFF) ABORT(fs, FR_DISK_ERR);
                    if (clst <= 1 || clst >= fs->n_fatent) ABORT(fs, FR_INT_ERR);
                    fp->clust = clst;
                }
                fp->fptr += ofs;
                if (ofs % SS(fs)) {
                    nsect = clst2sect(fs, clst);    /* Current sector */
                    if (nsect == 0) ABORT(fs, FR_INT_ERR);
                    nsect += (DWORD)(ofs / SS(fs));
                }
            }
        }
        if (!FF_FS_READONLY && fp->fptr > fp->obj.objsize) {    /* Set file change flag if the file size is extended */
            fp->obj.objsize = fp->fptr;
            fp->flag |= FA_MODIFIED;
        }
        if (fp->fptr % SS(fs) && nsect != fp->sect) {    /* Fill sector cache if needed */
#if !FF_FS_TINY
#if !FF_FS_READONLY
            if (fp->flag & FA_DIRTY) {            /* Write-back dirty sector cache */
                if (disk_write(fs->pdrv, fp->buf, fp->sect, 1) != RES_OK) ABORT(fs, FR_DISK_ERR);
                fp->flag &= (BYTE)~FA_DIRTY;
            }
#endif
            if (disk_read(fs->pdrv, fp->buf, nsect, 1) != RES_OK) ABORT(fs, FR_DISK_ERR);    /* Fill sector cache */
#endif
            fp->sect = nsect;
        }
    }

    LEAVE_FF(fs, res);
}



#if FF_FS_MINIMIZE <= 1
/*-----------------------------------------------------------------------*/
/* Create a Directory Object                                             */
/*-----------------------------------------------------------------------*/

FRESULT f_opendir (
    DIR* dp,            /* Pointer to directory object to create */
    const TCHAR* path    /* Pointer to the directory path */
)
{
    FRESULT res;
    FATFS *fs;
    DEF_NAMBUF


    if (!dp) return FR_INVALID_OBJECT;

    /* Get logical drive */
    res = find_volume(&path, &fs, 0);
    if (res == FR_OK) {
        dp->obj.fs = fs;
        INIT_NAMBUF(fs);
        res = follow_path(dp, path);            /* Follow the path to the directory */
        if (res == FR_OK) {                        /* Follow completed */
            if (!(dp->fn[NSFLAG] & NS_NONAME)) {    /* It is not the origin directory itself */
                if (dp->obj.attr & AM_DIR) {        /* This object is a sub-directory */
#if FF_FS_EXFAT
                    if (fs->fs_type == FS_EXFAT) {
                        dp->obj.c_scl = dp->obj.sclust;                            /* Get containing directory inforamation */
                        dp->obj.c_size = ((DWORD)dp->obj.objsize & 0xFFFFFF00) | dp->obj.stat;
                        dp->obj.c_ofs = dp->blk_ofs;
                        dp->obj.sclust = ld_dword(fs->dirbuf + XDIR_FstClus);    /* Get object allocation info */
                        dp->obj.objsize = ld_qword(fs->dirbuf + XDIR_FileSize);
                        dp->obj.stat = fs->dirbuf[XDIR_GenFlags] & 2;
                    } else
#endif
                    {
                        dp->obj.sclust = ld_clust(fs, dp->dir);    /* Get object allocation info */
                    }
                } else {                        /* This object is a file */
                    res = FR_NO_PATH;
                }
            }
            if (res == FR_OK) {
                dp->obj.id = fs->id;
                res = dir_sdi(dp, 0);            /* Rewind directory */
#if FF_FS_LOCK != 0
                if (res == FR_OK) {
                    if (dp->obj.sclust != 0) {
                        dp->obj.lockid = inc_lock(dp, 0);    /* Lock the sub directory */
                        if (!dp->obj.lockid) res = FR_TOO_MANY_OPEN_FILES;
                    } else {
                        dp->obj.lockid = 0;    /* Root directory need not to be locked */
                    }
                }
#endif
            }
        }
        FREE_NAMBUF();
        if (res == FR_NO_FILE) res = FR_NO_PATH;
    }
    if (res != FR_OK) dp->obj.fs = 0;        /* Invalidate the directory object if function faild */

    LEAVE_FF(fs, res);
}




/*-----------------------------------------------------------------------*/
/* Close Directory                                                       */
/*-----------------------------------------------------------------------*/

FRESULT f_closedir (
    DIR *dp        /* Pointer to the directory object to be closed */
)
{
    FRESULT res;
    FATFS *fs;


    res = validate(&dp->obj, &fs);    /* Check validity of the file object */
    if (res == FR_OK) {
#if FF_FS_LOCK != 0
        if (dp->obj.lockid) res = dec_lock(dp->obj.lockid);    /* Decrement sub-directory open counter */
        if (res == FR_OK) dp->obj.fs = 0;    /* Invalidate directory object */
#else
        dp->obj.fs = 0;    /* Invalidate directory object */
#endif
#if FF_FS_REENTRANT
        unlock_fs(fs, FR_OK);        /* Unlock volume */
#endif
    }
    return res;
}




/*-----------------------------------------------------------------------*/
/* Read Directory Entries in Sequence                                    */
/*-----------------------------------------------------------------------*/

FRESULT f_readdir (
    DIR* dp,            /* Pointer to the open directory object */
    FILINFO* fno        /* Pointer to file information to return */
)
{
    FRESULT res;
    FATFS *fs;
    DEF_NAMBUF


    res = validate(&dp->obj, &fs);    /* Check validity of the directory object */
    if (res == FR_OK) {
        if (!fno) {
            res = dir_sdi(dp, 0);            /* Rewind the directory object */
        } else {
            INIT_NAMBUF(fs);
            res = dir_read(dp, 0);            /* Read an item */
            if (res == FR_NO_FILE) res = FR_OK;    /* Ignore end of directory */
            if (res == FR_OK) {                /* A valid entry is found */
                get_fileinfo(dp, fno);        /* Get the object information */
                res = dir_next(dp, 0);        /* Increment index for next */
                if (res == FR_NO_FILE) res = FR_OK;    /* Ignore end of directory now */
            }
            FREE_NAMBUF();
        }
    }
    LEAVE_FF(fs, res);
}



#if FF_USE_FIND
/*-----------------------------------------------------------------------*/
/* Find Next File                                                        */
/*-----------------------------------------------------------------------*/

FRESULT f_findnext (
    DIR* dp,        /* Pointer to the open directory object */
    FILINFO* fno    /* Pointer to the file information structure */
)
{
    FRESULT res;


    for (;;) {
        res = f_readdir(dp, fno);        /* Get a directory item */
        if (res != FR_OK || !fno || !fno->fname[0]) break;    /* Terminate if any error or end of directory */
        if (pattern_matching(dp->pat, fno->fname, 0, 0)) break;        /* Test for the file name */
#if FF_USE_LFN && FF_USE_FIND == 2
        if (pattern_matching(dp->pat, fno->altname, 0, 0)) break;    /* Test for alternative name if exist */
#endif
    }
    return res;
}



/*-----------------------------------------------------------------------*/
/* Find First File                                                       */
/*-----------------------------------------------------------------------*/

FRESULT f_findfirst (
    DIR* dp,                /* Pointer to the blank directory object */
    FILINFO* fno,            /* Pointer to the file information structure */
    const TCHAR* path,        /* Pointer to the directory to open */
    const TCHAR* pattern    /* Pointer to the matching pattern */
)
{
    FRESULT res;


    dp->pat = pattern;        /* Save pointer to pattern string */
    res = f_opendir(dp, path);        /* Open the target directory */
    if (res == FR_OK) {
        res = f_findnext(dp, fno);    /* Find the first item */
    }
    return res;
}

#endif    /* FF_USE_FIND */



#if FF_FS_MINIMIZE == 0
/*-----------------------------------------------------------------------*/
/* Get File Status                                                       */
/*-----------------------------------------------------------------------*/

FRESULT f_stat (
    const TCHAR* path,    /* Pointer to the file path */
    FILINFO* fno        /* Pointer to file information to return */
)
{
    FRESULT res;
    DIR dj;
    DEF_NAMBUF


    /* Get logical drive */
    res = find_volume(&path, &dj.obj.fs, 0);
    if (res == FR_OK) {
        INIT_NAMBUF(dj.obj.fs);
        res = follow_path(&dj, path);    /* Follow the file path */
        if (res == FR_OK) {                /* Follow completed */
            if (dj.fn[NSFLAG] & NS_NONAME) {    /* It is origin directory */
                res = FR_INVALID_NAME;
            } else {                            /* Found an object */
                if (fno) get_fileinfo(&dj, fno);
            }
        }
        FREE_NAMBUF();
    }

    LEAVE_FF(dj.obj.fs, res);
}



#if !FF_FS_READONLY
/*-----------------------------------------------------------------------*/
/* Get Number of Free Clusters                                           */
/*-----------------------------------------------------------------------*/

FRESULT f_getfree (
    const TCHAR* path,    /* Path name of the logical drive number */
    DWORD* nclst,        /* Pointer to a variable to return number of free clusters */
    FATFS** fatfs        /* Pointer to return pointer to corresponding filesystem object */
)
{
    FRESULT res;
    FATFS *fs;
    DWORD nfree, clst, sect, stat;
    UINT i;
    FFOBJID obj;


    /* Get logical drive */
    res = find_volume(&path, &fs, 0);
    if (res == FR_OK) {
        *fatfs = fs;                /* Return ptr to the fs object */
        /* If free_clst is valid, return it without full FAT scan */
        if (fs->free_clst <= fs->n_fatent - 2) {
            *nclst = fs->free_clst;
        } else {
            /* Scan FAT to obtain number of free clusters */
            nfree = 0;
            if (fs->fs_type == FS_FAT12) {    /* FAT12: Scan bit field FAT entries */
                clst = 2; obj.fs = fs;
                do {
                    stat = get_fat(&obj, clst);
                    if (stat == 0xFFFFFFFF) { res = FR_DISK_ERR; break; }
                    if (stat == 1) { res = FR_INT_ERR; break; }
                    if (stat == 0) nfree++;
                } while (++clst < fs->n_fatent);
            } else {
#if FF_FS_EXFAT
                if (fs->fs_type == FS_EXFAT) {    /* exFAT: Scan allocation bitmap */
                    BYTE bm;
                    UINT b;

                    clst = fs->n_fatent - 2;    /* Number of clusters */
                    sect = fs->database;        /* Assuming bitmap starts at cluster 2 */
                    i = 0;                        /* Offset in the sector */
                    do {    /* Counts numbuer of bits with zero in the bitmap */
                        if (i == 0) {
                            res = move_window(fs, sect++);
                            if (res != FR_OK) break;
                        }
                        for (b = 8, bm = fs->win[i]; b && clst; b--, clst--) {
                            if (!(bm & 1)) nfree++;
                            bm >>= 1;
                        }
                        i = (i + 1) % SS(fs);
                    } while (clst);
                } else
#endif
                {    /* FAT16/32: Scan WORD/DWORD FAT entries */
                    clst = fs->n_fatent;    /* Number of entries */
                    sect = fs->fatbase;        /* Top of the FAT */
                    i = 0;                    /* Offset in the sector */
                    do {    /* Counts numbuer of entries with zero in the FAT */
                        if (i == 0) {
                            res = move_window(fs, sect++);
                            if (res != FR_OK) break;
                        }
                        if (fs->fs_type == FS_FAT16) {
                            if (ld_word(fs->win + i) == 0) nfree++;
                            i += 2;
                        } else {
                            if ((ld_dword(fs->win + i) & 0x0FFFFFFF) == 0) nfree++;
                            i += 4;
                        }
                        i %= SS(fs);
                    } while (--clst);
                }
            }
            *nclst = nfree;            /* Return the free clusters */
            fs->free_clst = nfree;    /* Now free_clst is valid */
            fs->fsi_flag |= 1;        /* FAT32: FSInfo is to be updated */
        }
    }

    LEAVE_FF(fs, res);
}




/*-----------------------------------------------------------------------*/
/* Truncate File                                                         */
/*-----------------------------------------------------------------------*/

FRESULT f_truncate (
    FIL* fp        /* Pointer to the file object */
)
{
    FRESULT res;
    FATFS *fs;
    DWORD ncl;


    res = validate(&fp->obj, &fs);    /* Check validity of the file object */
    if (res != FR_OK || (res = (FRESULT)fp->err) != FR_OK) LEAVE_FF(fs, res);
    if (!(fp->flag & FA_WRITE)) LEAVE_FF(fs, FR_DENIED);    /* Check access mode */

    if (fp->fptr < fp->obj.objsize) {    /* Process when fptr is not on the eof */
        if (fp->fptr == 0) {    /* When set file size to zero, remove entire cluster chain */
            res = remove_chain(&fp->obj, fp->obj.sclust, 0);
            fp->obj.sclust = 0;
        } else {                /* When truncate a part of the file, remove remaining clusters */
            ncl = get_fat(&fp->obj, fp->clust);
            res = FR_OK;
            if (ncl == 0xFFFFFFFF) res = FR_DISK_ERR;
            if (ncl == 1) res = FR_INT_ERR;
            if (res == FR_OK && ncl < fs->n_fatent) {
                res = remove_chain(&fp->obj, ncl, fp->clust);
            }
        }
        fp->obj.objsize = fp->fptr;    /* Set file size to current read/write point */
        fp->flag |= FA_MODIFIED;
#if !FF_FS_TINY
        if (res == FR_OK && (fp->flag & FA_DIRTY)) {
            if (disk_write(fs->pdrv, fp->buf, fp->sect, 1) != RES_OK) {
                res = FR_DISK_ERR;
            } else {
                fp->flag &= (BYTE)~FA_DIRTY;
            }
        }
#endif
        if (res != FR_OK) ABORT(fs, res);
    }

    LEAVE_FF(fs, res);
}




/*-----------------------------------------------------------------------*/
/* Delete a File/Directory                                               */
/*-----------------------------------------------------------------------*/

FRESULT f_unlink (
    const TCHAR* path        /* Pointer to the file or directory path */
)
{
    FRESULT res;
    DIR dj, sdj;
    DWORD dclst = 0;
    FATFS *fs;
#if FF_FS_EXFAT
    FFOBJID obj;
#endif
    DEF_NAMBUF


    /* Get logical drive */
    res = find_volume(&path, &fs, FA_WRITE);
    if (res == FR_OK) {
        dj.obj.fs = fs;
        INIT_NAMBUF(fs);
        res = follow_path(&dj, path);        /* Follow the file path */
        if (FF_FS_RPATH && res == FR_OK && (dj.fn[NSFLAG] & NS_DOT)) {
            res = FR_INVALID_NAME;            /* Cannot remove dot entry */
        }
#if FF_FS_LOCK != 0
        if (res == FR_OK) res = chk_lock(&dj, 2);    /* Check if it is an open object */
#endif
        if (res == FR_OK) {                    /* The object is accessible */
            if (dj.fn[NSFLAG] & NS_NONAME) {
                res = FR_INVALID_NAME;        /* Cannot remove the origin directory */
            } else {
                if (dj.obj.attr & AM_RDO) {
                    res = FR_DENIED;        /* Cannot remove R/O object */
                }
            }
            if (res == FR_OK) {
#if FF_FS_EXFAT
                obj.fs = fs;
                if (fs->fs_type == FS_EXFAT) {
                    obj.sclust = dclst = ld_dword(fs->dirbuf + XDIR_FstClus);
                    obj.objsize = ld_qword(fs->dirbuf + XDIR_FileSize);
                    obj.stat = fs->dirbuf[XDIR_GenFlags] & 2;
                } else
#endif
                {
                    dclst = ld_clust(fs, dj.dir);
                }
                if (dj.obj.attr & AM_DIR) {            /* Is it a sub-directory? */
#if FF_FS_RPATH != 0
                    if (dclst == fs->cdir) {             /* Is it the current directory? */
                        res = FR_DENIED;
                    } else
#endif
                    {
                        sdj.obj.fs = fs;                /* Open the sub-directory */
                        sdj.obj.sclust = dclst;
#if FF_FS_EXFAT
                        if (fs->fs_type == FS_EXFAT) {
                            sdj.obj.objsize = obj.objsize;
                            sdj.obj.stat = obj.stat;
                        }
#endif
                        res = dir_sdi(&sdj, 0);
                        if (res == FR_OK) {
                            res = dir_read(&sdj, 0);            /* Read an item */
                            if (res == FR_OK) res = FR_DENIED;    /* Not empty? */
                            if (res == FR_NO_FILE) res = FR_OK;    /* Empty? */
                        }
                    }
                }
            }
            if (res == FR_OK) {
                res = dir_remove(&dj);            /* Remove the directory entry */
                if (res == FR_OK && dclst) {    /* Remove the cluster chain if exist */
#if FF_FS_EXFAT
                    res = remove_chain(&obj, dclst, 0);
#else
                    res = remove_chain(&dj.obj, dclst, 0);
#endif
                }
                if (res == FR_OK) res = sync_fs(fs);
            }
        }
        FREE_NAMBUF();
    }

    LEAVE_FF(fs, res);
}




/*-----------------------------------------------------------------------*/
/* Create a Directory                                                    */
/*-----------------------------------------------------------------------*/

FRESULT f_mkdir (
    const TCHAR* path        /* Pointer to the directory path */
)
{
    FRESULT res;
    DIR dj;
    FATFS *fs;
    BYTE *dir;
    DWORD dcl, pcl, tm;
    DEF_NAMBUF


    /* Get logical drive */
    res = find_volume(&path, &fs, FA_WRITE);
    if (res == FR_OK) {
        dj.obj.fs = fs;
        INIT_NAMBUF(fs);
        res = follow_path(&dj, path);            /* Follow the file path */
        if (res == FR_OK) res = FR_EXIST;        /* Any object with same name is already existing */
        if (FF_FS_RPATH && res == FR_NO_FILE && (dj.fn[NSFLAG] & NS_DOT)) {
            res = FR_INVALID_NAME;
        }
        if (res == FR_NO_FILE) {                /* Can create a new directory */
            dcl = create_chain(&dj.obj, 0);        /* Allocate a cluster for the new directory table */
            dj.obj.objsize = (DWORD)fs->csize * SS(fs);
            res = FR_OK;
            if (dcl == 0) res = FR_DENIED;        /* No space to allocate a new cluster */
            if (dcl == 1) res = FR_INT_ERR;
            if (dcl == 0xFFFFFFFF) res = FR_DISK_ERR;
            if (res == FR_OK) res = sync_window(fs);    /* Flush FAT */
            tm = GET_FATTIME();
            if (res == FR_OK) {                    /* Initialize the new directory table */
                res = dir_clear(fs, dcl);        /* Clean up the new table */
                if (res == FR_OK && (!FF_FS_EXFAT || fs->fs_type != FS_EXFAT)) {    /* Create dot entries (FAT only) */
                    dir = fs->win;
                    MEMSET(dir + DIR_Name, ' ', 11);    /* Create "." entry */
                    dir[DIR_Name] = '.';
                    dir[DIR_Attr] = AM_DIR;
                    st_dword(dir + DIR_ModTime, tm);
                    st_clust(fs, dir, dcl);
                    MEMCPY(dir + SZDIRE, dir, SZDIRE); /* Create ".." entry */
                    dir[SZDIRE + 1] = '.'; pcl = dj.obj.sclust;
                    st_clust(fs, dir + SZDIRE, pcl);
                    fs->wflag = 1;
                }
            }
            if (res == FR_OK) {
                res = dir_register(&dj);    /* Register the object to the directoy */
            }
            if (res == FR_OK) {
#if FF_FS_EXFAT
                if (fs->fs_type == FS_EXFAT) {    /* Initialize directory entry block */
                    st_dword(fs->dirbuf + XDIR_ModTime, tm);    /* Created time */
                    st_dword(fs->dirbuf + XDIR_FstClus, dcl);    /* Table start cluster */
                    st_dword(fs->dirbuf + XDIR_FileSize, (DWORD)dj.obj.objsize);    /* File size needs to be valid */
                    st_dword(fs->dirbuf + XDIR_ValidFileSize, (DWORD)dj.obj.objsize);
                    fs->dirbuf[XDIR_GenFlags] = 3;                /* Initialize the object flag */
                    fs->dirbuf[XDIR_Attr] = AM_DIR;                /* Attribute */
                    res = store_xdir(&dj);
                } else
#endif
                {
                    dir = dj.dir;
                    st_dword(dir + DIR_ModTime, tm);    /* Created time */
                    st_clust(fs, dir, dcl);                /* Table start cluster */
                    dir[DIR_Attr] = AM_DIR;                /* Attribute */
                    fs->wflag = 1;
                }
                if (res == FR_OK) {
                    res = sync_fs(fs);
                }
            } else {
                remove_chain(&dj.obj, dcl, 0);        /* Could not register, remove cluster chain */
            }
        }
        FREE_NAMBUF();
    }

    LEAVE_FF(fs, res);
}




/*-----------------------------------------------------------------------*/
/* Rename a File/Directory                                               */
/*-----------------------------------------------------------------------*/

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
#endif /* FF_FS_MINIMIZE <= 1 */
#endif /* FF_FS_MINIMIZE <= 2 */



#if FF_USE_CHMOD && !FF_FS_READONLY
/*-----------------------------------------------------------------------*/
/* Change Attribute                                                      */
/*-----------------------------------------------------------------------*/

FRESULT f_chmod (
    const TCHAR* path,    /* Pointer to the file path */
    BYTE attr,            /* Attribute bits */
    BYTE mask            /* Attribute mask to change */
)
{
    FRESULT res;
    DIR dj;
    FATFS *fs;
    DEF_NAMBUF


    res = find_volume(&path, &fs, FA_WRITE);    /* Get logical drive */
    if (res == FR_OK) {
        dj.obj.fs = fs;
        INIT_NAMBUF(fs);
        res = follow_path(&dj, path);    /* Follow the file path */
        if (res == FR_OK && (dj.fn[NSFLAG] & (NS_DOT | NS_NONAME))) res = FR_INVALID_NAME;    /* Check object validity */
        if (res == FR_OK) {
            mask &= AM_RDO|AM_HID|AM_SYS|AM_ARC;    /* Valid attribute mask */
#if FF_FS_EXFAT
            if (fs->fs_type == FS_EXFAT) {
                fs->dirbuf[XDIR_Attr] = (attr & mask) | (fs->dirbuf[XDIR_Attr] & (BYTE)~mask);    /* Apply attribute change */
                res = store_xdir(&dj);
            } else
#endif
            {
                dj.dir[DIR_Attr] = (attr & mask) | (dj.dir[DIR_Attr] & (BYTE)~mask);    /* Apply attribute change */
                fs->wflag = 1;
            }
            if (res == FR_OK) {
                res = sync_fs(fs);
            }
        }
        FREE_NAMBUF();
    }

    LEAVE_FF(fs, res);
}




/*-----------------------------------------------------------------------*/
/* Change Timestamp                                                      */
/*-----------------------------------------------------------------------*/

FRESULT f_utime (
    const TCHAR* path,    /* Pointer to the file/directory name */
    const FILINFO* fno    /* Pointer to the timestamp to be set */
)
{
    FRESULT res;
    DIR dj;
    FATFS *fs;
    DEF_NAMBUF


    res = find_volume(&path, &fs, FA_WRITE);    /* Get logical drive */
    if (res == FR_OK) {
        dj.obj.fs = fs;
        INIT_NAMBUF(fs);
        res = follow_path(&dj, path);    /* Follow the file path */
        if (res == FR_OK && (dj.fn[NSFLAG] & (NS_DOT | NS_NONAME))) res = FR_INVALID_NAME;    /* Check object validity */
        if (res == FR_OK) {
#if FF_FS_EXFAT
            if (fs->fs_type == FS_EXFAT) {
                st_dword(fs->dirbuf + XDIR_ModTime, (DWORD)fno->fdate << 16 | fno->ftime);
                res = store_xdir(&dj);
            } else
#endif
            {
                st_dword(dj.dir + DIR_ModTime, (DWORD)fno->fdate << 16 | fno->ftime);
                fs->wflag = 1;
            }
            if (res == FR_OK) {
                res = sync_fs(fs);
            }
        }
        FREE_NAMBUF();
    }

    LEAVE_FF(fs, res);
}

#endif    /* FF_USE_CHMOD && !FF_FS_READONLY */



#if FF_USE_LABEL
/*-----------------------------------------------------------------------*/
/* Get Volume Label                                                      */
/*-----------------------------------------------------------------------*/

FRESULT f_getlabel (
    const TCHAR* path,    /* Path name of the logical drive number */
    TCHAR* label,        /* Pointer to a buffer to store the volume label */
    DWORD* vsn            /* Pointer to a variable to store the volume serial number */
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



#if !FF_FS_READONLY
/*-----------------------------------------------------------------------*/
/* Set Volume Label                                                      */
/*-----------------------------------------------------------------------*/

FRESULT f_setlabel (
    const TCHAR* label    /* Pointer to the volume label to set */
)
{
    FRESULT res;
    DIR dj;
    FATFS *fs;
    BYTE dirvn[22];
    UINT i, j, slen;
    WCHAR w;
    static const char badchr[] = "\"*+,.:;<=>\?[]|\x7F";


    /* Get logical drive */
    res = find_volume(&label, &fs, FA_WRITE);
    if (res != FR_OK) LEAVE_FF(fs, res);
    dj.obj.fs = fs;

    /* Get length of given volume label */
    for (slen = 0; (UINT)label[slen] >= ' '; slen++) {}    /* Get name length */

#if FF_FS_EXFAT
    if (fs->fs_type == FS_EXFAT) {    /* On the exFAT volume */
        for (i = j = 0; i < slen; ) {    /* Create volume label in directory form */
            w = label[i++];
#if !FF_LFN_UNICODE    /* ANSI/OEM API */
            if (dbc_1st((BYTE)w)) {
                w = (i < slen && dbc_2nd((BYTE)label[i])) ? w << 8 | (BYTE)label[i++] : 0;
            }
            w = ff_oem2uni(w, CODEPAGE);
#endif
            if (w == 0 || chk_chr(badchr, w) || j == 22) {    /* Check validity check validity of the volume label */
                LEAVE_FF(fs, FR_INVALID_NAME);
            }
            st_word(dirvn + j, w); j += 2;
        }
        slen = j;
    } else
#endif
    {    /* On the FAT/FAT32 volume */
        for ( ; slen && label[slen - 1] == ' '; slen--) ;    /* Remove trailing spaces */
        if (slen != 0) {        /* Is there a volume label to be set? */
            dirvn[0] = 0; i = j = 0;    /* Create volume label in directory form */
            do {
#if FF_LFN_UNICODE && FF_USE_LFN    /* Unicode API */
                w = ff_uni2oem(ff_wtoupper(label[i++]), CODEPAGE);
#else                                /* ANSI/OEM API */
                w = (BYTE)label[i++];
                if (dbc_1st((BYTE)w)) {
                    w = (j < 10 && i < slen && dbc_2nd((BYTE)label[i])) ? w << 8 | (BYTE)label[i++] : 0;
                }
#if FF_USE_LFN
                w = ff_uni2oem(ff_wtoupper(ff_oem2uni(w, CODEPAGE)), CODEPAGE);
#else
                if (IsLower(w)) w -= 0x20;            /* To upper ASCII characters */
#if FF_CODE_PAGE == 0
                if (ExCvt && w >= 0x80) w = ExCvt[w - 0x80];    /* To upper extended characters (SBCS cfg) */
#elif FF_CODE_PAGE < 900
                if (w >= 0x80) w = ExCvt[w - 0x80];    /* To upper extended characters (SBCS cfg) */
#endif
#endif
#endif
                if (w == 0 || chk_chr(badchr, w) || j >= (UINT)((w >= 0x100) ? 10 : 11)) {    /* Reject invalid characters for volume label */
                    LEAVE_FF(fs, FR_INVALID_NAME);
                }
                if (w >= 0x100) dirvn[j++] = (BYTE)(w >> 8);
                dirvn[j++] = (BYTE)w;
            } while (i < slen);
            while (j < 11) dirvn[j++] = ' ';    /* Fill remaining name field */
            if (dirvn[0] == DDEM) LEAVE_FF(fs, FR_INVALID_NAME);    /* Reject illegal name (heading DDEM) */
        }
    }

    /* Set volume label */
    dj.obj.sclust = 0;        /* Open root directory */
    res = dir_sdi(&dj, 0);
    if (res == FR_OK) {
        res = dir_read(&dj, 1);    /* Get volume label entry */
        if (res == FR_OK) {
            if (FF_FS_EXFAT && fs->fs_type == FS_EXFAT) {
                dj.dir[XDIR_NumLabel] = (BYTE)(slen / 2);    /* Change the volume label */
                MEMCPY(dj.dir + XDIR_Label, dirvn, slen);
            } else {
                if (slen != 0) {
                    MEMCPY(dj.dir, dirvn, 11);    /* Change the volume label */
                } else {
                    dj.dir[DIR_Name] = DDEM;    /* Remove the volume label */
                }
            }
            fs->wflag = 1;
            res = sync_fs(fs);
        } else {            /* No volume label entry or an error */
            if (res == FR_NO_FILE) {
                res = FR_OK;
                if (slen != 0) {    /* Create a volume label entry */
                    res = dir_alloc(&dj, 1);    /* Allocate an entry */
                    if (res == FR_OK) {
                        MEMSET(dj.dir, 0, SZDIRE);    /* Clear the entry */
                        if (FF_FS_EXFAT && fs->fs_type == FS_EXFAT) {
                            dj.dir[XDIR_Type] = 0x83;        /* Create 83 entry */
                            dj.dir[XDIR_NumLabel] = (BYTE)(slen / 2);
                            MEMCPY(dj.dir + XDIR_Label, dirvn, slen);
                        } else {
                            dj.dir[DIR_Attr] = AM_VOL;        /* Create volume label entry */
                            MEMCPY(dj.dir, dirvn, 11);
                        }
                        fs->wflag = 1;
                        res = sync_fs(fs);
                    }
                }
            }
        }
    }

    LEAVE_FF(fs, res);
}

#endif /* !FF_FS_READONLY */
#endif /* FF_USE_LABEL */



#if FF_USE_EXPAND && !FF_FS_READONLY
/*-----------------------------------------------------------------------*/
/* Allocate a Contiguous Blocks to the File                              */
/*-----------------------------------------------------------------------*/

FRESULT f_expand (
    FIL* fp,        /* Pointer to the file object */
    FSIZE_t fsz,    /* File size to be expanded to */
    BYTE opt        /* Operation mode 0:Find and prepare or 1:Find and allocate */
)
{
    FRESULT res;
    FATFS *fs;
    DWORD n, clst, stcl, scl, ncl, tcl, lclst;


    res = validate(&fp->obj, &fs);        /* Check validity of the file object */
    if (res != FR_OK || (res = (FRESULT)fp->err) != FR_OK) LEAVE_FF(fs, res);
    if (fsz == 0 || fp->obj.objsize != 0 || !(fp->flag & FA_WRITE)) LEAVE_FF(fs, FR_DENIED);
#if FF_FS_EXFAT
    if (fs->fs_type != FS_EXFAT && fsz >= 0x100000000) LEAVE_FF(fs, FR_DENIED);    /* Check if in size limit */
#endif
    n = (DWORD)fs->csize * SS(fs);    /* Cluster size */
    tcl = (DWORD)(fsz / n) + ((fsz & (n - 1)) ? 1 : 0);    /* Number of clusters required */
    stcl = fs->last_clst; lclst = 0;
    if (stcl < 2 || stcl >= fs->n_fatent) stcl = 2;

#if FF_FS_EXFAT
    if (fs->fs_type == FS_EXFAT) {
        scl = find_bitmap(fs, stcl, tcl);            /* Find a contiguous cluster block */
        if (scl == 0) res = FR_DENIED;                /* No contiguous cluster block was found */
        if (scl == 0xFFFFFFFF) res = FR_DISK_ERR;
        if (res == FR_OK) {    /* A contiguous free area is found */
            if (opt) {        /* Allocate it now */
                res = change_bitmap(fs, scl, tcl, 1);    /* Mark the cluster block 'in use' */
                lclst = scl + tcl - 1;
            } else {        /* Set it as suggested point for next allocation */
                lclst = scl - 1;
            }
        }
    } else
#endif
    {
        scl = clst = stcl; ncl = 0;
        for (;;) {    /* Find a contiguous cluster block */
            n = get_fat(&fp->obj, clst);
            if (++clst >= fs->n_fatent) clst = 2;
            if (n == 1) { res = FR_INT_ERR; break; }
            if (n == 0xFFFFFFFF) { res = FR_DISK_ERR; break; }
            if (n == 0) {    /* Is it a free cluster? */
                if (++ncl == tcl) break;    /* Break if a contiguous cluster block is found */
            } else {
                scl = clst; ncl = 0;        /* Not a free cluster */
            }
            if (clst == stcl) { res = FR_DENIED; break; }    /* No contiguous cluster? */
        }
        if (res == FR_OK) {    /* A contiguous free area is found */
            if (opt) {        /* Allocate it now */
                for (clst = scl, n = tcl; n; clst++, n--) {    /* Create a cluster chain on the FAT */
                    res = put_fat(fs, clst, (n == 1) ? 0xFFFFFFFF : clst + 1);
                    if (res != FR_OK) break;
                    lclst = clst;
                }
            } else {        /* Set it as suggested point for next allocation */
                lclst = scl - 1;
            }
        }
    }

    if (res == FR_OK) {
        fs->last_clst = lclst;        /* Set suggested start cluster to start next */
        if (opt) {    /* Is it allocated now? */
            fp->obj.sclust = scl;        /* Update object allocation information */
            fp->obj.objsize = fsz;
            if (FF_FS_EXFAT) fp->obj.stat = 2;    /* Set status 'contiguous chain' */
            fp->flag |= FA_MODIFIED;
            if (fs->free_clst <= fs->n_fatent - 2) {    /* Update FSINFO */
                fs->free_clst -= tcl;
                fs->fsi_flag |= 1;
            }
        }
    }

    LEAVE_FF(fs, res);
}

#endif /* FF_USE_EXPAND && !FF_FS_READONLY */



#if FF_USE_FORWARD
/*-----------------------------------------------------------------------*/
/* Forward Data to the Stream Directly                                   */
/*-----------------------------------------------------------------------*/

FRESULT f_forward (
    FIL* fp,                         /* Pointer to the file object */
    UINT (*func)(const BYTE*,UINT),    /* Pointer to the streaming function */
    UINT btf,                        /* Number of bytes to forward */
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



#if FF_USE_MKFS && !FF_FS_READONLY
/*-----------------------------------------------------------------------*/
/* Create an FAT/exFAT volume                                            */
/*-----------------------------------------------------------------------*/

FRESULT f_mkfs (
    const TCHAR* path,    /* Logical drive number */
    BYTE opt,            /* Format option */
    DWORD au,            /* Size of allocation unit (cluster) [byte] */
    void* work,            /* Pointer to working buffer */
    UINT len            /* Size of working buffer */
)
{
    const UINT n_fats = 1;        /* Number of FATs for FAT/FAT32 volume (1 or 2) */
    const UINT n_rootdir = 512;    /* Number of root directory entries for FAT volume */
    static const WORD cst[] = {1, 4, 16, 64, 256, 512, 0};    /* Cluster size boundary for FAT volume (4Ks unit) */
    static const WORD cst32[] = {1, 2, 4, 8, 16, 32, 0};    /* Cluster size boundary for FAT32 volume (128Ks unit) */
    BYTE fmt, sys, *buf, *pte, pdrv, part;
    WORD ss;
    DWORD szb_buf, sz_buf, sz_blk, n_clst, pau, sect, nsect, n;
    DWORD b_vol, b_fat, b_data;                /* Base LBA for volume, fat, data */
    DWORD sz_vol, sz_rsv, sz_fat, sz_dir;    /* Size for volume, fat, dir, data */
    UINT i;
    int vol;
    DSTATUS stat;
#if FF_USE_TRIM || FF_FS_EXFAT
    DWORD tbl[3];
#endif


    /* Check mounted drive and clear work area */
    vol = get_ldnumber(&path);                    /* Get target logical drive */
    if (vol < 0) return FR_INVALID_DRIVE;
    if (FatFs[vol]) FatFs[vol]->fs_type = 0;    /* Clear the volume */
    pdrv = LD2PD(vol);    /* Physical drive */
    part = LD2PT(vol);    /* Partition (0:create as new, 1-4:get from partition table) */

    /* Check physical drive status */
    stat = disk_initialize(pdrv);
    if (stat & STA_NOINIT) return FR_NOT_READY;
    if (stat & STA_PROTECT) return FR_WRITE_PROTECTED;
    if (disk_ioctl(pdrv, GET_BLOCK_SIZE, &sz_blk) != RES_OK || !sz_blk || sz_blk > 32768 || (sz_blk & (sz_blk - 1))) sz_blk = 1;    /* Erase block to align data area */
#if FF_MAX_SS != FF_MIN_SS        /* Get sector size of the medium if variable sector size cfg. */
    if (disk_ioctl(pdrv, GET_SECTOR_SIZE, &ss) != RES_OK) return FR_DISK_ERR;
    if (ss > FF_MAX_SS || ss < FF_MIN_SS || (ss & (ss - 1))) return FR_DISK_ERR;
#else
    ss = FF_MAX_SS;
#endif
    if ((au != 0 && au < ss) || au > 0x1000000 || (au & (au - 1))) return FR_INVALID_PARAMETER;    /* Check if au is valid */
    au /= ss;    /* Cluster size in unit of sector */

    /* Get working buffer */
    buf = (BYTE*)work;        /* Working buffer */
    sz_buf = len / ss;        /* Size of working buffer (sector) */
    szb_buf = sz_buf * ss;    /* Size of working buffer (byte) */
    if (szb_buf == 0) return FR_MKFS_ABORTED;

    /* Determine where the volume to be located (b_vol, sz_vol) */
    if (FF_MULTI_PARTITION && part != 0) {
        /* Get partition information from partition table in the MBR */
        if (disk_read(pdrv, buf, 0, 1) != RES_OK) return FR_DISK_ERR;    /* Load MBR */
        if (ld_word(buf + BS_55AA) != 0xAA55) return FR_MKFS_ABORTED;    /* Check if MBR is valid */
        pte = buf + (MBR_Table + (part - 1) * SZ_PTE);
        if (!pte[PTE_System]) return FR_MKFS_ABORTED;    /* No partition? */
        b_vol = ld_dword(pte + PTE_StLba);        /* Get volume start sector */
        sz_vol = ld_dword(pte + PTE_SizLba);    /* Get volume size */
    } else {
        /* Create a single-partition in this function */
        if (disk_ioctl(pdrv, GET_SECTOR_COUNT, &sz_vol) != RES_OK) return FR_DISK_ERR;
        b_vol = (opt & FM_SFD) ? 0 : 63;        /* Volume start sector */
        if (sz_vol < b_vol) return FR_MKFS_ABORTED;
        sz_vol -= b_vol;                        /* Volume size */
    }
    if (sz_vol < 128) return FR_MKFS_ABORTED;    /* Check if volume size is >=128s */

    /* Pre-determine the FAT type */
    do {
        if (FF_FS_EXFAT && (opt & FM_EXFAT)) {    /* exFAT possible? */
            if ((opt & FM_ANY) == FM_EXFAT || sz_vol >= 0x4000000 || au > 128) {    /* exFAT only, vol >= 64Ms or au > 128s ? */
                fmt = FS_EXFAT; break;
            }
        }
        if (au > 128) return FR_INVALID_PARAMETER;    /* Too large au for FAT/FAT32 */
        if (opt & FM_FAT32) {    /* FAT32 possible? */
            if ((opt & FM_ANY) == FM_FAT32 || !(opt & FM_FAT)) {    /* FAT32 only or no-FAT? */
                fmt = FS_FAT32; break;
            }
        }
        if (!(opt & FM_FAT)) return FR_INVALID_PARAMETER;    /* no-FAT? */
        fmt = FS_FAT16;
    } while (0);

#if FF_FS_EXFAT
    if (fmt == FS_EXFAT) {    /* Create an exFAT volume */
        DWORD szb_bit, szb_case, sum, nb, cl;
        WCHAR ch, si;
        UINT j, st;
        BYTE b;

        if (sz_vol < 0x1000) return FR_MKFS_ABORTED;    /* Too small volume? */
#if FF_USE_TRIM
        tbl[0] = b_vol; tbl[1] = b_vol + sz_vol - 1;    /* Inform the device the volume area may be erased */
        disk_ioctl(pdrv, CTRL_TRIM, tbl);
#endif
        /* Determine FAT location, data location and number of clusters */
        if (au == 0) {    /* au auto-selection */
            au = 8;
            if (sz_vol >= 0x80000) au = 64;        /* >= 512Ks */
            if (sz_vol >= 0x4000000) au = 256;    /* >= 64Ms */
        }
        b_fat = b_vol + 32;                                        /* FAT start at offset 32 */
        sz_fat = ((sz_vol / au + 2) * 4 + ss - 1) / ss;            /* Number of FAT sectors */
        b_data = (b_fat + sz_fat + sz_blk - 1) & ~(sz_blk - 1);    /* Align data area to the erase block boundary */
        if (b_data >= sz_vol / 2) return FR_MKFS_ABORTED;        /* Too small volume? */
        n_clst = (sz_vol - (b_data - b_vol)) / au;                /* Number of clusters */
        if (n_clst <16) return FR_MKFS_ABORTED;                    /* Too few clusters? */
        if (n_clst > MAX_EXFAT) return FR_MKFS_ABORTED;            /* Too many clusters? */

        szb_bit = (n_clst + 7) / 8;                        /* Size of allocation bitmap */
        tbl[0] = (szb_bit + au * ss - 1) / (au * ss);    /* Number of allocation bitmap clusters */

        /* Create a compressed up-case table */
        sect = b_data + au * tbl[0];    /* Table start sector */
        sum = 0;                        /* Table checksum to be stored in the 82 entry */
        st = si = i = j = szb_case = 0;
        do {
            switch (st) {
            case 0:
                ch = ff_wtoupper(si);    /* Get an up-case char */
                if (ch != si) {
                    si++; break;        /* Store the up-case char if exist */
                }
                for (j = 1; (WCHAR)(si + j) && (WCHAR)(si + j) == ff_wtoupper((WCHAR)(si + j)); j++) ;    /* Get run length of no-case block */
                if (j >= 128) {
                    ch = 0xFFFF; st = 2; break;    /* Compress the no-case block if run is >= 128 */
                }
                st = 1;            /* Do not compress short run */
                /* go to next case */
            case 1:
                ch = si++;        /* Fill the short run */
                if (--j == 0) st = 0;
                break;

            default:
                ch = (WCHAR)j; si += j;    /* Number of chars to skip */
                st = 0;
            }
            sum = xsum32(buf[i + 0] = (BYTE)ch, sum);        /* Put it into the write buffer */
            sum = xsum32(buf[i + 1] = (BYTE)(ch >> 8), sum);
            i += 2; szb_case += 2;
            if (si == 0|| i == szb_buf) {        /* Write buffered data when buffer full or end of process */
                n = (i + ss - 1) / ss;
                if (disk_write(pdrv, buf, sect, n) != RES_OK) return FR_DISK_ERR;
                sect += n; i = 0;
            }
        } while (si);
        tbl[1] = (szb_case + au * ss - 1) / (au * ss);    /* Number of up-case table clusters */
        tbl[2] = 1;                                        /* Number of root dir clusters */

        /* Initialize the allocation bitmap */
        sect = b_data; nsect = (szb_bit + ss - 1) / ss;    /* Start of bitmap and number of sectors */
        nb = tbl[0] + tbl[1] + tbl[2];                    /* Number of clusters in-use by system */
        do {
            MEMSET(buf, 0, szb_buf);
            for (i = 0; nb >= 8 && i < szb_buf; buf[i++] = 0xFF, nb -= 8) ;
            for (b = 1; nb != 0 && i < szb_buf; buf[i] |= b, b <<= 1, nb--) ;
            n = (nsect > sz_buf) ? sz_buf : nsect;        /* Write the buffered data */
            if (disk_write(pdrv, buf, sect, n) != RES_OK) return FR_DISK_ERR;
            sect += n; nsect -= n;
        } while (nsect);

        /* Initialize the FAT */
        sect = b_fat; nsect = sz_fat;    /* Start of FAT and number of FAT sectors */
        j = nb = cl = 0;
        do {
            MEMSET(buf, 0, szb_buf); i = 0;    /* Clear work area and reset write index */
            if (cl == 0) {    /* Set entry 0 and 1 */
                st_dword(buf + i, 0xFFFFFFF8); i += 4; cl++;
                st_dword(buf + i, 0xFFFFFFFF); i += 4; cl++;
            }
            do {            /* Create chains of bitmap, up-case and root dir */
                while (nb != 0 && i < szb_buf) {            /* Create a chain */
                    st_dword(buf + i, (nb > 1) ? cl + 1 : 0xFFFFFFFF);
                    i += 4; cl++; nb--;
                }
                if (nb == 0 && j < 3) nb = tbl[j++];    /* Next chain */
            } while (nb != 0 && i < szb_buf);
            n = (nsect > sz_buf) ? sz_buf : nsect;    /* Write the buffered data */
            if (disk_write(pdrv, buf, sect, n) != RES_OK) return FR_DISK_ERR;
            sect += n; nsect -= n;
        } while (nsect);

        /* Initialize the root directory */
        MEMSET(buf, 0, szb_buf);
        buf[SZDIRE * 0 + 0] = 0x83;        /* 83 entry (volume label) */
        buf[SZDIRE * 1 + 0] = 0x81;        /* 81 entry (allocation bitmap) */
        st_dword(buf + SZDIRE * 1 + 20, 2);
        st_dword(buf + SZDIRE * 1 + 24, szb_bit);
        buf[SZDIRE * 2 + 0] = 0x82;        /* 82 entry (up-case table) */
        st_dword(buf + SZDIRE * 2 + 4, sum);
        st_dword(buf + SZDIRE * 2 + 20, 2 + tbl[0]);
        st_dword(buf + SZDIRE * 2 + 24, szb_case);
        sect = b_data + au * (tbl[0] + tbl[1]);    nsect = au;    /* Start of the root directory and number of sectors */
        do {    /* Fill root directory sectors */
            n = (nsect > sz_buf) ? sz_buf : nsect;
            if (disk_write(pdrv, buf, sect, n) != RES_OK) return FR_DISK_ERR;
            MEMSET(buf, 0, ss);
            sect += n; nsect -= n;
        } while (nsect);

        /* Create two set of the exFAT VBR blocks */
        sect = b_vol;
        for (n = 0; n < 2; n++) {
            /* Main record (+0) */
            MEMSET(buf, 0, ss);
            MEMCPY(buf + BS_JmpBoot, "\xEB\x76\x90" "EXFAT   ", 11);    /* Boot jump code (x86), OEM name */
            st_dword(buf + BPB_VolOfsEx, b_vol);                    /* Volume offset in the physical drive [sector] */
            st_dword(buf + BPB_TotSecEx, sz_vol);                    /* Volume size [sector] */
            st_dword(buf + BPB_FatOfsEx, b_fat - b_vol);            /* FAT offset [sector] */
            st_dword(buf + BPB_FatSzEx, sz_fat);                    /* FAT size [sector] */
            st_dword(buf + BPB_DataOfsEx, b_data - b_vol);            /* Data offset [sector] */
            st_dword(buf + BPB_NumClusEx, n_clst);                    /* Number of clusters */
            st_dword(buf + BPB_RootClusEx, 2 + tbl[0] + tbl[1]);    /* Root dir cluster # */
            st_dword(buf + BPB_VolIDEx, GET_FATTIME());                /* VSN */
            st_word(buf + BPB_FSVerEx, 0x100);                        /* Filesystem version (1.00) */
            for (buf[BPB_BytsPerSecEx] = 0, i = ss; i >>= 1; buf[BPB_BytsPerSecEx]++) ;    /* Log2 of sector size [byte] */
            for (buf[BPB_SecPerClusEx] = 0, i = au; i >>= 1; buf[BPB_SecPerClusEx]++) ;    /* Log2 of cluster size [sector] */
            buf[BPB_NumFATsEx] = 1;                    /* Number of FATs */
            buf[BPB_DrvNumEx] = 0x80;                /* Drive number (for int13) */
            st_word(buf + BS_BootCodeEx, 0xFEEB);    /* Boot code (x86) */
            st_word(buf + BS_55AA, 0xAA55);            /* Signature (placed here regardless of sector size) */
            for (i = sum = 0; i < ss; i++) {        /* VBR checksum */
                if (i != BPB_VolFlagEx && i != BPB_VolFlagEx + 1 && i != BPB_PercInUseEx) sum = xsum32(buf[i], sum);
            }
            if (disk_write(pdrv, buf, sect++, 1) != RES_OK) return FR_DISK_ERR;
            /* Extended bootstrap record (+1..+8) */
            MEMSET(buf, 0, ss);
            st_word(buf + ss - 2, 0xAA55);    /* Signature (placed at end of sector) */
            for (j = 1; j < 9; j++) {
                for (i = 0; i < ss; sum = xsum32(buf[i++], sum)) ;    /* VBR checksum */
                if (disk_write(pdrv, buf, sect++, 1) != RES_OK) return FR_DISK_ERR;
            }
            /* OEM/Reserved record (+9..+10) */
            MEMSET(buf, 0, ss);
            for ( ; j < 11; j++) {
                for (i = 0; i < ss; sum = xsum32(buf[i++], sum)) ;    /* VBR checksum */
                if (disk_write(pdrv, buf, sect++, 1) != RES_OK) return FR_DISK_ERR;
            }
            /* Sum record (+11) */
            for (i = 0; i < ss; i += 4) st_dword(buf + i, sum);        /* Fill with checksum value */
            if (disk_write(pdrv, buf, sect++, 1) != RES_OK) return FR_DISK_ERR;
        }

    } else
#endif    /* FF_FS_EXFAT */
    {    /* Create an FAT/FAT32 volume */
        do {
            pau = au;
            /* Pre-determine number of clusters and FAT sub-type */
            if (fmt == FS_FAT32) {    /* FAT32 volume */
                if (pau == 0) {    /* au auto-selection */
                    n = sz_vol / 0x20000;    /* Volume size in unit of 128KS */
                    for (i = 0, pau = 1; cst32[i] && cst32[i] <= n; i++, pau <<= 1) ;    /* Get from table */
                }
                n_clst = sz_vol / pau;    /* Number of clusters */
                sz_fat = (n_clst * 4 + 8 + ss - 1) / ss;    /* FAT size [sector] */
                sz_rsv = 32;    /* Number of reserved sectors */
                sz_dir = 0;        /* No static directory */
                if (n_clst <= MAX_FAT16 || n_clst > MAX_FAT32) return FR_MKFS_ABORTED;
            } else {                /* FAT volume */
                if (pau == 0) {    /* au auto-selection */
                    n = sz_vol / 0x1000;    /* Volume size in unit of 4KS */
                    for (i = 0, pau = 1; cst[i] && cst[i] <= n; i++, pau <<= 1) ;    /* Get from table */
                }
                n_clst = sz_vol / pau;
                if (n_clst > MAX_FAT12) {
                    n = n_clst * 2 + 4;        /* FAT size [byte] */
                } else {
                    fmt = FS_FAT12;
                    n = (n_clst * 3 + 1) / 2 + 3;    /* FAT size [byte] */
                }
                sz_fat = (n + ss - 1) / ss;        /* FAT size [sector] */
                sz_rsv = 1;                        /* Number of reserved sectors */
                sz_dir = (DWORD)n_rootdir * SZDIRE / ss;    /* Rootdir size [sector] */
            }
            b_fat = b_vol + sz_rsv;                        /* FAT base */
            b_data = b_fat + sz_fat * n_fats + sz_dir;    /* Data base */

            /* Align data base to erase block boundary (for flash memory media) */
            n = ((b_data + sz_blk - 1) & ~(sz_blk - 1)) - b_data;    /* Next nearest erase block from current data base */
            if (fmt == FS_FAT32) {        /* FAT32: Move FAT base */
                sz_rsv += n; b_fat += n;
            } else {                    /* FAT: Expand FAT size */
                sz_fat += n / n_fats;
            }

            /* Determine number of clusters and final check of validity of the FAT sub-type */
            if (sz_vol < b_data + pau * 16 - b_vol) return FR_MKFS_ABORTED;    /* Too small volume */
            n_clst = (sz_vol - sz_rsv - sz_fat * n_fats - sz_dir) / pau;
            if (fmt == FS_FAT32) {
                if (n_clst <= MAX_FAT16) {    /* Too few clusters for FAT32 */
                    if (au == 0 && (au = pau / 2) != 0) continue;    /* Adjust cluster size and retry */
                    return FR_MKFS_ABORTED;
                }
            }
            if (fmt == FS_FAT16) {
                if (n_clst > MAX_FAT16) {    /* Too many clusters for FAT16 */
                    if (au == 0 && (pau * 2) <= 64) {
                        au = pau * 2; continue;        /* Adjust cluster size and retry */
                    }
                    if ((opt & FM_FAT32)) {
                        fmt = FS_FAT32; continue;    /* Switch type to FAT32 and retry */
                    }
                    if (au == 0 && (au = pau * 2) <= 128) continue;    /* Adjust cluster size and retry */
                    return FR_MKFS_ABORTED;
                }
                if  (n_clst <= MAX_FAT12) {    /* Too few clusters for FAT16 */
                    if (au == 0 && (au = pau * 2) <= 128) continue;    /* Adjust cluster size and retry */
                    return FR_MKFS_ABORTED;
                }
            }
            if (fmt == FS_FAT12 && n_clst > MAX_FAT12) return FR_MKFS_ABORTED;    /* Too many clusters for FAT12 */

            /* Ok, it is the valid cluster configuration */
            break;
        } while (1);

#if FF_USE_TRIM
        tbl[0] = b_vol; tbl[1] = b_vol + sz_vol - 1;    /* Inform the device the volume area can be erased */
        disk_ioctl(pdrv, CTRL_TRIM, tbl);
#endif
        /* Create FAT VBR */
        MEMSET(buf, 0, ss);
        MEMCPY(buf + BS_JmpBoot, "\xEB\xFE\x90" "MSDOS5.0", 11);/* Boot jump code (x86), OEM name */
        st_word(buf + BPB_BytsPerSec, ss);                /* Sector size [byte] */
        buf[BPB_SecPerClus] = (BYTE)pau;                /* Cluster size [sector] */
        st_word(buf + BPB_RsvdSecCnt, (WORD)sz_rsv);    /* Size of reserved area */
        buf[BPB_NumFATs] = (BYTE)n_fats;                /* Number of FATs */
        st_word(buf + BPB_RootEntCnt, (WORD)((fmt == FS_FAT32) ? 0 : n_rootdir));    /* Number of root directory entries */
        if (sz_vol < 0x10000) {
            st_word(buf + BPB_TotSec16, (WORD)sz_vol);    /* Volume size in 16-bit LBA */
        } else {
            st_dword(buf + BPB_TotSec32, sz_vol);        /* Volume size in 32-bit LBA */
        }
        buf[BPB_Media] = 0xF8;                            /* Media descriptor byte */
        st_word(buf + BPB_SecPerTrk, 63);                /* Number of sectors per track (for int13) */
        st_word(buf + BPB_NumHeads, 255);                /* Number of heads (for int13) */
        st_dword(buf + BPB_HiddSec, b_vol);                /* Volume offset in the physical drive [sector] */
        if (fmt == FS_FAT32) {
            st_dword(buf + BS_VolID32, GET_FATTIME());    /* VSN */
            st_dword(buf + BPB_FATSz32, sz_fat);        /* FAT size [sector] */
            st_dword(buf + BPB_RootClus32, 2);            /* Root directory cluster # (2) */
            st_word(buf + BPB_FSInfo32, 1);                /* Offset of FSINFO sector (VBR + 1) */
            st_word(buf + BPB_BkBootSec32, 6);            /* Offset of backup VBR (VBR + 6) */
            buf[BS_DrvNum32] = 0x80;                    /* Drive number (for int13) */
            buf[BS_BootSig32] = 0x29;                    /* Extended boot signature */
            MEMCPY(buf + BS_VolLab32, "NO NAME    " "FAT32   ", 19);    /* Volume label, FAT signature */
        } else {
            st_dword(buf + BS_VolID, GET_FATTIME());    /* VSN */
            st_word(buf + BPB_FATSz16, (WORD)sz_fat);    /* FAT size [sector] */
            buf[BS_DrvNum] = 0x80;                        /* Drive number (for int13) */
            buf[BS_BootSig] = 0x29;                        /* Extended boot signature */
            MEMCPY(buf + BS_VolLab, "NO NAME    " "FAT     ", 19);    /* Volume label, FAT signature */
        }
        st_word(buf + BS_55AA, 0xAA55);                    /* Signature (offset is fixed here regardless of sector size) */
        if (disk_write(pdrv, buf, b_vol, 1) != RES_OK) return FR_DISK_ERR;    /* Write it to the VBR sector */

        /* Create FSINFO record if needed */
        if (fmt == FS_FAT32) {
            disk_write(pdrv, buf, b_vol + 6, 1);        /* Write backup VBR (VBR + 6) */
            MEMSET(buf, 0, ss);
            st_dword(buf + FSI_LeadSig, 0x41615252);
            st_dword(buf + FSI_StrucSig, 0x61417272);
            st_dword(buf + FSI_Free_Count, n_clst - 1);    /* Number of free clusters */
            st_dword(buf + FSI_Nxt_Free, 2);            /* Last allocated cluster# */
            st_word(buf + BS_55AA, 0xAA55);
            disk_write(pdrv, buf, b_vol + 7, 1);        /* Write backup FSINFO (VBR + 7) */
            disk_write(pdrv, buf, b_vol + 1, 1);        /* Write original FSINFO (VBR + 1) */
        }

        /* Initialize FAT area */
        MEMSET(buf, 0, (UINT)szb_buf);
        sect = b_fat;        /* FAT start sector */
        for (i = 0; i < n_fats; i++) {            /* Initialize FATs each */
            if (fmt == FS_FAT32) {
                st_dword(buf + 0, 0xFFFFFFF8);    /* Entry 0 */
                st_dword(buf + 4, 0xFFFFFFFF);    /* Entry 1 */
                st_dword(buf + 8, 0x0FFFFFFF);    /* Entry 2 (root directory) */
            } else {
                st_dword(buf + 0, (fmt == FS_FAT12) ? 0xFFFFF8 : 0xFFFFFFF8);    /* Entry 0 and 1 */
            }
            nsect = sz_fat;        /* Number of FAT sectors */
            do {    /* Fill FAT sectors */
                n = (nsect > sz_buf) ? sz_buf : nsect;
                if (disk_write(pdrv, buf, sect, (UINT)n) != RES_OK) return FR_DISK_ERR;
                MEMSET(buf, 0, ss);
                sect += n; nsect -= n;
            } while (nsect);
        }

        /* Initialize root directory (fill with zero) */
        nsect = (fmt == FS_FAT32) ? pau : sz_dir;    /* Number of root directory sectors */
        do {
            n = (nsect > sz_buf) ? sz_buf : nsect;
            if (disk_write(pdrv, buf, sect, (UINT)n) != RES_OK) return FR_DISK_ERR;
            sect += n; nsect -= n;
        } while (nsect);
    }

    /* Determine system ID in the partition table */
    if (FF_FS_EXFAT && fmt == FS_EXFAT) {
        sys = 0x07;            /* HPFS/NTFS/exFAT */
    } else {
        if (fmt == FS_FAT32) {
            sys = 0x0C;        /* FAT32X */
        } else {
            if (sz_vol >= 0x10000) {
                sys = 0x06;    /* FAT12/16 (large) */
            } else {
                sys = (fmt == FS_FAT16) ? 0x04 : 0x01;    /* FAT16 : FAT12 */
            }
        }
    }

    /* Update partition information */
    if (FF_MULTI_PARTITION && part != 0) {    /* Created in the existing partition */
        /* Update system ID in the partition table */
        if (disk_read(pdrv, buf, 0, 1) != RES_OK) return FR_DISK_ERR;    /* Read the MBR */
        buf[MBR_Table + (part - 1) * SZ_PTE + PTE_System] = sys;        /* Set system ID */
        if (disk_write(pdrv, buf, 0, 1) != RES_OK) return FR_DISK_ERR;    /* Write it back to the MBR */
    } else {                                /* Created as a new single partition */
        if (!(opt & FM_SFD)) {    /* Create partition table if in FDISK format */
            MEMSET(buf, 0, ss);
            st_word(buf + BS_55AA, 0xAA55);        /* MBR signature */
            pte = buf + MBR_Table;                /* Create partition table for single partition in the drive */
            pte[PTE_Boot] = 0;                    /* Boot indicator */
            pte[PTE_StHead] = 1;                /* Start head */
            pte[PTE_StSec] = 1;                    /* Start sector */
            pte[PTE_StCyl] = 0;                    /* Start cylinder */
            pte[PTE_System] = sys;                /* System type */
            n = (b_vol + sz_vol) / (63 * 255);    /* (End CHS may be invalid) */
            pte[PTE_EdHead] = 254;                /* End head */
            pte[PTE_EdSec] = (BYTE)(((n >> 2) & 0xC0) | 63);    /* End sector */
            pte[PTE_EdCyl] = (BYTE)n;            /* End cylinder */
            st_dword(pte + PTE_StLba, b_vol);    /* Start offset in LBA */
            st_dword(pte + PTE_SizLba, sz_vol);    /* Size in sectors */
            if (disk_write(pdrv, buf, 0, 1) != RES_OK) return FR_DISK_ERR;    /* Write it to the MBR */
        }
    }

    if (disk_ioctl(pdrv, CTRL_SYNC, 0) != RES_OK) return FR_DISK_ERR;

    return FR_OK;
}



#if FF_MULTI_PARTITION
/*-----------------------------------------------------------------------*/
/* Create Partition Table on the Physical Drive                          */
/*-----------------------------------------------------------------------*/

FRESULT f_fdisk (
    BYTE pdrv,            /* Physical drive number */
    const DWORD* szt,    /* Pointer to the size table for each partitions */
    void* work            /* Pointer to the working buffer */
)
{
    UINT i, n, sz_cyl, tot_cyl, b_cyl, e_cyl, p_cyl;
    BYTE s_hd, e_hd, *p, *buf = (BYTE*)work;
    DSTATUS stat;
    DWORD sz_disk, sz_part, s_part;


    stat = disk_initialize(pdrv);
    if (stat & STA_NOINIT) return FR_NOT_READY;
    if (stat & STA_PROTECT) return FR_WRITE_PROTECTED;
    if (disk_ioctl(pdrv, GET_SECTOR_COUNT, &sz_disk)) return FR_DISK_ERR;

    /* Determine the CHS without any consideration of the drive geometry */
    for (n = 16; n < 256 && sz_disk / n / 63 > 1024; n *= 2) ;
    if (n == 256) n--;
    e_hd = n - 1;
    sz_cyl = 63 * n;
    tot_cyl = sz_disk / sz_cyl;

    /* Create partition table */
    MEMSET(buf, 0, FF_MAX_SS);
    p = buf + MBR_Table; b_cyl = 0;
    for (i = 0; i < 4; i++, p += SZ_PTE) {
        p_cyl = (szt[i] <= 100U) ? (DWORD)tot_cyl * szt[i] / 100 : szt[i] / sz_cyl;    /* Number of cylinders */
        if (p_cyl == 0) continue;
        s_part = (DWORD)sz_cyl * b_cyl;
        sz_part = (DWORD)sz_cyl * p_cyl;
        if (i == 0) {    /* Exclude first track of cylinder 0 */
            s_hd = 1;
            s_part += 63; sz_part -= 63;
        } else {
            s_hd = 0;
        }
        e_cyl = b_cyl + p_cyl - 1;    /* End cylinder */
        if (e_cyl >= tot_cyl) return FR_INVALID_PARAMETER;

        /* Set partition table */
        p[1] = s_hd;                        /* Start head */
        p[2] = (BYTE)(((b_cyl >> 2) & 0xC0) | 1);    /* Start sector and cylinder high */
        p[3] = (BYTE)b_cyl;                    /* Start cylinder low */
        p[4] = 0x07;                        /* System type (temporary setting) */
        p[5] = e_hd;                        /* End head */
        p[6] = (BYTE)(((e_cyl >> 2) & 0xC0) | 63);    /* End sector and cylinder high */
        p[7] = (BYTE)e_cyl;                    /* End cylinder low */
        st_dword(p + 8, s_part);            /* Start sector in LBA */
        st_dword(p + 12, sz_part);            /* Number of sectors */

        /* Next partition */
        b_cyl += p_cyl;
    }
    st_word(p, 0xAA55);

    /* Write it to the MBR */
    return (disk_write(pdrv, buf, 0, 1) != RES_OK || disk_ioctl(pdrv, CTRL_SYNC, 0) != RES_OK) ? FR_DISK_ERR : FR_OK;
}

#endif /* FF_MULTI_PARTITION */
#endif /* FF_USE_MKFS && !FF_FS_READONLY */




#if FF_USE_STRFUNC
/*-----------------------------------------------------------------------*/
/* Get a String from the File                                            */
/*-----------------------------------------------------------------------*/

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




#if !FF_FS_READONLY
#include <stdarg.h>
/*-----------------------------------------------------------------------*/
/* Put a Character to the File                                           */
/*-----------------------------------------------------------------------*/

typedef struct {
    FIL *fp;        /* Ptr to the writing file */
    int idx, nchr;    /* Write index of buf[] (-1:error), number of chars written */
    BYTE buf[64];    /* Write buffer */
} putbuff;


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



int f_putc (
    TCHAR c,    /* A character to be output */
    FIL* fp        /* Pointer to the file object */
)
{
    putbuff pb;


    putc_init(&pb, fp);
    putc_bfd(&pb, c);    /* Put the character */
    return putc_flush(&pb);
}




/*-----------------------------------------------------------------------*/
/* Put a String to the File                                              */
/*-----------------------------------------------------------------------*/

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




/*-----------------------------------------------------------------------*/
/* Put a Formatted String to the File                                    */
/*-----------------------------------------------------------------------*/

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

#endif /* !FF_FS_READONLY */
#endif /* FF_USE_STRFUNC */



#if FF_CODE_PAGE == 0
/*-----------------------------------------------------------------------*/
/* Set Active Codepage for the Path Name                                 */
/*-----------------------------------------------------------------------*/

FRESULT f_setcp (
    WORD cp        /* Value to be set as active code page */
)
{
    static const WORD       validcp[] = {  437,   720,   737,   771,   775,   850,   852,   857,   860,   861,   862,   863,   864,   865,   866,   869,   932,   936,   949,   950, 0};
    static const BYTE *const tables[] = {Ct437, Ct720, Ct737, Ct771, Ct775, Ct850, Ct852, Ct857, Ct860, Ct861, Ct862, Ct863, Ct864, Ct865, Ct866, Ct869, Dc932, Dc936, Dc949, Dc950, 0};
    UINT i;


    for (i = 0; validcp[i] != 0 && validcp[i] != cp; i++) ;    /* Find the code page */
    if (validcp[i] != cp) return FR_INVALID_PARAMETER;

    CodePage = cp;
    if (cp >= 900) {    /* DBCS */
        ExCvt = 0;
        DbcTbl = tables[i];
    } else {            /* SBCS */
        ExCvt = tables[i];
        DbcTbl = 0;
    }
    return FR_OK;
}
#endif    /* FF_CODE_PAGE == 0 */


