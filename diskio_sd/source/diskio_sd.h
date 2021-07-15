/*------------------------------------------------------------------------/
/
/  Copyright (C) 2019, feilipu, all right reserved.
/
/ * This software is a free software and there is NO WARRANTY.
/ * No restriction on use. You can use, modify and redistribute it for
/   personal, non-profit or commercial products UNDER YOUR RESPONSIBILITY.
/ * Redistributions of source code must retain the above copyright notice.
/
/------------------------------------------------------------------------*/

#ifndef __DISKIO_SD_H__
#define __DISKIO_SD_H__

/*
 * Disk Status Bits DSTATUS (uint8_t)
 *
 */

#define STA_NOINIT          0x01    /* Drive not initialised */
#define STA_NODISK          0x02    /* No medium in the drive */
#define STA_PROTECT         0x04    /* Write protected */

/*
 * Command codes for disk_ioctrl function
 *
 */

/* Generic command (Used by FatFs) */
#define CTRL_SYNC           0       /* Complete pending write process (needed at _FS_READONLY == 0) */
#define GET_SECTOR_COUNT    1       /* Get media size (needed at _USE_MKFS == 1) */
#define GET_SECTOR_SIZE     2       /* Get sector size (needed at _MAX_SS != _MIN_SS) */
#define GET_BLOCK_SIZE      3       /* Get erase block size (needed at _USE_MKFS == 1) */
#define CTRL_TRIM           4       /* Inform device that the data on the block of sectors is no longer used (needed at _USE_TRIM == 1) */

/* Generic command (not used by FatFs) */
#define CTRL_POWER          5       /* Get/Set power status */
#define CTRL_LOCK           6       /* Lock/Unlock media removal */
#define CTRL_EJECT          7       /* Eject media */
#define CTRL_FORMAT         8       /* Create physical format on the media */

/* MMC/SDC specific ioctl command */
#define MMC_GET_TYPE        10      /* Get card type */
#define MMC_GET_CSD         11      /* Get CSD */
#define MMC_GET_CID         12      /* Get CID */
#define MMC_GET_OCR         13      /* Get OCR */
#define MMC_GET_SDSTAT      14      /* Get SD status */
#define ISDIO_READ          55      /* Read data form SD iSDIO register */
#define ISDIO_WRITE         56      /* Write data to SD iSDIO register */
#define ISDIO_MRITE         57      /* Masked write data to SD iSDIO register */

/* ATA/CF specific ioctl command */
#define ATA_GET_REV         20      /* Get F/W revision */
#define ATA_GET_MODEL       21      /* Get model name */
#define ATA_GET_SN          22      /* Get serial number */

/* NAND specific ioctl command */
#define NAND_FORMAT         30      /* Create physical format */

/* MMC card type flags (MMC_GET_TYPE) */
#define CT_MMC              0x01    /* MMC ver 3 */
#define CT_SD1              0x02    /* SD ver 1 */
#define CT_SD2              0x04    /* SD ver 2 */
#define CT_SDC              (CT_SD1|CT_SD2) /* SD */
#define CT_BLOCK            0x08    /* Block addressing */

/* Integer types used for FatFs API */

#if defined(_WIN32)         /* Windows VC++ (for development only) */
#define FF_INTDEF 2
#include <windows.h>
typedef unsigned __int64    QWORD;
#include <float.h>
#define isnan(v) _isnan(v)
#define isinf(v) (!_finite(v))

#elif (defined(__STDC_VERSION__) && __STDC_VERSION__ >= 199901L) || defined(__cplusplus)    /* C99 or later */
#define FF_INTDEF 2
#include <stdint.h>
typedef unsigned char       BYTE;   /* char must be 8-bit */
typedef unsigned int        UINT;   /* int must be 16-bit or 32-bit */
typedef uint16_t            WORD;   /* 16-bit unsigned integer */
typedef uint16_t            WCHAR;  /* 16-bit unsigned integer */
typedef uint32_t            DWORD;  /* 32-bit unsigned integer */
#ifndef __SCCZ80
typedef uint64_t            QWORD;  /* 64-bit unsigned integer */
#endif
#else   /* Earlier than C99 */
#define FF_INTDEF 1
typedef unsigned char       BYTE;   /* char must be 8-bit */
typedef unsigned int        UINT;   /* int must be 16-bit or 32-bit */
typedef unsigned short      WORD;   /* 16-bit unsigned integer */
typedef unsigned short      WCHAR;  /* 16-bit unsigned integer */
typedef unsigned long       DWORD;  /* 32-bit unsigned integer */
#endif

/* Status of Disk Functions */
typedef BYTE DSTATUS;

/* Results of Disk Functions */
typedef enum {
    RES_OK = 0,     /* 0: Successful */
    RES_ERROR = 1,  /* 1: R/W Error */
    RES_WRPRT = 2,  /* 2: Write Protected */
    RES_NOTRDY = 3, /* 3: Not Ready */
    RES_PARERR = 4  /* 4: Invalid Parameter */
} DRESULT;

/* FatFS for non exFAT file systems */
typedef DWORD LBA_t;

#if __SDCC
DSTATUS disk_initialize_fastcall (
    BYTE pdrv               /* Physical drive number (0 or 1) */
) __preserves_regs(iyh,iyl) __z88dk_fastcall;
#elif __SCCZ80
DSTATUS __LIB__ disk_initialize (
    BYTE pdrv               /* Physical drive number (0 or 1) */
)  __smallc __z88dk_fastcall;
#endif

#if __SDCC
DSTATUS disk_status_fastcall (
    BYTE pdrv              /* Physical drive number (0 or 1) */
) __preserves_regs(iyh,iyl) __z88dk_fastcall;
#elif __SCCZ80
DSTATUS __LIB__ disk_status (
    BYTE pdrv              /* Physical drive number (0 or 1) */
) __smallc __z88dk_fastcall;
#endif

#if __SDCC
DRESULT disk_read (
    BYTE pdrv,              /* Physical drive number (0 or 1) */
    BYTE *buff,             /* Pointer to the data buffer to store read data */
    LBA_t sector,           /* Start sector number (LBA) */
    UINT count              /* Sector count (1..128) */
) __preserves_regs(iyh,iyl);
#elif __SCCZ80
DRESULT __LIB__ disk_read (
    BYTE pdrv,              /* Physical drive number (0 or 1) */
    BYTE *buff,             /* Pointer to the data buffer to store read data */
    LBA_t sector,           /* Start sector number (LBA) */
    UINT count              /* Sector count (1..128) */
) __smallc;
#endif

#if __SDCC
DRESULT disk_write (
    BYTE pdrv,              /* Physical drive number (0 or 1) */
    const BYTE *buff,       /* Pointer to the data to be written */
    LBA_t sector,           /* Start sector number (LBA) */
    UINT count              /* Sector count (1..128) */
) __preserves_regs(iyh,iyl);
#elif __SCCZ80
DRESULT __LIB__ disk_write (
    BYTE pdrv,              /* Physical drive number (0 or 1 ) */
    const BYTE *buff,       /* Pointer to the data to be written */
    LBA_t sector,           /* Start sector number (LBA) */
    UINT count              /* Sector count (1..128) */
) __smallc;
#endif

#if __SDCC
DRESULT disk_ioctl (
    BYTE pdrv,              /* Physical drive number (0 or 1) */
    BYTE cmd,               /* Control code */
    void *buff              /* Buffer to send/receive control data */
) __preserves_regs(iyh,iyl);
#elif __SCCZ80
DRESULT __LIB__ disk_ioctl (
    BYTE pdrv,              /* Physical drive number (0 or 1) */
    BYTE cmd,               /* Control code */
    void *buff              /* Buffer to send/receive control data */
) __smallc;
#endif

#endif /* !__DISKIO_SD_H__ */
