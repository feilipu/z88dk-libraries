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
/* Create an FAT/exFAT volume                                            */
/*-----------------------------------------------------------------------*/
#if FF_USE_MKFS && !FF_FS_READONLY

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

#endif /* FF_USE_MKFS && !FF_FS_READONLY */

