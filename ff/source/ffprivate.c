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
/* LFN/Directory working buffer                                          */
/*-----------------------------------------------------------------------*/

#if FF_USE_LFN != 0             /* LFN configurations */
extern const BYTE LfnOfs[];     /* FAT: Offset of LFN characters in the directory entry */
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
/* Load/Store multi-byte word in the FAT structure                       */
/*-----------------------------------------------------------------------*/

WORD ld_word (const BYTE* ptr)    /*     Load a 2-byte little-endian word */
{
    WORD rv;

    rv = ptr[1];
    rv = rv << 8 | ptr[0];
    return rv;
}

DWORD ld_dword (const BYTE* ptr)    /* Load a 4-byte little-endian word */
{
    DWORD rv;

    rv = ptr[3];
    rv = rv << 8 | ptr[2];
    rv = rv << 8 | ptr[1];
    rv = rv << 8 | ptr[0];
    return rv;
}

#if FF_FS_EXFAT
QWORD ld_qword (const BYTE* ptr)    /* Load an 8-byte little-endian word */
{
    QWORD rv;

    rv = ptr[7];
    rv = rv << 8 | ptr[6];
    rv = rv << 8 | ptr[5];
    rv = rv << 8 | ptr[4];
    rv = rv << 8 | ptr[3];
    rv = rv << 8 | ptr[2];
    rv = rv << 8 | ptr[1];
    rv = rv << 8 | ptr[0];
    return rv;
}
#endif

#if !FF_FS_READONLY
void st_word (BYTE* ptr, WORD val)    /* Store a 2-byte word in little-endian */
{
    *ptr++ = (BYTE)val; val >>= 8;
    *ptr++ = (BYTE)val;
}

void st_dword (BYTE* ptr, DWORD val)    /* Store a 4-byte word in little-endian */
{
    *ptr++ = (BYTE)val; val >>= 8;
    *ptr++ = (BYTE)val; val >>= 8;
    *ptr++ = (BYTE)val; val >>= 8;
    *ptr++ = (BYTE)val;
}

#if FF_FS_EXFAT
void st_qword (BYTE* ptr, QWORD val)    /* Store an 8-byte word in little-endian */
{
    *ptr++ = (BYTE)val; val >>= 8;
    *ptr++ = (BYTE)val; val >>= 8;
    *ptr++ = (BYTE)val; val >>= 8;
    *ptr++ = (BYTE)val; val >>= 8;
    *ptr++ = (BYTE)val; val >>= 8;
    *ptr++ = (BYTE)val; val >>= 8;
    *ptr++ = (BYTE)val; val >>= 8;
    *ptr++ = (BYTE)val;
}
#endif
#endif    /* !FF_FS_READONLY */



/*-----------------------------------------------------------------------*/
/* String functions                                                      */
/*-----------------------------------------------------------------------*/

/* Copy memory to memory */
void mem_cpy (void* dst, const void* src, UINT cnt)
{
    BYTE *d = (BYTE*)dst;
    const BYTE *s = (const BYTE*)src;

    if (cnt != 0) {
        do {
            *d++ = *s++;
        } while (--cnt);
    }
}


/* Fill memory block */
void mem_set (void* dst, int val, UINT cnt)
{
    BYTE *d = (BYTE*)dst;

    do {
        *d++ = (BYTE)val;
    } while (--cnt);
}


/* Compare memory block */
int mem_cmp (const void* dst, const void* src, UINT cnt)    /* ZR:same, NZ:different */
{
    const BYTE *d = (const BYTE *)dst, *s = (const BYTE *)src;
    int r = 0;

    do {
        r = *d++ - *s++;
    } while (--cnt && r == 0);

    return r;
}


/* Check if chr is contained in the string */
int chk_chr (const char* str, int chr)    /* NZ:contained, ZR:not contained */
{
    while (*str && *str != chr) str++;
    return *str;
}



/*-----------------------------------------------------------------------*/
/* Code Page functions                                                   */
/*-----------------------------------------------------------------------*/

/* Test if the character is DBC 1st byte */
int dbc_1st (BYTE c)
{
#if FF_CODE_PAGE == 0        /* Variable code page */
    if (DbcTbl && c >= DbcTbl[0]) {
        if (c <= DbcTbl[1]) return 1;                    /* 1st byte range 1 */
        if (c >= DbcTbl[2] && c <= DbcTbl[3]) return 1;    /* 1st byte range 2 */
    }
#elif FF_CODE_PAGE >= 900    /* DBCS fixed code page */
    if (c >= DbcTbl[0]) {
        if (c <= DbcTbl[1]) return 1;
        if (c >= DbcTbl[2] && c <= DbcTbl[3]) return 1;
    }
#else                    /* SBCS fixed code page */
    if (c) return 0;    /* Always false */
#endif
    return 0;
}


/* Test if the character is DBC 2nd byte */
int dbc_2nd (BYTE c)
{
#if FF_CODE_PAGE == 0        /* Variable code page */
    if (DbcTbl && c >= DbcTbl[4]) {
        if (c <= DbcTbl[5]) return 1;                    /* 2nd byte range 1 */
        if (c >= DbcTbl[6] && c <= DbcTbl[7]) return 1;    /* 2nd byte range 2 */
        if (c >= DbcTbl[8] && c <= DbcTbl[9]) return 1;    /* 2nd byte range 3 */
    }
#elif FF_CODE_PAGE >= 900    /* DBCD fixed code page */
    if (c >= DbcTbl[4]) {
        if (c <= DbcTbl[5]) return 1;
        if (c >= DbcTbl[6] && c <= DbcTbl[7]) return 1;
        if (c >= DbcTbl[8] && c <= DbcTbl[9]) return 1;
    }
#else                    /* SBCS fixed code page */
    if (c) return 0;    /* Always false */
#endif
    return 0;
}


#if FF_FS_REENTRANT
/*-----------------------------------------------------------------------*/
/* Request/Release grant to access the volume                            */
/*-----------------------------------------------------------------------*/
int lock_fs (        /* 1:Ok, 0:timeout */
    FATFS* fs        /* Filesystem object */
)
{
    return ff_req_grant(fs->sobj);
}


void unlock_fs (
    FATFS* fs,        /* Filesystem object */
    FRESULT res       /* Result code to be returned */
)
{
    if (fs && res != FR_NOT_ENABLED && res != FR_INVALID_DRIVE && res != FR_TIMEOUT) {
        ff_rel_grant(fs->sobj);
    }
}

#endif



#if FF_FS_LOCK != 0
/*-----------------------------------------------------------------------*/
/* File lock control functions                                           */
/*-----------------------------------------------------------------------*/

FRESULT chk_lock (    /* Check if the file can be accessed */
    DIR* dp,        /* Directory object pointing the file to be checked */
    int acc            /* Desired access type (0:Read mode open, 1:Write mode open, 2:Delete or rename) */
)
{
    UINT i, be;

    /* Search open object table for the object */
    be = 0;
    for (i = 0; i < FF_FS_LOCK; i++) {
        if (Files[i].fs) {    /* Existing entry */
            if (Files[i].fs == dp->obj.fs &&         /* Check if the object matches with an open object */
                Files[i].clu == dp->obj.sclust &&
                Files[i].ofs == dp->dptr) break;
        } else {            /* Blank entry */
            be = 1;
        }
    }
    if (i == FF_FS_LOCK) {    /* The object has not been opened */
        return (!be && acc != 2) ? FR_TOO_MANY_OPEN_FILES : FR_OK;    /* Is there a blank entry for new object? */
    }

    /* The object was opened. Reject any open against writing file and all write mode open */
    return (acc != 0 || Files[i].ctr == 0x100) ? FR_LOCKED : FR_OK;
}


int enq_lock (void)    /* Check if an entry is available for a new object */
{
    UINT i;

    for (i = 0; i < FF_FS_LOCK && Files[i].fs; i++) ;
    return (i == FF_FS_LOCK) ? 0 : 1;
}


UINT inc_lock (    /* Increment object open counter and returns its index (0:Internal error) */
    DIR* dp,    /* Directory object pointing the file to register or increment */
    int acc        /* Desired access (0:Read, 1:Write, 2:Delete/Rename) */
)
{
    UINT i;


    for (i = 0; i < FF_FS_LOCK; i++) {    /* Find the object */
        if (Files[i].fs == dp->obj.fs &&
            Files[i].clu == dp->obj.sclust &&
            Files[i].ofs == dp->dptr) break;
    }

    if (i == FF_FS_LOCK) {                /* Not opened. Register it as new. */
        for (i = 0; i < FF_FS_LOCK && Files[i].fs; i++) ;
        if (i == FF_FS_LOCK) return 0;    /* No free entry to register (int err) */
        Files[i].fs = dp->obj.fs;
        Files[i].clu = dp->obj.sclust;
        Files[i].ofs = dp->dptr;
        Files[i].ctr = 0;
    }

    if (acc && Files[i].ctr) return 0;    /* Access violation (int err) */

    Files[i].ctr = acc ? 0x100 : Files[i].ctr + 1;    /* Set semaphore value */

    return i + 1;    /* Index number origin from 1 */
}


FRESULT dec_lock (    /* Decrement object open counter */
    UINT i            /* Semaphore index (1..) */
)
{
    WORD n;
    FRESULT res;


    if (--i < FF_FS_LOCK) {    /* Index number origin from 0 */
        n = Files[i].ctr;
        if (n == 0x100) n = 0;        /* If write mode open, delete the entry */
        if (n > 0) n--;                /* Decrement read mode open count */
        Files[i].ctr = n;
        if (n == 0) Files[i].fs = 0;    /* Delete the entry if open count gets zero */
        res = FR_OK;
    } else {
        res = FR_INT_ERR;            /* Invalid index nunber */
    }
    return res;
}


void clear_lock (    /* Clear lock entries of the volume */
    FATFS *fs
)
{
    UINT i;

    for (i = 0; i < FF_FS_LOCK; i++) {
        if (Files[i].fs == fs) Files[i].fs = 0;
    }
}

#endif    /* FF_FS_LOCK != 0 */



/*-----------------------------------------------------------------------*/
/* Move/Flush disk access window in the filesystem object                */
/*-----------------------------------------------------------------------*/
#if !FF_FS_READONLY
FRESULT sync_window (    /* Returns FR_OK or FR_DISK_ERR */
    FATFS* fs            /* Filesystem object */
)
{
    FRESULT res = FR_OK;


    if (fs->wflag) {    /* Is the disk access window dirty */
        if (disk_write(fs->pdrv, fs->win, fs->winsect, 1) == RES_OK) {    /* Write back the window */
            fs->wflag = 0;    /* Clear window dirty flag */
            if (fs->winsect - fs->fatbase < fs->fsize) {    /* Is it in the 1st FAT? */
                if (fs->n_fats == 2) disk_write(fs->pdrv, fs->win, fs->winsect + fs->fsize, 1);    /* Reflect it to 2nd FAT if needed */
            }
        } else {
            res = FR_DISK_ERR;
        }
    }
    return res;
}
#endif


FRESULT move_window (    /* Returns FR_OK or FR_DISK_ERR */
    FATFS* fs,            /* Filesystem object */
    DWORD sector        /* Sector number to make appearance in the fs->win[] */
)
{
    FRESULT res = FR_OK;


    if (sector != fs->winsect) {    /* Window offset changed? */
#if !FF_FS_READONLY
        res = sync_window(fs);        /* Write-back changes */
#endif
        if (res == FR_OK) {            /* Fill sector window with new data */
            if (disk_read(fs->pdrv, fs->win, sector, 1) != RES_OK) {
                sector = 0xFFFFFFFF;    /* Invalidate window if read data is not valid */
                res = FR_DISK_ERR;
            }
            fs->winsect = sector;
        }
    }
    return res;
}




#if !FF_FS_READONLY
/*-----------------------------------------------------------------------*/
/* Synchronize filesystem and data on the storage                        */
/*-----------------------------------------------------------------------*/

FRESULT sync_fs (    /* Returns FR_OK or FR_DISK_ERR */
    FATFS* fs        /* Filesystem object */
)
{
    FRESULT res;


    res = sync_window(fs);
    if (res == FR_OK) {
        if (fs->fs_type == FS_FAT32 && fs->fsi_flag == 1) {    /* FAT32: Update FSInfo sector if needed */
            /* Create FSInfo structure */
            MEMSET(fs->win, 0, SS(fs));
            st_word(fs->win + BS_55AA, 0xAA55);
            st_dword(fs->win + FSI_LeadSig, 0x41615252);
            st_dword(fs->win + FSI_StrucSig, 0x61417272);
            st_dword(fs->win + FSI_Free_Count, fs->free_clst);
            st_dword(fs->win + FSI_Nxt_Free, fs->last_clst);
            /* Write it into the FSInfo sector */
            fs->winsect = fs->volbase + 1;
            disk_write(fs->pdrv, fs->win, fs->winsect, 1);
            fs->fsi_flag = 0;
        }
        /* Make sure that no pending write process in the lower layer */
        if (disk_ioctl(fs->pdrv, CTRL_SYNC, 0) != RES_OK) res = FR_DISK_ERR;
    }

    return res;
}

#endif



/*-----------------------------------------------------------------------*/
/* Get physical sector number from cluster number                        */
/*-----------------------------------------------------------------------*/

DWORD clst2sect (    /* !=0:Sector number, 0:Failed (invalid cluster#) */
    FATFS* fs,        /* Filesystem object */
    DWORD clst        /* Cluster# to be converted */
)
{
    clst -= 2;        /* Cluster number is origin from 2 */
    if (clst >= fs->n_fatent - 2) return 0;        /* Is it invalid cluster number? */
    return fs->database + fs->csize * clst;        /* Start sector number of the cluster */
}




/*-----------------------------------------------------------------------*/
/* FAT access - Read value of a FAT entry                                */
/*-----------------------------------------------------------------------*/

DWORD get_fat (        /* 0xFFFFFFFF:Disk error, 1:Internal error, 2..0x7FFFFFFF:Cluster status */
    FFOBJID* obj,    /* Corresponding object */
    DWORD clst        /* Cluster number to get the value */
)
{
    UINT wc, bc;
    DWORD val;
    FATFS *fs = obj->fs;


    if (clst < 2 || clst >= fs->n_fatent) {    /* Check if in valid range */
        val = 1;    /* Internal error */

    } else {
        val = 0xFFFFFFFF;    /* Default value falls on disk error */

        switch (fs->fs_type) {
        case FS_FAT12 :
            bc = (UINT)clst; bc += bc / 2;
            if (move_window(fs, fs->fatbase + (bc / SS(fs))) != FR_OK) break;
            wc = fs->win[bc++ % SS(fs)];        /* Get 1st byte of the entry */
            if (move_window(fs, fs->fatbase + (bc / SS(fs))) != FR_OK) break;
            wc |= fs->win[bc % SS(fs)] << 8;    /* Merge 2nd byte of the entry */
            val = (clst & 1) ? (wc >> 4) : (wc & 0xFFF);    /* Adjust bit position */
            break;

        case FS_FAT16 :
            if (move_window(fs, fs->fatbase + (clst / (SS(fs) / 2))) != FR_OK) break;
            val = ld_word(fs->win + clst * 2 % SS(fs));        /* Simple WORD array */
            break;

        case FS_FAT32 :
            if (move_window(fs, fs->fatbase + (clst / (SS(fs) / 4))) != FR_OK) break;
            val = ld_dword(fs->win + clst * 4 % SS(fs)) & 0x0FFFFFFF;    /* Simple DWORD array but mask out upper 4 bits */
            break;
#if FF_FS_EXFAT
        case FS_EXFAT :
            if (obj->objsize != 0) {
                DWORD cofs = clst - obj->sclust;    /* Offset from start cluster */
                DWORD clen = (DWORD)((obj->objsize - 1) / SS(fs)) / fs->csize;    /* Number of clusters - 1 */

                if (obj->stat == 2 && cofs <= clen) {    /* Is it a contiguous chain? */
                    val = (cofs == clen) ? 0x7FFFFFFF : clst + 1;    /* No data on the FAT, generate the value */
                    break;
                }
                if (obj->stat == 3 && cofs < obj->n_cont) {    /* Is it in the 1st fragment? */
                    val = clst + 1;     /* Generate the value */
                    break;
                }
                if (obj->stat != 2) {    /* Get value from FAT if FAT chain is valid */
                    if (obj->n_frag != 0) {    /* Is it on the growing edge? */
                        val = 0x7FFFFFFF;    /* Generate EOC */
                    } else {
                        if (move_window(fs, fs->fatbase + (clst / (SS(fs) / 4))) != FR_OK) break;
                        val = ld_dword(fs->win + clst * 4 % SS(fs)) & 0x7FFFFFFF;
                    }
                    break;
                }
            }
            /* go to default */
#endif
        default:
            val = 1;    /* Internal error */
        }
    }

    return val;
}




#if !FF_FS_READONLY
/*-----------------------------------------------------------------------*/
/* FAT access - Change value of a FAT entry                              */
/*-----------------------------------------------------------------------*/

FRESULT put_fat (    /* FR_OK(0):succeeded, !=0:error */
    FATFS* fs,        /* Corresponding filesystem object */
    DWORD clst,        /* FAT index number (cluster number) to be changed */
    DWORD val        /* New value to be set to the entry */
)
{
    UINT bc;
    BYTE *p;
    FRESULT res = FR_INT_ERR;


    if (clst >= 2 && clst < fs->n_fatent) {    /* Check if in valid range */
        switch (fs->fs_type) {
        case FS_FAT12 :
            bc = (UINT)clst; bc += bc / 2;    /* bc: byte offset of the entry */
            res = move_window(fs, fs->fatbase + (bc / SS(fs)));
            if (res != FR_OK) break;
            p = fs->win + bc++ % SS(fs);
            *p = (clst & 1) ? ((*p & 0x0F) | ((BYTE)val << 4)) : (BYTE)val;        /* Put 1st byte */
            fs->wflag = 1;
            res = move_window(fs, fs->fatbase + (bc / SS(fs)));
            if (res != FR_OK) break;
            p = fs->win + bc % SS(fs);
            *p = (clst & 1) ? (BYTE)(val >> 4) : ((*p & 0xF0) | ((BYTE)(val >> 8) & 0x0F));    /* Put 2nd byte */
            fs->wflag = 1;
            break;

        case FS_FAT16 :
            res = move_window(fs, fs->fatbase + (clst / (SS(fs) / 2)));
            if (res != FR_OK) break;
            st_word(fs->win + clst * 2 % SS(fs), (WORD)val);    /* Simple WORD array */
            fs->wflag = 1;
            break;

        case FS_FAT32 :
#if FF_FS_EXFAT
        case FS_EXFAT :
#endif
            res = move_window(fs, fs->fatbase + (clst / (SS(fs) / 4)));
            if (res != FR_OK) break;
            if (!FF_FS_EXFAT || fs->fs_type != FS_EXFAT) {
                val = (val & 0x0FFFFFFF) | (ld_dword(fs->win + clst * 4 % SS(fs)) & 0xF0000000);
            }
            st_dword(fs->win + clst * 4 % SS(fs), val);
            fs->wflag = 1;
            break;
        }
    }
    return res;
}

#endif /* !FF_FS_READONLY */




#if FF_FS_EXFAT && !FF_FS_READONLY
/*-----------------------------------------------------------------------*/
/* exFAT: Accessing FAT and Allocation Bitmap                            */
/*-----------------------------------------------------------------------*/

/*--------------------------------------*/
/* Find a contiguous free cluster block */
/*--------------------------------------*/

DWORD find_bitmap (    /* 0:Not found, 2..:Cluster block found, 0xFFFFFFFF:Disk error */
    FATFS* fs,    /* Filesystem object */
    DWORD clst,    /* Cluster number to scan from */
    DWORD ncl    /* Number of contiguous clusters to find (1..) */
)
{
    BYTE bm, bv;
    UINT i;
    DWORD val, scl, ctr;


    clst -= 2;    /* The first bit in the bitmap corresponds to cluster #2 */
    if (clst >= fs->n_fatent - 2) clst = 0;
    scl = val = clst; ctr = 0;
    for (;;) {
        if (move_window(fs, fs->database + val / 8 / SS(fs)) != FR_OK) return 0xFFFFFFFF;    /* (assuming bitmap is located top of the cluster heap) */
        i = val / 8 % SS(fs); bm = 1 << (val % 8);
        do {
            do {
                bv = fs->win[i] & bm; bm <<= 1;        /* Get bit value */
                if (++val >= fs->n_fatent - 2) {    /* Next cluster (with wrap-around) */
                    val = 0; bm = 0; i = SS(fs);
                }
                if (!bv) {    /* Is it a free cluster? */
                    if (++ctr == ncl) return scl + 2;    /* Check if run length is sufficient for required */
                } else {
                    scl = val; ctr = 0;        /* Encountered a cluster in-use, restart to scan */
                }
                if (val == clst) return 0;    /* All cluster scanned? */
            } while (bm);
            bm = 1;
        } while (++i < SS(fs));
    }
}


/*----------------------------------------*/
/* Set/Clear a block of allocation bitmap */
/*----------------------------------------*/

FRESULT change_bitmap (
    FATFS* fs,    /* Filesystem object */
    DWORD clst,    /* Cluster number to change from */
    DWORD ncl,    /* Number of clusters to be changed */
    int bv        /* bit value to be set (0 or 1) */
)
{
    BYTE bm;
    UINT i;
    DWORD sect;


    clst -= 2;    /* The first bit corresponds to cluster #2 */
    sect = fs->database + clst / 8 / SS(fs);    /* Sector address (assuming bitmap is located top of the cluster heap) */
    i = clst / 8 % SS(fs);                        /* Byte offset in the sector */
    bm = 1 << (clst % 8);                        /* Bit mask in the byte */
    for (;;) {
        if (move_window(fs, sect++) != FR_OK) return FR_DISK_ERR;
        do {
            do {
                if (bv == (int)((fs->win[i] & bm) != 0)) return FR_INT_ERR;    /* Is the bit expected value? */
                fs->win[i] ^= bm;    /* Flip the bit */
                fs->wflag = 1;
                if (--ncl == 0) return FR_OK;    /* All bits processed? */
            } while (bm <<= 1);        /* Next bit */
            bm = 1;
        } while (++i < SS(fs));        /* Next byte */
        i = 0;
    }
}


/*---------------------------------------------*/
/* Fill the first fragment of the FAT chain    */
/*---------------------------------------------*/

FRESULT fill_first_frag (
    FFOBJID* obj    /* Pointer to the corresponding object */
)
{
    FRESULT res;
    DWORD cl, n;


    if (obj->stat == 3) {    /* Has the object been changed 'fragmented' in this session? */
        for (cl = obj->sclust, n = obj->n_cont; n; cl++, n--) {    /* Create cluster chain on the FAT */
            res = put_fat(obj->fs, cl, cl + 1);
            if (res != FR_OK) return res;
        }
        obj->stat = 0;    /* Change status 'FAT chain is valid' */
    }
    return FR_OK;
}


/*---------------------------------------------*/
/* Fill the last fragment of the FAT chain     */
/*---------------------------------------------*/

FRESULT fill_last_frag (
    FFOBJID* obj,    /* Pointer to the corresponding object */
    DWORD lcl,        /* Last cluster of the fragment */
    DWORD term        /* Value to set the last FAT entry */
)
{
    FRESULT res;


    while (obj->n_frag > 0) {    /* Create the last chain on the FAT */
        res = put_fat(obj->fs, lcl - obj->n_frag + 1, (obj->n_frag > 1) ? lcl - obj->n_frag + 2 : term);
        if (res != FR_OK) return res;
        obj->n_frag--;
    }
    return FR_OK;
}

#endif    /* FF_FS_EXFAT && !FF_FS_READONLY */



#if !FF_FS_READONLY
/*-----------------------------------------------------------------------*/
/* FAT handling - Remove a cluster chain                                 */
/*-----------------------------------------------------------------------*/
FRESULT remove_chain (    /* FR_OK(0):succeeded, !=0:error */
    FFOBJID* obj,        /* Corresponding object */
    DWORD clst,            /* Cluster to remove a chain from */
    DWORD pclst            /* Previous cluster of clst (0:entire chain) */
)
{
    FRESULT res = FR_OK;
    DWORD nxt;
    FATFS *fs = obj->fs;
#if FF_FS_EXFAT || FF_USE_TRIM
    DWORD scl = clst, ecl = clst;
#endif
#if FF_USE_TRIM
    DWORD rt[2];
#endif

    if (clst < 2 || clst >= fs->n_fatent) return FR_INT_ERR;    /* Check if in valid range */

    /* Mark the previous cluster 'EOC' on the FAT if it exists */
    if (pclst != 0 && (!FF_FS_EXFAT || fs->fs_type != FS_EXFAT || obj->stat != 2)) {
        res = put_fat(fs, pclst, 0xFFFFFFFF);
        if (res != FR_OK) return res;
    }

    /* Remove the chain */
    do {
        nxt = get_fat(obj, clst);            /* Get cluster status */
        if (nxt == 0) break;                /* Empty cluster? */
        if (nxt == 1) return FR_INT_ERR;    /* Internal error? */
        if (nxt == 0xFFFFFFFF) return FR_DISK_ERR;    /* Disk error? */
        if (!FF_FS_EXFAT || fs->fs_type != FS_EXFAT) {
            res = put_fat(fs, clst, 0);        /* Mark the cluster 'free' on the FAT */
            if (res != FR_OK) return res;
        }
        if (fs->free_clst < fs->n_fatent - 2) {    /* Update FSINFO */
            fs->free_clst++;
            fs->fsi_flag |= 1;
        }
#if FF_FS_EXFAT || FF_USE_TRIM
        if (ecl + 1 == nxt) {    /* Is next cluster contiguous? */
            ecl = nxt;
        } else {                /* End of contiguous cluster block */
#if FF_FS_EXFAT
            if (fs->fs_type == FS_EXFAT) {
                res = change_bitmap(fs, scl, ecl - scl + 1, 0);    /* Mark the cluster block 'free' on the bitmap */
                if (res != FR_OK) return res;
            }
#endif
#if FF_USE_TRIM
            rt[0] = clst2sect(fs, scl);                    /* Start of data area freed */
            rt[1] = clst2sect(fs, ecl) + fs->csize - 1;    /* End of data area freed */
            disk_ioctl(fs->pdrv, CTRL_TRIM, rt);        /* Inform device the data in the block is no longer needed */
#endif
            scl = ecl = nxt;
        }
#endif
        clst = nxt;                    /* Next cluster */
    } while (clst < fs->n_fatent);    /* Repeat while not the last link */

#if FF_FS_EXFAT
    /* Some post processes for chain status */
    if (fs->fs_type == FS_EXFAT) {
        if (pclst == 0) {    /* Has the entire chain been removed? */
            obj->stat = 0;        /* Change the chain status 'initial' */
        } else {
            if (obj->stat == 0) {    /* Is it a fragmented chain from the beginning of this session? */
                clst = obj->sclust;        /* Follow the chain to check if it gets contiguous */
                while (clst != pclst) {
                    nxt = get_fat(obj, clst);
                    if (nxt < 2) return FR_INT_ERR;
                    if (nxt == 0xFFFFFFFF) return FR_DISK_ERR;
                    if (nxt != clst + 1) break;    /* Not contiguous? */
                    clst++;
                }
                if (clst == pclst) {    /* Has the chain got contiguous again? */
                    obj->stat = 2;        /* Change the chain status 'contiguous' */
                }
            } else {
                if (obj->stat == 3 && pclst >= obj->sclust && pclst <= obj->sclust + obj->n_cont) {    /* Was the chain fragmented in this session and got contiguous again? */
                    obj->stat = 2;    /* Change the chain status 'contiguous' */
                }
            }
        }
    }
#endif
    return FR_OK;
}




/*-----------------------------------------------------------------------*/
/* FAT handling - Stretch a chain or Create a new chain                  */
/*-----------------------------------------------------------------------*/
DWORD create_chain (    /* 0:No free cluster, 1:Internal error, 0xFFFFFFFF:Disk error, >=2:New cluster# */
    FFOBJID* obj,        /* Corresponding object */
    DWORD clst            /* Cluster# to stretch, 0:Create a new chain */
)
{
    DWORD cs, ncl, scl;
    FRESULT res;
    FATFS *fs = obj->fs;


    if (clst == 0) {    /* Create a new chain */
        scl = fs->last_clst;                /* Suggested cluster to start to find */
        if (scl == 0 || scl >= fs->n_fatent) scl = 1;
    }
    else {                /* Stretch a chain */
        cs = get_fat(obj, clst);            /* Check the cluster status */
        if (cs < 2) return 1;                /* Test for insanity */
        if (cs == 0xFFFFFFFF) return cs;    /* Test for disk error */
        if (cs < fs->n_fatent) return cs;    /* It is already followed by next cluster */
        scl = clst;                            /* Cluster to start to find */
    }
    if (fs->free_clst == 0) return 0;        /* No free cluster */

#if FF_FS_EXFAT
    if (fs->fs_type == FS_EXFAT) {    /* On the exFAT volume */
        ncl = find_bitmap(fs, scl, 1);                /* Find a free cluster */
        if (ncl == 0 || ncl == 0xFFFFFFFF) return ncl;    /* No free cluster or hard error? */
        res = change_bitmap(fs, ncl, 1, 1);            /* Mark the cluster 'in use' */
        if (res == FR_INT_ERR) return 1;
        if (res == FR_DISK_ERR) return 0xFFFFFFFF;
        if (clst == 0) {                            /* Is it a new chain? */
            obj->stat = 2;                            /* Set status 'contiguous' */
        } else {                                    /* It is a stretched chain */
            if (obj->stat == 2 && ncl != scl + 1) {    /* Is the chain got fragmented? */
                obj->n_cont = scl - obj->sclust;    /* Set size of the contiguous part */
                obj->stat = 3;                        /* Change status 'just fragmented' */
            }
        }
        if (obj->stat != 2) {    /* Is the file non-contiguous? */
            if (ncl == clst + 1) {    /* Is the cluster next to previous one? */
                obj->n_frag = obj->n_frag ? obj->n_frag + 1 : 2;    /* Increment size of last framgent */
            } else {                /* New fragment */
                if (obj->n_frag == 0) obj->n_frag = 1;
                res = fill_last_frag(obj, clst, ncl);    /* Fill last fragment on the FAT and link it to new one */
                if (res == FR_OK) obj->n_frag = 1;
            }
        }
    } else
#endif
    {    /* On the FAT/FAT32 volume */
        ncl = 0;
        if (scl == clst) {                        /* Stretching an existing chain? */
            ncl = scl + 1;                        /* Test if next cluster is free */
            if (ncl >= fs->n_fatent) ncl = 2;
            cs = get_fat(obj, ncl);                /* Get next cluster status */
            if (cs == 1 || cs == 0xFFFFFFFF) return cs;    /* Test for error */
            if (cs != 0) {                        /* Not free? */
                cs = fs->last_clst;                /* Start at suggested cluster if it is valid */
                if (cs >= 2 && cs < fs->n_fatent) scl = cs;
                ncl = 0;
            }
        }
        if (ncl == 0) {    /* The new cluster cannot be contiguous and find another fragment */
            ncl = scl;    /* Start cluster */
            for (;;) {
                ncl++;                            /* Next cluster */
                if (ncl >= fs->n_fatent) {        /* Check wrap-around */
                    ncl = 2;
                    if (ncl > scl) return 0;    /* No free cluster found? */
                }
                cs = get_fat(obj, ncl);            /* Get the cluster status */
                if (cs == 0) break;                /* Found a free cluster? */
                if (cs == 1 || cs == 0xFFFFFFFF) return cs;    /* Test for error */
                if (ncl == scl) return 0;        /* No free cluster found? */
            }
        }
        res = put_fat(fs, ncl, 0xFFFFFFFF);        /* Mark the new cluster 'EOC' */
        if (res == FR_OK && clst != 0) {
            res = put_fat(fs, clst, ncl);        /* Link it from the previous one if needed */
        }
    }

    if (res == FR_OK) {            /* Update FSINFO if function succeeded. */
        fs->last_clst = ncl;
        if (fs->free_clst <= fs->n_fatent - 2) fs->free_clst--;
        fs->fsi_flag |= 1;
    } else {
        ncl = (res == FR_DISK_ERR) ? 0xFFFFFFFF : 1;    /* Failed. Generate error status */
    }

    return ncl;        /* Return new cluster number or error status */
}

#endif /* !FF_FS_READONLY */




#if FF_USE_FASTSEEK
/*-----------------------------------------------------------------------*/
/* FAT handling - Convert offset into cluster with link map table        */
/*-----------------------------------------------------------------------*/

DWORD clmt_clust (    /* <2:Error, >=2:Cluster number */
    FIL* fp,        /* Pointer to the file object */
    FSIZE_t ofs        /* File offset to be converted to cluster# */
)
{
    DWORD cl, ncl, *tbl;
    FATFS *fs = fp->obj.fs;


    tbl = fp->cltbl + 1;    /* Top of CLMT */
    cl = (DWORD)(ofs / SS(fs) / fs->csize);    /* Cluster order from top of the file */
    for (;;) {
        ncl = *tbl++;            /* Number of cluters in the fragment */
        if (ncl == 0) return 0;    /* End of table? (error) */
        if (cl < ncl) break;    /* In this fragment? */
        cl -= ncl; tbl++;        /* Next fragment */
    }
    return cl + *tbl;    /* Return the cluster number */
}

#endif    /* FF_USE_FASTSEEK */




/*-----------------------------------------------------------------------*/
/* Directory handling - Fill a cluster with zeros                        */
/*-----------------------------------------------------------------------*/

#if !FF_FS_READONLY
FRESULT dir_clear (    /* Returns FR_OK or FR_DISK_ERR */
    FATFS *fs,        /* Filesystem object */
    DWORD clst        /* Directory table to clear */
)
{
    DWORD sect;
    UINT n, szb;
    BYTE *ibuf;


    if (sync_window(fs) != FR_OK) return FR_DISK_ERR;    /* Flush disk access window */
    sect = clst2sect(fs, clst);        /* Top of the cluster */
    fs->winsect = sect;                /* Set window to top of the cluster */
    MEMSET(fs->win, 0, SS(fs));    /* Clear window buffer */
#if FF_USE_LFN == 3        /* Quick table clear by using multi-secter write */
    /* Allocate a temporary buffer (32 KB max) */
    for (szb = ((DWORD)fs->csize * SS(fs) >= 0x8000) ? 0x8000 : fs->csize * SS(fs); szb > SS(fs) && !(ibuf = ff_memalloc(szb)); szb /= 2) ;
    if (szb > SS(fs)) {        /* Buffer allocated? */
        MEMSET(ibuf, 0, szb);
        szb /= SS(fs);        /* Bytes -> Sectors */
        for (n = 0; n < fs->csize && disk_write(fs->pdrv, ibuf, sect + n, szb) == RES_OK; n += szb) ;    /* Fill the cluster with 0 */
        ff_memfree(ibuf);
    } else
#endif
    {
        ibuf = fs->win; szb = 1;    /* Use window buffer (single-sector writes may take a time) */
        for (n = 0; n < fs->csize && disk_write(fs->pdrv, ibuf, sect + n, szb) == RES_OK; n += szb) ;    /* Fill the cluster with 0 */
    }
    return (n == fs->csize) ? FR_OK : FR_DISK_ERR;
}
#endif    /* !FF_FS_READONLY */




/*-----------------------------------------------------------------------*/
/* Directory handling - Set directory index                              */
/*-----------------------------------------------------------------------*/

FRESULT dir_sdi (    /* FR_OK(0):succeeded, !=0:error */
    DIR* dp,        /* Pointer to directory object */
    DWORD ofs        /* Offset of directory table */
)
{
    DWORD csz, clst;
    FATFS *fs = dp->obj.fs;


    if (ofs >= (DWORD)((FF_FS_EXFAT && fs->fs_type == FS_EXFAT) ? MAX_DIR_EX : MAX_DIR) || ofs % SZDIRE) {    /* Check range of offset and alignment */
        return FR_INT_ERR;
    }
    dp->dptr = ofs;                /* Set current offset */
    clst = dp->obj.sclust;        /* Table start cluster (0:root) */
    if (clst == 0 && fs->fs_type >= FS_FAT32) {    /* Replace cluster# 0 with root cluster# */
        clst = fs->dirbase;
        if (FF_FS_EXFAT) dp->obj.stat = 0;    /* exFAT: Root dir has an FAT chain */
    }

    if (clst == 0) {    /* Static table (root-directory on the FAT volume) */
        if (ofs / SZDIRE >= fs->n_rootdir) return FR_INT_ERR;    /* Is index out of range? */
        dp->sect = fs->dirbase;

    } else {            /* Dynamic table (sub-directory or root-directory on the FAT32/exFAT volume) */
        csz = (DWORD)fs->csize * SS(fs);    /* Bytes per cluster */
        while (ofs >= csz) {                /* Follow cluster chain */
            clst = get_fat(&dp->obj, clst);                /* Get next cluster */
            if (clst == 0xFFFFFFFF) return FR_DISK_ERR;    /* Disk error */
            if (clst < 2 || clst >= fs->n_fatent) return FR_INT_ERR;    /* Reached to end of table or internal error */
            ofs -= csz;
        }
        dp->sect = clst2sect(fs, clst);
    }
    dp->clust = clst;                    /* Current cluster# */
    if (dp->sect == 0) return FR_INT_ERR;
    dp->sect += ofs / SS(fs);            /* Sector# of the directory entry */
    dp->dir = fs->win + (ofs % SS(fs));    /* Pointer to the entry in the win[] */

    return FR_OK;
}




/*-----------------------------------------------------------------------*/
/* Directory handling - Move directory table index next                  */
/*-----------------------------------------------------------------------*/

FRESULT dir_next (    /* FR_OK(0):succeeded, FR_NO_FILE:End of table, FR_DENIED:Could not stretch */
    DIR* dp,        /* Pointer to the directory object */
    int stretch        /* 0: Do not stretch table, 1: Stretch table if needed */
)
{
    DWORD ofs, clst;
    FATFS *fs = dp->obj.fs;


    ofs = dp->dptr + SZDIRE;    /* Next entry */
    if (dp->sect == 0 || ofs >= (DWORD)((FF_FS_EXFAT && fs->fs_type == FS_EXFAT) ? MAX_DIR_EX : MAX_DIR)) return FR_NO_FILE;    /* Report EOT when offset has reached max value */

    if (ofs % SS(fs) == 0) {    /* Sector changed? */
        dp->sect++;                /* Next sector */

        if (dp->clust == 0) {    /* Static table */
            if (ofs / SZDIRE >= fs->n_rootdir) {    /* Report EOT if it reached end of static table */
                dp->sect = 0; return FR_NO_FILE;
            }
        }
        else {                    /* Dynamic table */
            if ((ofs / SS(fs) & (fs->csize - 1)) == 0) {    /* Cluster changed? */
                clst = get_fat(&dp->obj, dp->clust);        /* Get next cluster */
                if (clst <= 1) return FR_INT_ERR;            /* Internal error */
                if (clst == 0xFFFFFFFF) return FR_DISK_ERR;    /* Disk error */
                if (clst >= fs->n_fatent) {                    /* It reached end of dynamic table */
#if !FF_FS_READONLY
                    if (!stretch) {                                /* If no stretch, report EOT */
                        dp->sect = 0; return FR_NO_FILE;
                    }
                    clst = create_chain(&dp->obj, dp->clust);    /* Allocate a cluster */
                    if (clst == 0) return FR_DENIED;            /* No free cluster */
                    if (clst == 1) return FR_INT_ERR;            /* Internal error */
                    if (clst == 0xFFFFFFFF) return FR_DISK_ERR;    /* Disk error */
                    if (dir_clear(fs, clst) != FR_OK) return FR_DISK_ERR;    /* Clean up the stretched table */
                    if (FF_FS_EXFAT) dp->obj.stat |= 4;            /* exFAT: The directory has been stretched */
#else
                    if (!stretch) dp->sect = 0;                    /* (this line is to suppress compiler warning) */
                    dp->sect = 0; return FR_NO_FILE;            /* Report EOT */
#endif
                }
                dp->clust = clst;        /* Initialize data for new cluster */
                dp->sect = clst2sect(fs, clst);
            }
        }
    }
    dp->dptr = ofs;                        /* Current entry */
    dp->dir = fs->win + ofs % SS(fs);    /* Pointer to the entry in the win[] */

    return FR_OK;
}




#if !FF_FS_READONLY
/*-----------------------------------------------------------------------*/
/* Directory handling - Reserve a block of directory entries             */
/*-----------------------------------------------------------------------*/

FRESULT dir_alloc (    /* FR_OK(0):succeeded, !=0:error */
    DIR* dp,        /* Pointer to the directory object */
    UINT nent        /* Number of contiguous entries to allocate */
)
{
    FRESULT res;
    UINT n;
    FATFS *fs = dp->obj.fs;


    res = dir_sdi(dp, 0);
    if (res == FR_OK) {
        n = 0;
        do {
            res = move_window(fs, dp->sect);
            if (res != FR_OK) break;
#if FF_FS_EXFAT
            if ((fs->fs_type == FS_EXFAT) ? (int)((dp->dir[XDIR_Type] & 0x80) == 0) : (int)(dp->dir[DIR_Name] == DDEM || dp->dir[DIR_Name] == 0)) {
#else
            if (dp->dir[DIR_Name] == DDEM || dp->dir[DIR_Name] == 0) {
#endif
                if (++n == nent) break;    /* A block of contiguous free entries is found */
            } else {
                n = 0;                    /* Not a blank entry. Restart to search */
            }
            res = dir_next(dp, 1);
        } while (res == FR_OK);    /* Next entry with table stretch enabled */
    }

    if (res == FR_NO_FILE) res = FR_DENIED;    /* No directory entry to allocate */
    return res;
}

#endif    /* !FF_FS_READONLY */




/*-----------------------------------------------------------------------*/
/* FAT: Directory handling - Load/Store start cluster number             */
/*-----------------------------------------------------------------------*/

DWORD ld_clust (    /* Returns the top cluster value of the SFN entry */
    FATFS* fs,        /* Pointer to the fs object */
    const BYTE* dir    /* Pointer to the key entry */
)
{
    DWORD cl;

    cl = ld_word(dir + DIR_FstClusLO);
    if (fs->fs_type == FS_FAT32) {
        cl |= (DWORD)ld_word(dir + DIR_FstClusHI) << 16;
    }

    return cl;
}


#if !FF_FS_READONLY
void st_clust (
    FATFS* fs,    /* Pointer to the fs object */
    BYTE* dir,    /* Pointer to the key entry */
    DWORD cl    /* Value to be set */
)
{
    st_word(dir + DIR_FstClusLO, (WORD)cl);
    if (fs->fs_type == FS_FAT32) {
        st_word(dir + DIR_FstClusHI, (WORD)(cl >> 16));
    }
}
#endif



#if FF_USE_LFN
/*--------------------------------------------------------*/
/* FAT-LFN: Compare a part of file name with an LFN entry */
/*--------------------------------------------------------*/
int cmp_lfn (                /* 1:matched, 0:not matched */
    const WCHAR* lfnbuf,    /* Pointer to the LFN working buffer to be compared */
    BYTE* dir                /* Pointer to the directory entry containing the part of LFN */
)
{
    UINT i, s;
    WCHAR wc, uc;


    if (ld_word(dir + LDIR_FstClusLO) != 0) return 0;    /* Check LDIR_FstClusLO */

    i = ((dir[LDIR_Ord] & 0x3F) - 1) * 13;    /* Offset in the LFN buffer */

    for (wc = 1, s = 0; s < 13; s++) {        /* Process all characters in the entry */
        uc = ld_word(dir + LfnOfs[s]);        /* Pick an LFN character */
        if (wc) {
            if (i >= FF_MAX_LFN || ff_wtoupper(uc) != ff_wtoupper(lfnbuf[i++])) {    /* Compare it */
                return 0;                    /* Not matched */
            }
            wc = uc;
        } else {
            if (uc != 0xFFFF) return 0;        /* Check filler */
        }
    }

    if ((dir[LDIR_Ord] & LLEF) && wc && lfnbuf[i]) return 0;    /* Last segment matched but different length */

    return 1;        /* The part of LFN matched */
}


#if FF_FS_MINIMIZE <= 1 || FF_FS_RPATH >= 2 || FF_USE_LABEL || FF_FS_EXFAT
/*-----------------------------------------------------*/
/* FAT-LFN: Pick a part of file name from an LFN entry */
/*-----------------------------------------------------*/
int pick_lfn (            /* 1:succeeded, 0:buffer overflow or invalid LFN entry */
    WCHAR* lfnbuf,        /* Pointer to the LFN working buffer */
    BYTE* dir            /* Pointer to the LFN entry */
)
{
    UINT i, s;
    WCHAR wc, uc;


    if (ld_word(dir + LDIR_FstClusLO) != 0) return 0;    /* Check LDIR_FstClusLO is 0 */

    i = ((dir[LDIR_Ord] & ~LLEF) - 1) * 13;    /* Offset in the LFN buffer */

    for (wc = 1, s = 0; s < 13; s++) {        /* Process all characters in the entry */
        uc = ld_word(dir + LfnOfs[s]);        /* Pick an LFN character */
        if (wc) {
            if (i >= FF_MAX_LFN) return 0;    /* Buffer overflow? */
            lfnbuf[i++] = wc = uc;            /* Store it */
        } else {
            if (uc != 0xFFFF) return 0;        /* Check filler */
        }
    }

    if (dir[LDIR_Ord] & LLEF) {                /* Put terminator if it is the last LFN part */
        if (i >= FF_MAX_LFN) return 0;        /* Buffer overflow? */
        lfnbuf[i] = 0;
    }

    return 1;        /* The part of LFN is valid */
}
#endif


#if !FF_FS_READONLY
/*-----------------------------------------*/
/* FAT-LFN: Create an entry of LFN entries */
/*-----------------------------------------*/
void put_lfn (
    const WCHAR* lfn,    /* Pointer to the LFN */
    BYTE* dir,            /* Pointer to the LFN entry to be created */
    BYTE ord,            /* LFN order (1-20) */
    BYTE sum            /* Checksum of the corresponding SFN */
)
{
    UINT i, s;
    WCHAR wc;


    dir[LDIR_Chksum] = sum;            /* Set checksum */
    dir[LDIR_Attr] = AM_LFN;        /* Set attribute. LFN entry */
    dir[LDIR_Type] = 0;
    st_word(dir + LDIR_FstClusLO, 0);

    i = (ord - 1) * 13;                /* Get offset in the LFN working buffer */
    s = wc = 0;
    do {
        if (wc != 0xFFFF) wc = lfn[i++];    /* Get an effective character */
        st_word(dir + LfnOfs[s], wc);        /* Put it */
        if (wc == 0) wc = 0xFFFF;        /* Padding characters for left locations */
    } while (++s < 13);
    if (wc == 0xFFFF || !lfn[i]) ord |= LLEF;    /* Last LFN part is the start of LFN sequence */
    dir[LDIR_Ord] = ord;            /* Set the LFN order */
}

#endif    /* !FF_FS_READONLY */
#endif    /* FF_USE_LFN */



#if FF_USE_LFN && !FF_FS_READONLY
/*-----------------------------------------------------------------------*/
/* FAT-LFN: Create a Numbered SFN                                        */
/*-----------------------------------------------------------------------*/

void gen_numname (
    BYTE* dst,            /* Pointer to the buffer to store numbered SFN */
    const BYTE* src,    /* Pointer to SFN */
    const WCHAR* lfn,    /* Pointer to LFN */
    UINT seq            /* Sequence number */
)
{
    BYTE ns[8], c;
    UINT i, j;
    WCHAR wc;
    DWORD sr;


    MEMCPY  (dst, src, 11);

    if (seq > 5) {    /* In case of many collisions, generate a hash number instead of sequential number */
        sr = seq;
        while (*lfn) {    /* Create a CRC */
            wc = *lfn++;
            for (i = 0; i < 16; i++) {
                sr = (sr << 1) + (wc & 1);
                wc >>= 1;
                if (sr & 0x10000) sr ^= 0x11021;
            }
        }
        seq = (UINT)sr;
    }

    /* itoa (hexdecimal) */
    i = 7;
    do {
        c = (BYTE)((seq % 16) + '0');
        if (c > '9') c += 7;
        ns[i--] = c;
        seq /= 16;
    } while (seq);
    ns[i] = '~';

    /* Append the number to the SFN body */
    for (j = 0; j < i && dst[j] != ' '; j++) {
        if (dbc_1st(dst[j])) {
            if (j == i - 1) break;
            j++;
        }
    }
    do {
        dst[j++] = (i < 8) ? ns[i++] : ' ';
    } while (j < 8);
}
#endif    /* FF_USE_LFN && !FF_FS_READONLY */



#if FF_USE_LFN
/*-----------------------------------------------------------------------*/
/* FAT-LFN: Calculate checksum of an SFN entry                           */
/*-----------------------------------------------------------------------*/

BYTE sum_sfn (
    const BYTE* dir        /* Pointer to the SFN entry */
)
{
    BYTE sum = 0;
    UINT n = 11;

    do {
        sum = (sum >> 1) + (sum << 7) + *dir++;
    } while (--n);
    return sum;
}

#endif    /* FF_USE_LFN */



#if FF_FS_EXFAT
/*-----------------------------------------------------------------------*/
/* exFAT: Checksum                                                       */
/*-----------------------------------------------------------------------*/

WORD xdir_sum (            /* Get checksum of the directoly entry block */
    const BYTE* dir        /* Directory entry block to be calculated */
)
{
    UINT i, szblk;
    WORD sum;


    szblk = (dir[XDIR_NumSec] + 1) * SZDIRE;
    for (i = sum = 0; i < szblk; i++) {
        if (i == XDIR_SetSum) {    /* Skip sum field */
            i++;
        } else {
            sum = ((sum & 1) ? 0x8000 : 0) + (sum >> 1) + dir[i];
        }
    }
    return sum;
}



WORD xname_sum (        /* Get check sum (to be used as hash) of the name */
    const WCHAR* name    /* File name to be calculated */
)
{
    WCHAR chr;
    WORD sum = 0;


    while ((chr = *name++) != 0) {
        chr = ff_wtoupper(chr);        /* File name needs to be upper-case converted */
        sum = ((sum & 1) ? 0x8000 : 0) + (sum >> 1) + (chr & 0xFF);
        sum = ((sum & 1) ? 0x8000 : 0) + (sum >> 1) + (chr >> 8);
    }
    return sum;
}


#if !FF_FS_READONLY && FF_USE_MKFS
DWORD xsum32 (
    BYTE  dat,    /* Byte to be calculated */
    DWORD sum    /* Previous sum */
)
{
    sum = ((sum & 1) ? 0x80000000 : 0) + (sum >> 1) + dat;
    return sum;
}
#endif


#if FF_FS_MINIMIZE <= 1 || FF_FS_RPATH >= 2
/*------------------------------------------------------*/
/* exFAT: Get object information from a directory block */
/*------------------------------------------------------*/

void get_xdir_info (
    BYTE* dirb,            /* Pointer to the direcotry entry block 85+C0+C1s */
    FILINFO* fno        /* Buffer to store the extracted file information */
)
{
    WCHAR w;
    UINT di, si, nc;

    /* Get file name */
    for (si = SZDIRE * 2, nc = di = 0; nc < dirb[XDIR_NumName]; si += 2, nc++) {
        if ((si % SZDIRE) == 0) si += 2;        /* Skip entry type field */
        w = ld_word(dirb + si);                    /* Get a character */
#if !FF_LFN_UNICODE        /* ANSI/OEM API */
        w = ff_uni2oem(w, CODEPAGE);            /* Convert it to OEM code */
        if (w >= 0x100) {                        /* Is it a double byte char? */
            fno->fname[di++] = (char)(w >> 8);    /* Store 1st byte of the DBC */
        }
#endif
        if (w == 0 || di >= FF_MAX_LFN) { di = 0; break; }    /* Invalid char or buffer overflow --> inaccessible object name */
        fno->fname[di++] = (TCHAR)w;            /* Store the character */
    }
    if (di == 0) fno->fname[di++] = '?';    /* Inaccessible object name? */
    fno->fname[di] = 0;                        /* Terminate file name */

    fno->altname[0] = 0;                    /* No SFN */
    fno->fattrib = dirb[XDIR_Attr];            /* Attribute */
    fno->fsize = (fno->fattrib & AM_DIR) ? 0 : ld_qword(dirb + XDIR_FileSize);    /* Size */
    fno->ftime = ld_word(dirb + XDIR_ModTime + 0);    /* Time */
    fno->fdate = ld_word(dirb + XDIR_ModTime + 2);    /* Date */
}

#endif    /* FF_FS_MINIMIZE <= 1 || FF_FS_RPATH >= 2 */


/*-----------------------------------*/
/* exFAT: Get a directry entry block */
/*-----------------------------------*/

FRESULT load_xdir (    /* FR_INT_ERR: invalid entry block */
    DIR* dp            /* Reading direcotry object pointing top of the entry block to load */
)
{
    FRESULT res;
    UINT i, sz_ent;
    BYTE* dirb = dp->obj.fs->dirbuf;    /* Pointer to the on-memory direcotry entry block 85+C0+C1s */


    /* Load 85 entry */
    res = move_window(dp->obj.fs, dp->sect);
    if (res != FR_OK) return res;
    if (dp->dir[XDIR_Type] != 0x85) return FR_INT_ERR;    /* Invalid order */
    MEMCPY(dirb + 0 * SZDIRE, dp->dir, SZDIRE);
    sz_ent = (dirb[XDIR_NumSec] + 1) * SZDIRE;
    if (sz_ent < 3 * SZDIRE || sz_ent > 19 * SZDIRE) return FR_INT_ERR;

    /* Load C0 entry */
    res = dir_next(dp, 0);
    if (res == FR_NO_FILE) res = FR_INT_ERR;    /* It cannot be */
    if (res != FR_OK) return res;
    res = move_window(dp->obj.fs, dp->sect);
    if (res != FR_OK) return res;
    if (dp->dir[XDIR_Type] != 0xC0) return FR_INT_ERR;    /* Invalid order */
    MEMCPY(dirb + 1 * SZDIRE, dp->dir, SZDIRE);
    if (MAXDIRB(dirb[XDIR_NumName]) > sz_ent) return FR_INT_ERR;

    /* Load C1 entries */
    i = 2 * SZDIRE;    /* C1 offset to load */
    do {
        res = dir_next(dp, 0);
        if (res == FR_NO_FILE) res = FR_INT_ERR;    /* It cannot be */
        if (res != FR_OK) return res;
        res = move_window(dp->obj.fs, dp->sect);
        if (res != FR_OK) return res;
        if (dp->dir[XDIR_Type] != 0xC1) return FR_INT_ERR;    /* Invalid order */
        if (i < MAXDIRB(FF_MAX_LFN)) MEMCPY(dirb + i, dp->dir, SZDIRE);
    } while ((i += SZDIRE) < sz_ent);

    /* Sanity check (do it for only accessible object) */
    if (i <= MAXDIRB(FF_MAX_LFN)) {
        if (xdir_sum(dirb) != ld_word(dirb + XDIR_SetSum)) return FR_INT_ERR;
    }
    return FR_OK;
}


#if !FF_FS_READONLY || FF_FS_RPATH != 0
/*------------------------------------------------*/
/* exFAT: Load the object's directory entry block */
/*------------------------------------------------*/
FRESULT load_obj_xdir (    
    DIR* dp,            /* Blank directory object to be used to access containing direcotry */
    const FFOBJID* obj    /* Object with its containing directory information */
)
{
    FRESULT res;

    /* Open object containing directory */
    dp->obj.fs = obj->fs;
    dp->obj.sclust = obj->c_scl;
    dp->obj.stat = (BYTE)obj->c_size;
    dp->obj.objsize = obj->c_size & 0xFFFFFF00;
    dp->obj.n_frag = 0;
    dp->blk_ofs = obj->c_ofs;

    res = dir_sdi(dp, dp->blk_ofs);    /* Goto object's entry block */
    if (res == FR_OK) {
        res = load_xdir(dp);        /* Load the object's entry block */
    }
    return res;
}
#endif


#if !FF_FS_READONLY
/*----------------------------------------*/
/* exFAT: Store the directory entry block */
/*----------------------------------------*/
FRESULT store_xdir (
    DIR* dp                /* Pointer to the direcotry object */
)
{
    FRESULT res;
    UINT nent;
    BYTE* dirb = dp->obj.fs->dirbuf;    /* Pointer to the direcotry entry block 85+C0+C1s */

    /* Create set sum */
    st_word(dirb + XDIR_SetSum, xdir_sum(dirb));
    nent = dirb[XDIR_NumSec] + 1;

    /* Store the direcotry entry block to the directory */
    res = dir_sdi(dp, dp->blk_ofs);
    while (res == FR_OK) {
        res = move_window(dp->obj.fs, dp->sect);
        if (res != FR_OK) break;
        MEMCPY(dp->dir, dirb, SZDIRE);
        dp->obj.fs->wflag = 1;
        if (--nent == 0) break;
        dirb += SZDIRE;
        res = dir_next(dp, 0);
    }
    return (res == FR_OK || res == FR_DISK_ERR) ? res : FR_INT_ERR;
}



/*-------------------------------------------*/
/* exFAT: Create a new directory enrty block */
/*-------------------------------------------*/

void create_xdir (
    BYTE* dirb,            /* Pointer to the direcotry entry block buffer */
    const WCHAR* lfn    /* Pointer to the object name */
)
{
    UINT i;
    BYTE nc1, nlen;
    WCHAR chr;


    /* Create 85+C0 entry */
    MEMSET(dirb, 0, 2 * SZDIRE);
    dirb[0 * SZDIRE + XDIR_Type] = 0x85;
    dirb[1 * SZDIRE + XDIR_Type] = 0xC0;

    /* Create C1 entries */
    nlen = nc1 = 0; chr = 1; i = SZDIRE * 2;
    do {
        dirb[i++] = 0xC1; dirb[i++] = 0;    /* Entry type C1 */
        do {    /* Fill name field */
            if (chr && (chr = lfn[nlen]) != 0) nlen++;    /* Get a character if exist */
            st_word(dirb + i, chr);         /* Store it */
            i += 2;
        } while (i % SZDIRE != 0);
        nc1++;
    } while (lfn[nlen]);    /* Fill next entry if any char follows */

    dirb[XDIR_NumName] = nlen;        /* Set name length */
    dirb[XDIR_NumSec] = 1 + nc1;    /* Set secondary count */
    st_word(dirb + XDIR_NameHash, xname_sum(lfn));    /* Set name hash */
}

#endif    /* !FF_FS_READONLY */
#endif    /* FF_FS_EXFAT */



#if FF_FS_MINIMIZE <= 1 || FF_FS_RPATH >= 2 || FF_USE_LABEL || FF_FS_EXFAT
/*-----------------------------------------------------------------------*/
/* Read an object from the directory                                     */
/*-----------------------------------------------------------------------*/

FRESULT dir_read (
    DIR* dp,        /* Pointer to the directory object */
    int vol            /* Filtered by 0:file/directory or 1:volume label */
)
{
    FRESULT res = FR_NO_FILE;
    FATFS *fs = dp->obj.fs;
    BYTE a, c;
#if FF_USE_LFN
    BYTE ord = 0xFF, sum = 0xFF;
#endif

    while (dp->sect) {
        res = move_window(fs, dp->sect);
        if (res != FR_OK) break;
        c = dp->dir[DIR_Name];    /* Test for the entry type */
        if (c == 0) {
            res = FR_NO_FILE; break; /* Reached to end of the directory */
        }
#if FF_FS_EXFAT
        if (fs->fs_type == FS_EXFAT) {    /* On the exFAT volume */
            if (FF_USE_LABEL && vol) {
                if (c == 0x83) break;    /* Volume label entry? */
            } else {
                if (c == 0x85) {        /* Start of the file entry block? */
                    dp->blk_ofs = dp->dptr;    /* Get location of the block */
                    res = load_xdir(dp);    /* Load the entry block */
                    if (res == FR_OK) {
                        dp->obj.attr = fs->dirbuf[XDIR_Attr] & AM_MASK;    /* Get attribute */
                    }
                    break;
                }
            }
        } else
#endif
        {    /* On the FAT/FAT32 volume */
            dp->obj.attr = a = dp->dir[DIR_Attr] & AM_MASK;    /* Get attribute */
#if FF_USE_LFN        /* LFN configuration */
            if (c == DDEM || c == '.' || (int)((a & ~AM_ARC) == AM_VOL) != vol) {    /* An entry without valid data */
                ord = 0xFF;
            } else {
                if (a == AM_LFN) {            /* An LFN entry is found */
                    if (c & LLEF) {            /* Is it start of an LFN sequence? */
                        sum = dp->dir[LDIR_Chksum];
                        c &= (BYTE)~LLEF; ord = c;
                        dp->blk_ofs = dp->dptr;
                    }
                    /* Check LFN validity and capture it */
                    ord = (c == ord && sum == dp->dir[LDIR_Chksum] && pick_lfn(fs->lfnbuf, dp->dir)) ? ord - 1 : 0xFF;
                } else {                    /* An SFN entry is found */
                    if (ord || sum != sum_sfn(dp->dir)) {    /* Is there a valid LFN? */
                        dp->blk_ofs = 0xFFFFFFFF;            /* It has no LFN. */
                    }
                    break;
                }
            }
#else        /* Non LFN configuration */
            if (c != DDEM && c != '.' && a != AM_LFN && (int)((a & ~AM_ARC) == AM_VOL) == vol) {    /* Is it a valid entry? */
                break;
            }
#endif
        }
        res = dir_next(dp, 0);        /* Next entry */
        if (res != FR_OK) break;
    }

    if (res != FR_OK) dp->sect = 0;        /* Terminate the read operation on error or EOT */
    return res;
}

#endif    /* FF_FS_MINIMIZE <= 1 || FF_USE_LABEL || FF_FS_RPATH >= 2 */



/*-----------------------------------------------------------------------*/
/* Directory handling - Find an object in the directory                  */
/*-----------------------------------------------------------------------*/

FRESULT dir_find (    /* FR_OK(0):succeeded, !=0:error */
    DIR* dp            /* Pointer to the directory object with the file name */
)
{
    FRESULT res;
    FATFS *fs = dp->obj.fs;
    BYTE c;
#if FF_USE_LFN
    BYTE a, ord, sum;
#endif

    res = dir_sdi(dp, 0);            /* Rewind directory object */
    if (res != FR_OK) return res;
#if FF_FS_EXFAT
    if (fs->fs_type == FS_EXFAT) {    /* On the exFAT volume */
        BYTE nc;
        UINT di, ni;
        WORD hash = xname_sum(fs->lfnbuf);        /* Hash value of the name to find */

        while ((res = dir_read(dp, 0)) == FR_OK) {    /* Read an item */
#if FF_MAX_LFN < 255
            if (fs->dirbuf[XDIR_NumName] > FF_MAX_LFN) continue;            /* Skip comparison if inaccessible object name */
#endif
            if (ld_word(fs->dirbuf + XDIR_NameHash) != hash) continue;    /* Skip comparison if hash mismatched */
            for (nc = fs->dirbuf[XDIR_NumName], di = SZDIRE * 2, ni = 0; nc; nc--, di += 2, ni++) {    /* Compare the name */
                if ((di % SZDIRE) == 0) di += 2;
                if (ff_wtoupper(ld_word(fs->dirbuf + di)) != ff_wtoupper(fs->lfnbuf[ni])) break;
            }
            if (nc == 0 && !fs->lfnbuf[ni]) break;    /* Name matched? */
        }
        return res;
    }
#endif
    /* On the FAT/FAT32 volume */
#if FF_USE_LFN
    ord = sum = 0xFF; dp->blk_ofs = 0xFFFFFFFF;    /* Reset LFN sequence */
#endif
    do {
        res = move_window(fs, dp->sect);
        if (res != FR_OK) break;
        c = dp->dir[DIR_Name];
        if (c == 0) { res = FR_NO_FILE; break; }    /* Reached to end of table */
#if FF_USE_LFN        /* LFN configuration */
        dp->obj.attr = a = dp->dir[DIR_Attr] & AM_MASK;
        if (c == DDEM || ((a & AM_VOL) && a != AM_LFN)) {    /* An entry without valid data */
            ord = 0xFF; dp->blk_ofs = 0xFFFFFFFF;    /* Reset LFN sequence */
        } else {
            if (a == AM_LFN) {            /* An LFN entry is found */
                if (!(dp->fn[NSFLAG] & NS_NOLFN)) {
                    if (c & LLEF) {        /* Is it start of LFN sequence? */
                        sum = dp->dir[LDIR_Chksum];
                        c &= (BYTE)~LLEF; ord = c;    /* LFN start order */
                        dp->blk_ofs = dp->dptr;    /* Start offset of LFN */
                    }
                    /* Check validity of the LFN entry and compare it with given name */
                    ord = (c == ord && sum == dp->dir[LDIR_Chksum] && cmp_lfn(fs->lfnbuf, dp->dir)) ? ord - 1 : 0xFF;
                }
            } else {                    /* An SFN entry is found */
                if (!ord && sum == sum_sfn(dp->dir)) break;    /* LFN matched? */
                if (!(dp->fn[NSFLAG] & NS_LOSS) && !MEMCMP(dp->dir, dp->fn, 11)) break;    /* SFN matched? */
                ord = 0xFF; dp->blk_ofs = 0xFFFFFFFF;    /* Reset LFN sequence */
            }
        }
#else        /* Non LFN configuration */
        dp->obj.attr = dp->dir[DIR_Attr] & AM_MASK;
        if (!(dp->dir[DIR_Attr] & AM_VOL) && !MEMCMP(dp->dir, dp->fn, 11)) break;    /* Is it a valid entry? */
#endif
        res = dir_next(dp, 0);    /* Next entry */
    } while (res == FR_OK);

    return res;
}




#if !FF_FS_READONLY
/*-----------------------------------------------------------------------*/
/* Register an object to the directory                                   */
/*-----------------------------------------------------------------------*/

FRESULT dir_register (    /* FR_OK:succeeded, FR_DENIED:no free entry or too many SFN collision, FR_DISK_ERR:disk error */
    DIR* dp                /* Target directory with object name to be created */
)
{
    FRESULT res;
    FATFS *fs = dp->obj.fs;
#if FF_USE_LFN        /* LFN configuration */
    UINT n, nlen, nent;
    BYTE sn[12], sum;


    if (dp->fn[NSFLAG] & (NS_DOT | NS_NONAME)) return FR_INVALID_NAME;    /* Check name validity */
    for (nlen = 0; fs->lfnbuf[nlen]; nlen++) ;    /* Get lfn length */

#if FF_FS_EXFAT
    if (fs->fs_type == FS_EXFAT) {    /* On the exFAT volume */
        nent = (nlen + 14) / 15 + 2;    /* Number of entries to allocate (85+C0+C1s) */
        res = dir_alloc(dp, nent);        /* Allocate entries */
        if (res != FR_OK) return res;
        dp->blk_ofs = dp->dptr - SZDIRE * (nent - 1);    /* Set the allocated entry block offset */

        if (dp->obj.stat & 4) {            /* Has the directory been stretched? */
            dp->obj.stat &= ~4;
            res = fill_first_frag(&dp->obj);    /* Fill the first fragment on the FAT if needed */
            if (res != FR_OK) return res;
            res = fill_last_frag(&dp->obj, dp->clust, 0xFFFFFFFF);    /* Fill the last fragment on the FAT if needed */
            if (res != FR_OK) return res;
            if (dp->obj.sclust != 0) {        /* Is it a sub directory? */
                DIR dj;

                res = load_obj_xdir(&dj, &dp->obj);    /* Load the object status */
                if (res != FR_OK) return res;
                dp->obj.objsize += (DWORD)fs->csize * SS(fs);            /* Increase the directory size by cluster size */
                st_qword(fs->dirbuf + XDIR_FileSize, dp->obj.objsize);    /* Update the allocation status */
                st_qword(fs->dirbuf + XDIR_ValidFileSize, dp->obj.objsize);
                fs->dirbuf[XDIR_GenFlags] = dp->obj.stat | 1;
                res = store_xdir(&dj);                /* Store the object status */
                if (res != FR_OK) return res;
            }
        }

        create_xdir(fs->dirbuf, fs->lfnbuf);    /* Create on-memory directory block to be written later */
        return FR_OK;
    }
#endif
    /* On the FAT/FAT32 volume */
    MEMCPY(sn, dp->fn, 12);
    if (sn[NSFLAG] & NS_LOSS) {            /* When LFN is out of 8.3 format, generate a numbered name */
        dp->fn[NSFLAG] = NS_NOLFN;        /* Find only SFN */
        for (n = 1; n < 100; n++) {
            gen_numname(dp->fn, sn, fs->lfnbuf, n);    /* Generate a numbered name */
            res = dir_find(dp);                /* Check if the name collides with existing SFN */
            if (res != FR_OK) break;
        }
        if (n == 100) return FR_DENIED;        /* Abort if too many collisions */
        if (res != FR_NO_FILE) return res;    /* Abort if the result is other than 'not collided' */
        dp->fn[NSFLAG] = sn[NSFLAG];
    }

    /* Create an SFN with/without LFNs. */
    nent = (sn[NSFLAG] & NS_LFN) ? (nlen + 12) / 13 + 1 : 1;    /* Number of entries to allocate */
    res = dir_alloc(dp, nent);        /* Allocate entries */
    if (res == FR_OK && --nent) {    /* Set LFN entry if needed */
        res = dir_sdi(dp, dp->dptr - nent * SZDIRE);
        if (res == FR_OK) {
            sum = sum_sfn(dp->fn);    /* Checksum value of the SFN tied to the LFN */
            do {                    /* Store LFN entries in bottom first */
                res = move_window(fs, dp->sect);
                if (res != FR_OK) break;
                put_lfn(fs->lfnbuf, dp->dir, (BYTE)nent, sum);
                fs->wflag = 1;
                res = dir_next(dp, 0);    /* Next entry */
            } while (res == FR_OK && --nent);
        }
    }

#else    /* Non LFN configuration */
    res = dir_alloc(dp, 1);        /* Allocate an entry for SFN */

#endif

    /* Set SFN entry */
    if (res == FR_OK) {
        res = move_window(fs, dp->sect);
        if (res == FR_OK) {
            MEMSET(dp->dir, 0, SZDIRE);    /* Clean the entry */
            MEMCPY(dp->dir + DIR_Name, dp->fn, 11);    /* Put SFN */
#if FF_USE_LFN
            dp->dir[DIR_NTres] = dp->fn[NSFLAG] & (NS_BODY | NS_EXT);    /* Put NT flag */
#endif
            fs->wflag = 1;
        }
    }

    return res;
}

#endif /* !FF_FS_READONLY */



#if !FF_FS_READONLY && FF_FS_MINIMIZE == 0
/*-----------------------------------------------------------------------*/
/* Remove an object from the directory                                   */
/*-----------------------------------------------------------------------*/

FRESULT dir_remove (    /* FR_OK:Succeeded, FR_DISK_ERR:A disk error */
    DIR* dp                /* Directory object pointing the entry to be removed */
)
{
    FRESULT res;
    FATFS *fs = dp->obj.fs;
#if FF_USE_LFN        /* LFN configuration */
    DWORD last = dp->dptr;

    res = (dp->blk_ofs == 0xFFFFFFFF) ? FR_OK : dir_sdi(dp, dp->blk_ofs);    /* Goto top of the entry block if LFN is exist */
    if (res == FR_OK) {
        do {
            res = move_window(fs, dp->sect);
            if (res != FR_OK) break;
            /* Mark an entry 'deleted' */
            if (FF_FS_EXFAT && fs->fs_type == FS_EXFAT) {    /* On the exFAT volume */
                dp->dir[XDIR_Type] &= 0x7F;
            } else {                                    /* On the FAT/FAT32 volume */
                dp->dir[DIR_Name] = DDEM;
            }
            fs->wflag = 1;
            if (dp->dptr >= last) break;    /* If reached last entry then all entries of the object has been deleted. */
            res = dir_next(dp, 0);    /* Next entry */
        } while (res == FR_OK);
        if (res == FR_NO_FILE) res = FR_INT_ERR;
    }
#else            /* Non LFN configuration */

    res = move_window(fs, dp->sect);
    if (res == FR_OK) {
        dp->dir[DIR_Name] = DDEM;
        fs->wflag = 1;
    }
#endif

    return res;
}

#endif /* !FF_FS_READONLY && FF_FS_MINIMIZE == 0 */



#if FF_FS_MINIMIZE <= 1 || FF_FS_RPATH >= 2
/*-----------------------------------------------------------------------*/
/* Get file information from directory entry                             */
/*-----------------------------------------------------------------------*/

void get_fileinfo (        /* No return code */
    DIR* dp,            /* Pointer to the directory object */
    FILINFO* fno         /* Pointer to the file information to be filled */
)
{
    UINT i, j;
    TCHAR c;
    DWORD tm;
#if FF_USE_LFN
    WCHAR w, lfv;
    FATFS *fs = dp->obj.fs;
#endif


    fno->fname[0] = 0;            /* Invaidate file info */
    if (dp->sect == 0) return;    /* Exit if read pointer has reached end of directory */

#if FF_USE_LFN        /* LFN configuration */
#if FF_FS_EXFAT
    if (fs->fs_type == FS_EXFAT) {    /* On the exFAT volume */
        get_xdir_info(fs->dirbuf, fno);
        return;
    } else
#endif
    {    /* On the FAT/FAT32 volume */
        if (dp->blk_ofs != 0xFFFFFFFF) {    /* Get LFN if available */
            i = j = 0;
            while ((w = fs->lfnbuf[j++]) != 0) {    /* Get an LFN character */
#if !FF_LFN_UNICODE    /* ANSI/OEM API */
                w = ff_uni2oem(w, CODEPAGE);    /* Unicode -> OEM */
                if (w == 0) { i = 0; break; }    /* No LFN if it could not be converted */
                if (w >= 0x100) {                /* Put 1st byte if it is a DBC */
                    fno->fname[i++] = (char)(w >> 8);
                }
#endif
                if (i >= FF_MAX_LFN) { i = 0; break; }    /* No LFN if buffer overflow */
                fno->fname[i++] = (TCHAR)w;
            }
            fno->fname[i] = 0;    /* Terminate the LFN */
        }
    }

    i = j = 0;
    lfv = fno->fname[i];    /* LFN is exist if non-zero */
    while (i < 11) {        /* Copy name body and extension */
        c = (TCHAR)dp->dir[i++];
        if (c == ' ') continue;                /* Skip padding spaces */
        if (c == RDDEM) c = (TCHAR)DDEM;    /* Restore replaced DDEM character */
        if (i == 9) {                        /* Insert a . if extension is exist */
            if (!lfv) fno->fname[j] = '.';
            fno->altname[j++] = '.';
        }
#if FF_LFN_UNICODE    /* Unicode API */
        if (dbc_1st((BYTE)c) && i != 8 && i != 11 && dbc_2nd(dp->dir[i])) {
            c = c << 8 | dp->dir[i++];
        }
        c = ff_oem2uni(c, CODEPAGE);    /* OEM -> Unicode */
        if (!c) c = '?';
#endif
        fno->altname[j] = c;
        if (!lfv) {
            if (IsUpper(c) && (dp->dir[DIR_NTres] & ((i >= 9) ? NS_EXT : NS_BODY))) {
                c += 0x20;            /* To lower */
            }
            fno->fname[j] = c;
        }
        j++;
    }
    if (!lfv) {
        fno->fname[j] = 0;
        if (!dp->dir[DIR_NTres]) j = 0;    /* Altname is no longer needed if neither LFN nor case info is exist. */
    }
    fno->altname[j] = 0;    /* Terminate the SFN */

#else    /* Non-LFN configuration */
    i = j = 0;
    while (i < 11) {        /* Copy name body and extension */
        c = (TCHAR)dp->dir[i++];
        if (c == ' ') continue;                /* Skip padding spaces */
        if (c == RDDEM) c = (TCHAR)DDEM;    /* Restore replaced DDEM character */
        if (i == 9) fno->fname[j++] = '.';    /* Insert a . if extension is exist */
        fno->fname[j++] = c;
    }
    fno->fname[j] = 0;
#endif

    fno->fattrib = dp->dir[DIR_Attr];                /* Attribute */
    fno->fsize = ld_dword(dp->dir + DIR_FileSize);    /* Size */
    tm = ld_dword(dp->dir + DIR_ModTime);            /* Timestamp */
    fno->ftime = (WORD)tm; fno->fdate = (WORD)(tm >> 16);
}

#endif /* FF_FS_MINIMIZE <= 1 || FF_FS_RPATH >= 2 */



#if FF_USE_FIND && FF_FS_MINIMIZE <= 1
/*-----------------------------------------------------------------------*/
/* Pattern matching                                                      */
/*-----------------------------------------------------------------------*/

WCHAR get_achar (        /* Get a character and advances ptr 1 or 2 */
    const TCHAR** ptr    /* Pointer to pointer to the SBCS/DBCS/Unicode string */
)
{
    WCHAR chr;

#if FF_LFN_UNICODE && FF_USE_LFN    /* Unicode API */
    chr = ff_wtoupper(*(*ptr)++);            /* Get a Unicode char and to upper */
#else                                /* ANSI/OEM API */
    chr = (BYTE)*(*ptr)++;                    /* Get a byte */
    if (IsLower(chr)) chr -= 0x20;            /* To upper ASCII char */
#if FF_CODE_PAGE == 0
    if (ExCvt && chr >= 0x80) chr = ExCvt[chr - 0x80];    /* To upper SBCS extended char */
#elif FF_CODE_PAGE < 900
    if (chr >= 0x80) chr = ExCvt[chr - 0x80];    /* To upper SBCS extended char */
#endif
#if FF_CODE_PAGE == 0 || FF_CODE_PAGE >= 900
    if (dbc_1st((BYTE)chr) && dbc_2nd((BYTE)**ptr)) {    /* Get DBC 2nd byte if needed */
        chr = chr << 8 | (BYTE)*(*ptr)++;
    }
#endif
#endif
    return chr;
}


int pattern_matching (    /* 0:not matched, 1:matched */
    const TCHAR* pat,    /* Matching pattern */
    const TCHAR* nam,    /* String to be tested */
    int skip,            /* Number of pre-skip chars (number of ?s) */
    int inf                /* Infinite search (* specified) */
)
{
    const TCHAR *pp, *np;
    WCHAR pc, nc;
    int nm, nx;


    while (skip--) {                /* Pre-skip name chars */
        if (!get_achar(&nam)) return 0;    /* Branch mismatched if less name chars */
    }
    if (!*pat && inf) return 1;        /* (short circuit) */

    do {
        pp = pat; np = nam;            /* Top of pattern and name to match */
        for (;;) {
            if (*pp == '?' || *pp == '*') {    /* Wildcard? */
                nm = nx = 0;
                do {                /* Analyze the wildcard chars */
                    if (*pp++ == '?') nm++; else nx = 1;
                } while (*pp == '?' || *pp == '*');
                if (pattern_matching(pp, np, nm, nx)) return 1;    /* Test new branch (recurs upto number of wildcard blocks in the pattern) */
                nc = *np; break;    /* Branch mismatched */
            }
            pc = get_achar(&pp);    /* Get a pattern char */
            nc = get_achar(&np);    /* Get a name char */
            if (pc != nc) break;    /* Branch mismatched? */
            if (pc == 0) return 1;    /* Branch matched? (matched at end of both strings) */
        }
        get_achar(&nam);            /* nam++ */
    } while (inf && nc);            /* Retry until end of name if infinite search is specified */

    return 0;
}

#endif /* FF_USE_FIND && FF_FS_MINIMIZE <= 1 */



/*-----------------------------------------------------------------------*/
/* Pick a top segment and create the object name in directory form       */
/*-----------------------------------------------------------------------*/

FRESULT create_name (    /* FR_OK: successful, FR_INVALID_NAME: could not create */
    DIR* dp,            /* Pointer to the directory object */
    const TCHAR** path    /* Pointer to pointer to the segment in the path string */
)
{
#if FF_USE_LFN        /* LFN configuration */
    BYTE b, cf;
    WCHAR w, *lfn;
    UINT i, ni, si, di;
    const TCHAR *p;


    /* Create LFN in Unicode */
    p = *path; lfn = dp->obj.fs->lfnbuf; si = di = 0;
    for (;;) {
        w = p[si++];                    /* Get a character */
        if (w < ' ') break;                /* Break if end of the path name */
        if (w == '/' || w == '\\') {    /* Break if a separator is found */
            while (p[si] == '/' || p[si] == '\\') si++;    /* Skip duplicated separator if exist */
            break;
        }
        if (di >= FF_MAX_LFN) return FR_INVALID_NAME;    /* Reject too long name */
#if !FF_LFN_UNICODE        /* ANSI/OEM API */
        w &= 0xFF;
        if (dbc_1st((BYTE)w)) {            /* Check if it is a DBC 1st byte */
            b = (BYTE)p[si++];            /* Get 2nd byte */
            w = (w << 8) + b;            /* Create a DBC */
            if (!dbc_2nd(b)) return FR_INVALID_NAME;    /* Reject invalid sequence */
        }
        w = ff_oem2uni(w, CODEPAGE);    /* Convert ANSI/OEM to Unicode */
        if (!w) return FR_INVALID_NAME;    /* Reject invalid code */
#endif
        if (w < 0x80 && chk_chr("\"*:<>\?|\x7F", w)) return FR_INVALID_NAME;    /* Reject illegal characters for LFN */
        lfn[di++] = w;                    /* Store the Unicode character */
    }
    *path = &p[si];                        /* Return pointer to the next segment */
    cf = (w < ' ') ? NS_LAST : 0;        /* Set last segment flag if end of the path */
#if FF_FS_RPATH != 0
    if ((di == 1 && lfn[di - 1] == '.') ||
        (di == 2 && lfn[di - 1] == '.' && lfn[di - 2] == '.')) {    /* Is this segment a dot name? */
        lfn[di] = 0;
        for (i = 0; i < 11; i++)        /* Create dot name for SFN entry */
            dp->fn[i] = (i < di) ? '.' : ' ';
        dp->fn[i] = cf | NS_DOT;        /* This is a dot entry */
        return FR_OK;
    }
#endif
    while (di) {                        /* Snip off trailing spaces and dots if exist */
        w = lfn[di - 1];
        if (w != ' ' && w != '.') break;
        di--;
    }
    lfn[di] = 0;                        /* LFN is created */
    if (di == 0) return FR_INVALID_NAME;    /* Reject nul name */

    /* Create SFN in directory form */
    MEMSET(dp->fn, ' ', 11);
    for (si = 0; lfn[si] == ' ' || lfn[si] == '.'; si++) ;    /* Strip leading spaces and dots */
    if (si > 0) cf |= NS_LOSS | NS_LFN;
    while (di > 0 && lfn[di - 1] != '.') di--;    /* Find extension (di<=si: no extension) */

    i = b = 0; ni = 8;
    for (;;) {
        w = lfn[si++];                    /* Get an LFN character */
        if (!w) break;                    /* Break on end of the LFN */
        if (w == ' ' || (w == '.' && si != di)) {    /* Remove spaces and dots */
            cf |= NS_LOSS | NS_LFN;
            continue;
        }

        if (i >= ni || si == di) {        /* Entered extension or end of SFN */
            if (ni == 11) {                /* Extension fileld overflow? */
                cf |= NS_LOSS | NS_LFN;
                break;
            }
            if (si != di) cf |= NS_LOSS | NS_LFN;    /* Out of 8.3 format */
            if (si > di) break;            /* No extension */
            si = di; i = 8; ni = 11;    /* Enter extension fileld */
            b <<= 2; continue;
        }

        if (w >= 0x80) {                /* Is this a non-ASCII character? */
            cf |= NS_LFN;                /* Force to create LFN entry */
#if FF_CODE_PAGE == 0
            if (ExCvt) {    /* In SBCS */
                w = ff_uni2oem(w, CODEPAGE);    /* Unicode -> OEM code */
                if (w & 0x80) w = ExCvt[w & 0x7F];    /* Convert extended character to upper (SBCS) */
            } else {        /* In DBCS */
                w = ff_uni2oem(ff_wtoupper(w), CODEPAGE);    /* Upper converted Unicode -> OEM code */
            }
#elif FF_CODE_PAGE < 900    /* SBCS cfg */
            w = ff_uni2oem(w, CODEPAGE);    /* Unicode -> OEM code */
            if (w & 0x80) w = ExCvt[w & 0x7F];    /* Convert extended character to upper (SBCS) */
#else                    /* DBCS cfg */
            w = ff_uni2oem(ff_wtoupper(w), CODEPAGE);    /* Upper converted Unicode -> OEM code */
#endif
        }

        if (w >= 0x100) {                /* Is this a DBC? */
            if (i >= ni - 1) {            /* Field overflow? */
                cf |= NS_LOSS | NS_LFN;
                i = ni; continue;        /* Next field */
            }
            dp->fn[i++] = (BYTE)(w >> 8);    /* Put 1st byte */
        } else {                        /* SBC */
            if (!w || chk_chr("+,;=[]", w)) {    /* Replace illegal characters for SFN */
                w = '_'; cf |= NS_LOSS | NS_LFN;/* Lossy conversion */
            } else {
                if (IsUpper(w)) {        /* ASCII large capital */
                    b |= 2;
                } else {
                    if (IsLower(w)) {    /* ASCII small capital */
                        b |= 1; w -= 0x20;
                    }
                }
            }
        }
        dp->fn[i++] = (BYTE)w;
    }

    if (dp->fn[0] == DDEM) dp->fn[0] = RDDEM;    /* If the first character collides with DDEM, replace it with RDDEM */

    if (ni == 8) b <<= 2;
    if ((b & 0x0C) == 0x0C || (b & 0x03) == 0x03) cf |= NS_LFN;    /* Create LFN entry when there are composite capitals */
    if (!(cf & NS_LFN)) {                        /* When LFN is in 8.3 format without extended character, NT flags are created */
        if ((b & 0x03) == 0x01) cf |= NS_EXT;    /* NT flag (Extension has only small capital) */
        if ((b & 0x0C) == 0x04) cf |= NS_BODY;    /* NT flag (Filename has only small capital) */
    }

    dp->fn[NSFLAG] = cf;    /* SFN is created */

    return FR_OK;


#else    /* FF_USE_LFN : Non-LFN configuration */
    BYTE c, d, *sfn;
    UINT ni, si, i;
    const char *p;

    /* Create file name in directory form */
    p = *path; sfn = dp->fn;
    MEMSET(sfn, ' ', 11);
    si = i = 0; ni = 8;
#if FF_FS_RPATH != 0
    if (p[si] == '.') { /* Is this a dot entry? */
        for (;;) {
            c = (BYTE)p[si++];
            if (c != '.' || si >= 3) break;
            sfn[i++] = c;
        }
        if (c != '/' && c != '\\' && c > ' ') return FR_INVALID_NAME;
        *path = p + si;                                /* Return pointer to the next segment */
        sfn[NSFLAG] = (c <= ' ') ? NS_LAST | NS_DOT : NS_DOT;    /* Set last segment flag if end of the path */
        return FR_OK;
    }
#endif
    for (;;) {
        c = (BYTE)p[si++];
        if (c <= ' ') break;             /* Break if end of the path name */
        if (c == '/' || c == '\\') {    /* Break if a separator is found */
            while (p[si] == '/' || p[si] == '\\') si++;    /* Skip duplicated separator if exist */
            break;
        }
        if (c == '.' || i >= ni) {        /* End of body or over size? */
            if (ni == 11 || c != '.') return FR_INVALID_NAME;    /* Over size or invalid dot */
            i = 8; ni = 11;                /* Goto extension */
            continue;
        }
#if FF_CODE_PAGE == 0
        if (ExCvt && c >= 0x80) {        /* Is SBC extended character? */
            c = ExCvt[c - 0x80];        /* To upper SBC extended character */
        }
#elif FF_CODE_PAGE < 900
        if (c >= 0x80) {                /* Is SBC extended character? */
            c = ExCvt[c - 0x80];        /* To upper SBC extended character */
        }
#endif
        if (dbc_1st(c)) {                /* Check if it is a DBC 1st byte */
            d = (BYTE)p[si++];            /* Get 2nd byte */
            if (!dbc_2nd(d) || i >= ni - 1) return FR_INVALID_NAME;    /* Reject invalid DBC */
            sfn[i++] = c;
            sfn[i++] = d;
        } else {                        /* SBC */
            if (chk_chr("\"*+,:;<=>\?[]|\x7F", c)) return FR_INVALID_NAME;    /* Reject illegal chrs for SFN */
            if (IsLower(c)) c -= 0x20;    /* To upper */
            sfn[i++] = c;
        }
    }
    *path = p + si;                        /* Return pointer to the next segment */
    if (i == 0) return FR_INVALID_NAME;    /* Reject nul string */

    if (sfn[0] == DDEM) sfn[0] = RDDEM;    /* If the first character collides with DDEM, replace it with RDDEM */
    sfn[NSFLAG] = (c <= ' ') ? NS_LAST : 0;        /* Set last segment flag if end of the path */

    return FR_OK;
#endif /* FF_USE_LFN */
}




/*-----------------------------------------------------------------------*/
/* Follow a file path                                                    */
/*-----------------------------------------------------------------------*/

FRESULT follow_path (    /* FR_OK(0): successful, !=0: error code */
    DIR* dp,            /* Directory object to return last directory and found object */
    const TCHAR* path    /* Full-path string to find a file or directory */
)
{
    FRESULT res;
    BYTE ns;
    FATFS *fs = dp->obj.fs;


#if FF_FS_RPATH != 0
    if (*path != '/' && *path != '\\') {    /* Without heading separator */
        dp->obj.sclust = fs->cdir;                /* Start from current directory */
    } else
#endif
    {                                        /* With heading separator */
        while (*path == '/' || *path == '\\') path++;    /* Strip heading separator */
        dp->obj.sclust = 0;                    /* Start from root directory */
    }
#if FF_FS_EXFAT
    dp->obj.n_frag = 0;    /* Invalidate last fragment counter of the object */
#if FF_FS_RPATH != 0
    if (fs->fs_type == FS_EXFAT && dp->obj.sclust) {    /* exFAT: Retrieve the sub-directory's status */
        DIR dj;

        dp->obj.c_scl = fs->cdc_scl;
        dp->obj.c_size = fs->cdc_size;
        dp->obj.c_ofs = fs->cdc_ofs;
        res = load_obj_xdir(&dj, &dp->obj);
        if (res != FR_OK) return res;
        dp->obj.objsize = ld_dword(fs->dirbuf + XDIR_FileSize);
        dp->obj.stat = fs->dirbuf[XDIR_GenFlags] & 2;
    }
#endif
#endif

    if ((UINT)*path < ' ') {                /* Null path name is the origin directory itself */
        dp->fn[NSFLAG] = NS_NONAME;
        res = dir_sdi(dp, 0);

    } else {                                /* Follow path */
        for (;;) {
            res = create_name(dp, &path);    /* Get a segment name of the path */
            if (res != FR_OK) break;
            res = dir_find(dp);                /* Find an object with the segment name */
            ns = dp->fn[NSFLAG];
            if (res != FR_OK) {                /* Failed to find the object */
                if (res == FR_NO_FILE) {    /* Object is not found */
                    if (FF_FS_RPATH && (ns & NS_DOT)) {    /* If dot entry is not exist, stay there */
                        if (!(ns & NS_LAST)) continue;    /* Continue to follow if not last segment */
                        dp->fn[NSFLAG] = NS_NONAME;
                        res = FR_OK;
                    } else {                            /* Could not find the object */
                        if (!(ns & NS_LAST)) res = FR_NO_PATH;    /* Adjust error code if not last segment */
                    }
                }
                break;
            }
            if (ns & NS_LAST) break;            /* Last segment matched. Function completed. */
            /* Get into the sub-directory */
            if (!(dp->obj.attr & AM_DIR)) {        /* It is not a sub-directory and cannot follow */
                res = FR_NO_PATH; break;
            }
#if FF_FS_EXFAT
            if (fs->fs_type == FS_EXFAT) {        /* Save containing directory information for next dir */
                dp->obj.c_scl = dp->obj.sclust;
                dp->obj.c_size = ((DWORD)dp->obj.objsize & 0xFFFFFF00) | dp->obj.stat;
                dp->obj.c_ofs = dp->blk_ofs;
                dp->obj.sclust = ld_dword(fs->dirbuf + XDIR_FstClus);    /* Open next directory */
                dp->obj.stat = fs->dirbuf[XDIR_GenFlags] & 2;
                dp->obj.objsize = ld_qword(fs->dirbuf + XDIR_FileSize);
            } else
#endif
            {
                dp->obj.sclust = ld_clust(fs, fs->win + dp->dptr % SS(fs));    /* Open next directory */
            }
        }
    }

    return res;
}




/*-----------------------------------------------------------------------*/
/* Get logical drive number from path name                               */
/*-----------------------------------------------------------------------*/

int get_ldnumber (        /* Returns logical drive number (-1:invalid drive) */
    const TCHAR** path    /* Pointer to pointer to the path name */
)
{
    const TCHAR *tp, *tt;
    UINT i;
    int vol = -1;
#if FF_STR_VOLUME_ID        /* Find string drive id */
    static const char* const volid[] = {FF_VOLUME_STRS};
    const char *sp;
    char c;
    TCHAR tc;
#endif


    if (*path) {    /* If the pointer is not a null */
        for (tt = *path; (UINT)*tt >= (FF_USE_LFN ? ' ' : '!') && *tt != ':'; tt++) ;    /* Find a colon in the path */
        if (*tt == ':') {    /* If a colon is exist in the path name */
            tp = *path;
            i = *tp++;
            if (IsDigit(i) && tp == tt) {    /* Is there a numeric drive id + colon? */
                if ((i -= '0') < FF_VOLUMES) {    /* If drive id is found, get the value and strip it */
                    vol = (int)i;
                    *path = ++tt;
                }
            }
#if FF_STR_VOLUME_ID
             else {    /* No numeric drive number, find string drive id */
                i = 0; tt++;
                do {
                    sp = volid[i]; tp = *path;
                    do {    /* Compare a string drive id with path name */
                        c = *sp++; tc = *tp++;
                        if (IsLower(tc)) tc -= 0x20;
                    } while (c && (TCHAR)c == tc);
                } while ((c || tp != tt) && ++i < FF_VOLUMES);    /* Repeat for each id until pattern match */
                if (i < FF_VOLUMES) {    /* If a drive id is found, get the value and strip it */
                    vol = (int)i;
                    *path = tt;
                }
            }
#endif
        } else {    /* No volume id and use default drive */
#if FF_FS_RPATH != 0 && FF_VOLUMES >= 2
            vol = CurrVol;    /* Current drive */
#else
            vol = 0;        /* Drive 0 */
#endif
        }
    }
    return vol;
}




/*-----------------------------------------------------------------------*/
/* Load a sector and check if it is an FAT VBR                           */
/*-----------------------------------------------------------------------*/

BYTE check_fs (    /* 0:FAT, 1:exFAT, 2:Valid BS but not FAT, 3:Not a BS, 4:Disk error */
    FATFS* fs,    /* Filesystem object */
    DWORD sect    /* Sector# (lba) to load and check if it is an FAT-VBR or not */
)
{
    fs->wflag = 0; fs->winsect = 0xFFFFFFFF;        /* Invaidate window */
    if (move_window(fs, sect) != FR_OK) return 4;    /* Load boot record */

    if (ld_word(fs->win + BS_55AA) != 0xAA55) return 3;    /* Check boot record signature (always placed here even if the sector size is >512) */

#if FF_FS_EXFAT
    if (!MEMCMP(fs->win + BS_JmpBoot, "\xEB\x76\x90" "EXFAT   ", 11)) return 1;    /* Check if exFAT VBR */
#endif
    if (fs->win[BS_JmpBoot] == 0xE9 || (fs->win[BS_JmpBoot] == 0xEB && fs->win[BS_JmpBoot + 2] == 0x90)) {    /* Valid JumpBoot code? */
        if (!MEMCMP(fs->win + BS_FilSysType, "FAT", 3)) return 0;        /* Is it an FAT VBR? */
        if (!MEMCMP(fs->win + BS_FilSysType32, "FAT32", 5)) return 0;    /* Is it an FAT32 VBR? */
    }
    return 2;    /* Valid BS but not FAT */
}




/*-----------------------------------------------------------------------*/
/* Find logical drive and check if the volume is mounted                 */
/*-----------------------------------------------------------------------*/

FRESULT find_volume (    /* FR_OK(0): successful, !=0: any error occurred */
    const TCHAR** path,    /* Pointer to pointer to the path name (drive number) */
    FATFS** rfs,        /* Pointer to pointer to the found filesystem object */
    BYTE mode            /* !=0: Check write protection for write access */
)
{
    BYTE fmt, *pt;
    int vol;
    DSTATUS stat;
    DWORD bsect, fasize, tsect, sysect, nclst, szbfat, br[4];
    WORD nrsv;
    FATFS *fs;
    UINT i;


    /* Get logical drive number */
    *rfs = 0;
    vol = get_ldnumber(path);
    if (vol < 0) return FR_INVALID_DRIVE;

    /* Check if the filesystem object is valid or not */
    fs = FatFs[vol];                    /* Get pointer to the filesystem object */
    if (!fs) return FR_NOT_ENABLED;        /* Is the filesystem object available? */
#if FF_FS_REENTRANT
    if (!lock_fs(fs)) return FR_TIMEOUT;    /* Lock the volume */
#endif
    *rfs = fs;                            /* Return pointer to the filesystem object */

    mode &= (BYTE)~FA_READ;                /* Desired access mode, write access or not */
    if (fs->fs_type != 0) {                /* If the volume has been mounted */
        stat = disk_status(fs->pdrv);
        if (!(stat & STA_NOINIT)) {        /* and the physical drive is kept initialized */
            if (!FF_FS_READONLY && mode && (stat & STA_PROTECT)) {    /* Check write protection if needed */
                return FR_WRITE_PROTECTED;
            }
            return FR_OK;                /* The filesystem object is valid */
        }
    }

    /* The filesystem object is not valid. */
    /* Following code attempts to mount the volume. (analyze BPB and initialize the filesystem object) */

    fs->fs_type = 0;                    /* Clear the filesystem object */
    fs->pdrv = LD2PD(vol);                /* Bind the logical drive and a physical drive */
    stat = disk_initialize(fs->pdrv);    /* Initialize the physical drive */
    if (stat & STA_NOINIT) {             /* Check if the initialization succeeded */
        return FR_NOT_READY;            /* Failed to initialize due to no medium or hard error */
    }
    if (!FF_FS_READONLY && mode && (stat & STA_PROTECT)) { /* Check disk write protection if needed */
        return FR_WRITE_PROTECTED;
    }
#if FF_MAX_SS != FF_MIN_SS                /* Get sector size (multiple sector size cfg only) */
    if (disk_ioctl(fs->pdrv, GET_SECTOR_SIZE, &SS(fs)) != RES_OK) return FR_DISK_ERR;
    if (SS(fs) > FF_MAX_SS || SS(fs) < FF_MIN_SS || (SS(fs) & (SS(fs) - 1))) return FR_DISK_ERR;
#endif

    /* Find an FAT partition on the drive. Supports only generic partitioning rules, FDISK and SFD. */
    bsect = 0;
    fmt = check_fs(fs, bsect);            /* Load sector 0 and check if it is an FAT-VBR as SFD */
    if (fmt == 2 || (fmt < 2 && LD2PT(vol) != 0)) {    /* Not an FAT-VBR or forced partition number */
        for (i = 0; i < 4; i++) {        /* Get partition offset */
            pt = fs->win + (MBR_Table + i * SZ_PTE);
            br[i] = pt[PTE_System] ? ld_dword(pt + PTE_StLba) : 0;
        }
        i = LD2PT(vol);                    /* Partition number: 0:auto, 1-4:forced */
        if (i != 0) i--;
        do {                            /* Find an FAT volume */
            bsect = br[i];
            fmt = bsect ? check_fs(fs, bsect) : 3;    /* Check the partition */
        } while (LD2PT(vol) == 0 && fmt >= 2 && ++i < 4);
    }
    if (fmt == 4) return FR_DISK_ERR;        /* An error occured in the disk I/O layer */
    if (fmt >= 2) return FR_NO_FILESYSTEM;    /* No FAT volume is found */

    /* An FAT volume is found (bsect). Following code initializes the filesystem object */

#if FF_FS_EXFAT
    if (fmt == 1) {
        QWORD maxlba;

        for (i = BPB_ZeroedEx; i < BPB_ZeroedEx + 53 && fs->win[i] == 0; i++) ;    /* Check zero filler */
        if (i < BPB_ZeroedEx + 53) return FR_NO_FILESYSTEM;

        if (ld_word(fs->win + BPB_FSVerEx) != 0x100) return FR_NO_FILESYSTEM;    /* Check exFAT version (must be version 1.0) */

        if (1 << fs->win[BPB_BytsPerSecEx] != SS(fs)) {    /* (BPB_BytsPerSecEx must be equal to the physical sector size) */
            return FR_NO_FILESYSTEM;
        }

        maxlba = ld_qword(fs->win + BPB_TotSecEx) + bsect;    /* Last LBA + 1 of the volume */
        if (maxlba >= 0x100000000) return FR_NO_FILESYSTEM;    /* (It cannot be handled in 32-bit LBA) */

        fs->fsize = ld_dword(fs->win + BPB_FatSzEx);    /* Number of sectors per FAT */

        fs->n_fats = fs->win[BPB_NumFATsEx];            /* Number of FATs */
        if (fs->n_fats != 1) return FR_NO_FILESYSTEM;    /* (Supports only 1 FAT) */

        fs->csize = 1 << fs->win[BPB_SecPerClusEx];        /* Cluster size */
        if (fs->csize == 0)    return FR_NO_FILESYSTEM;    /* (Must be 1..32768) */

        nclst = ld_dword(fs->win + BPB_NumClusEx);        /* Number of clusters */
        if (nclst > MAX_EXFAT) return FR_NO_FILESYSTEM;    /* (Too many clusters) */
        fs->n_fatent = nclst + 2;

        /* Boundaries and Limits */
        fs->volbase = bsect;
        fs->database = bsect + ld_dword(fs->win + BPB_DataOfsEx);
        fs->fatbase = bsect + ld_dword(fs->win + BPB_FatOfsEx);
        if (maxlba < (QWORD)fs->database + nclst * fs->csize) return FR_NO_FILESYSTEM;    /* (Volume size must not be smaller than the size requiered) */
        fs->dirbase = ld_dword(fs->win + BPB_RootClusEx);

        /* Check if bitmap location is in assumption (at the first cluster) */
        if (move_window(fs, clst2sect(fs, fs->dirbase)) != FR_OK) return FR_DISK_ERR;
        for (i = 0; i < SS(fs); i += SZDIRE) {
            if (fs->win[i] == 0x81 && ld_dword(fs->win + i + 20) == 2) break;    /* 81 entry with cluster #2? */
        }
        if (i == SS(fs)) return FR_NO_FILESYSTEM;
#if !FF_FS_READONLY
        fs->last_clst = fs->free_clst = 0xFFFFFFFF;        /* Initialize cluster allocation information */
#endif
        fmt = FS_EXFAT;            /* FAT sub-type */
    } else
#endif    /* FF_FS_EXFAT */
    {
        if (ld_word(fs->win + BPB_BytsPerSec) != SS(fs)) return FR_NO_FILESYSTEM;    /* (BPB_BytsPerSec must be equal to the physical sector size) */

        fasize = ld_word(fs->win + BPB_FATSz16);        /* Number of sectors per FAT */
        if (fasize == 0) fasize = ld_dword(fs->win + BPB_FATSz32);
        fs->fsize = fasize;

        fs->n_fats = fs->win[BPB_NumFATs];                /* Number of FATs */
        if (fs->n_fats != 1 && fs->n_fats != 2) return FR_NO_FILESYSTEM;    /* (Must be 1 or 2) */
        fasize *= fs->n_fats;                            /* Number of sectors for FAT area */

        fs->csize = fs->win[BPB_SecPerClus];            /* Cluster size */
        if (fs->csize == 0 || (fs->csize & (fs->csize - 1))) return FR_NO_FILESYSTEM;    /* (Must be power of 2) */

        fs->n_rootdir = ld_word(fs->win + BPB_RootEntCnt);    /* Number of root directory entries */
        if (fs->n_rootdir % (SS(fs) / SZDIRE)) return FR_NO_FILESYSTEM;    /* (Must be sector aligned) */

        tsect = ld_word(fs->win + BPB_TotSec16);        /* Number of sectors on the volume */
        if (tsect == 0) tsect = ld_dword(fs->win + BPB_TotSec32);

        nrsv = ld_word(fs->win + BPB_RsvdSecCnt);        /* Number of reserved sectors */
        if (nrsv == 0) return FR_NO_FILESYSTEM;            /* (Must not be 0) */

        /* Determine the FAT sub type */
        sysect = nrsv + fasize + fs->n_rootdir / (SS(fs) / SZDIRE);    /* RSV + FAT + DIR */
        if (tsect < sysect) return FR_NO_FILESYSTEM;    /* (Invalid volume size) */
        nclst = (tsect - sysect) / fs->csize;            /* Number of clusters */
        if (nclst == 0) return FR_NO_FILESYSTEM;        /* (Invalid volume size) */
        fmt = 0;
        if (nclst <= MAX_FAT32) fmt = FS_FAT32;
        if (nclst <= MAX_FAT16) fmt = FS_FAT16;
        if (nclst <= MAX_FAT12) fmt = FS_FAT12;
        if (fmt == 0) return FR_NO_FILESYSTEM;

        /* Boundaries and Limits */
        fs->n_fatent = nclst + 2;                        /* Number of FAT entries */
        fs->volbase = bsect;                            /* Volume start sector */
        fs->fatbase = bsect + nrsv;                     /* FAT start sector */
        fs->database = bsect + sysect;                    /* Data start sector */
        if (fmt == FS_FAT32) {
            if (ld_word(fs->win + BPB_FSVer32) != 0) return FR_NO_FILESYSTEM;    /* (Must be FAT32 revision 0.0) */
            if (fs->n_rootdir != 0) return FR_NO_FILESYSTEM;    /* (BPB_RootEntCnt must be 0) */
            fs->dirbase = ld_dword(fs->win + BPB_RootClus32);    /* Root directory start cluster */
            szbfat = fs->n_fatent * 4;                    /* (Needed FAT size) */
        } else {
            if (fs->n_rootdir == 0)    return FR_NO_FILESYSTEM;    /* (BPB_RootEntCnt must not be 0) */
            fs->dirbase = fs->fatbase + fasize;            /* Root directory start sector */
            szbfat = (fmt == FS_FAT16) ?                /* (Needed FAT size) */
                fs->n_fatent * 2 : fs->n_fatent * 3 / 2 + (fs->n_fatent & 1);
        }
        if (fs->fsize < (szbfat + (SS(fs) - 1)) / SS(fs)) return FR_NO_FILESYSTEM;    /* (BPB_FATSz must not be less than the size needed) */

#if !FF_FS_READONLY
        /* Get FSInfo if available */
        fs->last_clst = fs->free_clst = 0xFFFFFFFF;        /* Initialize cluster allocation information */
        fs->fsi_flag = 0x80;
#if (FF_FS_NOFSINFO & 3) != 3
        if (fmt == FS_FAT32                /* Allow to update FSInfo only if BPB_FSInfo32 == 1 */
            && ld_word(fs->win + BPB_FSInfo32) == 1
            && move_window(fs, bsect + 1) == FR_OK)
        {
            fs->fsi_flag = 0;
            if (ld_word(fs->win + BS_55AA) == 0xAA55    /* Load FSInfo data if available */
                && ld_dword(fs->win + FSI_LeadSig) == 0x41615252
                && ld_dword(fs->win + FSI_StrucSig) == 0x61417272)
            {
#if (FF_FS_NOFSINFO & 1) == 0
                fs->free_clst = ld_dword(fs->win + FSI_Free_Count);
#endif
#if (FF_FS_NOFSINFO & 2) == 0
                fs->last_clst = ld_dword(fs->win + FSI_Nxt_Free);
#endif
            }
        }
#endif    /* (FF_FS_NOFSINFO & 3) != 3 */
#endif    /* !FF_FS_READONLY */
    }

    fs->fs_type = fmt;        /* FAT sub-type */
    fs->id = ++Fsid;        /* Volume mount ID */
#if FF_USE_LFN == 1
    fs->lfnbuf = LfnBuf;    /* Static LFN working buffer */
#if FF_FS_EXFAT
    fs->dirbuf = DirBuf;    /* Static directory block scratchpad buuffer */
#endif
#endif
#if FF_FS_RPATH != 0
    fs->cdir = 0;            /* Initialize current directory */
#endif
#if FF_FS_LOCK != 0            /* Clear file lock semaphores */
    clear_lock(fs);
#endif
    return FR_OK;
}




/*-----------------------------------------------------------------------*/
/* Check if the file/directory object is valid or not                    */
/*-----------------------------------------------------------------------*/

FRESULT validate (    /* Returns FR_OK or FR_INVALID_OBJECT */
    FFOBJID* obj,    /* Pointer to the FFOBJID, the 1st member in the FIL/DIR object, to check validity */
    FATFS** rfs        /* Pointer to pointer to the owner filesystem object to return */
)
{
    FRESULT res = FR_INVALID_OBJECT;


    if (obj && obj->fs && obj->fs->fs_type && obj->id == obj->fs->id) {    /* Test if the object is valid */
#if FF_FS_REENTRANT
        if (lock_fs(obj->fs)) {    /* Obtain the filesystem object */
            if (!(disk_status(obj->fs->pdrv) & STA_NOINIT)) { /* Test if the phsical drive is kept initialized */
                res = FR_OK;
            } else {
                unlock_fs(obj->fs, FR_OK);
            }
        } else {
            res = FR_TIMEOUT;
        }
#else
        if (!(disk_status(obj->fs->pdrv) & STA_NOINIT)) { /* Test if the phsical drive is kept initialized */
            res = FR_OK;
        }
#endif
    }
    *rfs = (res == FR_OK) ? obj->fs : 0;    /* Corresponding filesystem object */
    return res;
}



