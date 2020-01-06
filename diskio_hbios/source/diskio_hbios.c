/*------------------------------------------------------------------------/
/  Foolproof HBIOS control module
/-------------------------------------------------------------------------/
/
/  Copyright (C) 2019, feilipu, all right reserved.
/
/ * This software is a free software and there is NO WARRANTY.
/ * No restriction on use. You can use, modify and redistribute it for
/   personal, non-profit or commercial products UNDER YOUR RESPONSIBILITY.
/ * Redistributions of source code must retain the above copyright notice.
/
/------------------------------------------------------------------------*/

#include <stdint.h>
#include <stdbool.h>

#include "diskio_hbios.h"           /* Type definitions */

#if __HBIOS
#include <arch/hbios.h>             /* Device I/O functions */
#else
#error Do you have HBIOS?
#endif

/*--------------------------------------------------------------------------

   Module Private Functions

---------------------------------------------------------------------------*/

#define READ_ATTEMPTS   (3)         /* Attempts to read block. */
#define WRITE_ATTEMPTS  (3)         /* Attempts to write block. */

#define LBA_ADDRESS (0x80000000UL)  /* Use LBA addressing. */

/*------------------------------------------------------------------------

   Public Functions

-------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------*/
/* Initialize Disk Drive                                                 */
/*-----------------------------------------------------------------------*/

#if __SDCC
DSTATUS disk_initialize_fastcall (
    BYTE pdrv               /* Physical drive number */
) __preserves_regs(iyh,iyl) __z88dk_fastcall
#elif __SCCZ80
DSTATUS disk_initialize (
    BYTE pdrv               /* Physical drive number */
) __smallc __z88dk_fastcall
#endif
{
    if ( hbios_a( BF_DIOSTATUS<<8|pdrv) == RES_OK ) return RES_OK;
    return hbios_a( BF_DIORESET<<8|pdrv);
}

/*-----------------------------------------------------------------------*/
/* Get Disk Status                                                       */
/*-----------------------------------------------------------------------*/

#if __SDCC
DSTATUS disk_status_fastcall (
    BYTE pdrv              /* Physical drive number */
) __preserves_regs(iyh,iyl) __z88dk_fastcall
#elif __SCCZ80
DSTATUS disk_status (
    BYTE pdrv              /* Physical drive number */
) __smallc __z88dk_fastcall
#endif
{
    return hbios_a( BF_DIOSTATUS<<8|pdrv);
}

/*-----------------------------------------------------------------------*/
/* Read Sector(s)                                                        */
/*-----------------------------------------------------------------------*/

#if __SDCC
DRESULT disk_read (
    BYTE pdrv,              /* Physical drive number */
    BYTE *buff,             /* Pointer to the data buffer to store read data */
    LBA_t sector,           /* Start sector number (LBA) */
    UINT count              /* Sector count (1..128) */
) __preserves_regs(iyh,iyl)
#elif __SCCZ80
DRESULT disk_read (
    BYTE pdrv,              /* Physical drive number */
    BYTE *buff,             /* Pointer to the data buffer to store read data */
    LBA_t sector,           /* Start sector number (LBA) */
    UINT count              /* Sector count (1..128) */
) __smallc
#endif
{
    uint8_t rattempt = READ_ATTEMPTS;               /* Read attempts */
    uint8_t resp = 0;

    if (count == 0 ) return RES_PARERR;             /* sector count can't be zero */

    do {
        if ( hbios_a_dehl( BF_DIOSEEK<<8|pdrv, sector|LBA_ADDRESS ) == 0) {
            resp = hbios_e_de_hl( BF_DIOREAD<<8|pdrv, count, buff );
            if ( resp == count ) resp = RES_OK;
        }
    } while  ((--rattempt != 0) && (resp != RES_OK) );

    return resp;
}


/*-----------------------------------------------------------------------*/
/* Write Sector(s)                                                       */
/*-----------------------------------------------------------------------*/

#if __SDCC
DRESULT disk_write (
    BYTE pdrv,              /* Physical drive number */
    const BYTE *buff,       /* Pointer to the data to be written */
    LBA_t sector,           /* Start sector number (LBA) */
    UINT count              /* Sector count (1..128) */
) __preserves_regs(iyh,iyl)
#elif __SCCZ80
DRESULT disk_write (
    BYTE pdrv,              /* Physical drive number */
    const BYTE *buff,       /* Pointer to the data to be written */
    LBA_t sector,           /* Start sector number (LBA) */
    UINT count              /* Sector count (1..128) */
) __smallc
#endif
{
    uint8_t wattempt = WRITE_ATTEMPTS;              /* Write attempts */
    uint8_t resp = 0;

    if (count == 0 ) return RES_PARERR;             /* sector count can't be zero */

    do {
        if ( hbios_a_dehl( BF_DIOSEEK<<8|pdrv, sector|LBA_ADDRESS ) == 0) {
            resp = hbios_e_de_hl( BF_DIOWRITE<<8|pdrv, count, buff );
            if ( resp == count ) resp = RES_OK;
        }
    } while  ((--wattempt != 0) && (resp != RES_OK) );

    return resp;
}


/*-----------------------------------------------------------------------*/
/* Miscellaneous Functions                                               */
/*-----------------------------------------------------------------------*/

#if __SDCC
DRESULT disk_ioctl (
    BYTE pdrv,              /* Physical drive number */
    BYTE cmd,               /* Control code */
    void *buff              /* Buffer to send/receive control data */
) __preserves_regs(iyh,iyl)
#elif __SCCZ80
DRESULT disk_ioctl (
    BYTE pdrv,              /* Physical drive number */
    BYTE cmd,               /* Control code */
    void *buff              /* Buffer to send/receive control data */
) __smallc
#endif
{

    DRESULT resp = RES_ERROR;

    switch (cmd) {
        case CTRL_SYNC :            /* Make sure that no pending write process */
            resp = RES_OK;
            break;

        case GET_SECTOR_COUNT :     /* Get number of sectors on the disk (DWORD) */
            *(uint32_t*)buff = hbios( BF_DIOCAP<<8|pdrv );
            resp = RES_OK;
            break;

        case GET_SECTOR_SIZE :      /* Get R/W sector size (uint16_t) */
            *(uint16_t*)buff = 512;
            resp = RES_OK;
            break;

        case GET_BLOCK_SIZE  :      /* Get erase block size */
            *(uint16_t*)buff = 1;
            resp = RES_OK;
            break;

        default:
            resp = RES_PARERR;
            break;
    }

    return resp;
}
