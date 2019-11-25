
#ifndef __LIB_DISKIO_H__
#define __LIB_DISKIO_H__

#include <arch.h>
#include <stdint.h>

/*
 * Disk Status Bits DSTATUS (uint8_t)
 *
 */

#define STA_NOINIT		0x01	/* Drive not initialised */
#define STA_NODISK		0x02	/* No medium in the drive */
#define STA_PROTECT		0x04	/* Write protected */

/*
 * Command codes for disk_ioctrl function
 *
 */

/* Generic command (Used by FatFs) */
#define CTRL_SYNC			0	/* Complete pending write process (needed at _FS_READONLY == 0) */
#define GET_SECTOR_COUNT	1	/* Get media size (needed at _USE_MKFS == 1) */
#define GET_SECTOR_SIZE		2	/* Get sector size (needed at _MAX_SS != _MIN_SS) */
#define GET_BLOCK_SIZE		3	/* Get erase block size (needed at _USE_MKFS == 1) */

/* Generic command (not used by FatFs) */
#define CTRL_POWER			5	/* Get/Set power status */
#define CTRL_LOCK			6	/* Lock/Unlock media removal */
#define CTRL_EJECT			7	/* Eject media */
#define CTRL_FORMAT			8	/* Create physical format on the media */

/* MMC/SDC specific ioctl command */
#define MMC_GET_TYPE		10	/* Get card type */
#define MMC_GET_CSD			11	/* Get CSD */
#define MMC_GET_CID			12	/* Get CID */
#define MMC_GET_OCR			13	/* Get OCR */
#define MMC_GET_SDSTAT		14	/* Get SD status */
#define ISDIO_READ			55	/* Read data form SD iSDIO register */
#define ISDIO_WRITE			56	/* Write data to SD iSDIO register */
#define ISDIO_MRITE			57	/* Masked write data to SD iSDIO register */

/* ATA/CF specific ioctl command */
#define ATA_GET_REV			20	/* Get F/W revision */
#define ATA_GET_MODEL		21	/* Get model name */
#define ATA_GET_SN			22	/* Get serial number */

/* NAND specific ioctl command */
#define NAND_FORMAT			30	/* Create physical format */

/* MMC card type flags (MMC_GET_TYPE) */
#define CT_MMC				0x01		/* MMC ver 3 */
#define CT_SD1				0x02		/* SD ver 1 */
#define CT_SD2				0x04		/* SD ver 2 */
#define CT_SDC				(CT_SD1|CT_SD2)	/* SD */
#define CT_BLOCK			0x08		/* Block addressing */



/* Status of Disk Functions */
typedef BYTE DSTATUS;

/* Results of Disk Functions */
typedef enum {
    RES_OK = 0,	    /* 0: Successful */
    RES_ERROR = 1,  /* 1: R/W Error */
    RES_WRPRT = 2,  /* 2: Write Protected */
    RES_NOTRDY = 3, /* 3: Not Ready */
    RES_PARERR = 4  /* 4: Invalid Parameter */
} DRESULT;

#if __SDCC
//
// IDE DISK COMMANDS
//

DSTATUS disk_initialize_fastcall(BYTE pdrv) __preserves_regs(iyh,iyl) __z88dk_fastcall;
#define disk_initialize(a) disk_initialize_fastcall(a)

DSTATUS disk_status_fastcall(BYTE pdrv) __preserves_regs(iyh,iyl) __z88dk_fastcall;
#define disk_status(a) disk_status_fastcall(a)

DRESULT disk_read(BYTE pdrv,BYTE* buff,DWORD sector,UINT count) __preserves_regs(iyh,iyl);

DRESULT disk_write(BYTE pdrv,const BYTE* buff,DWORD sector,UINT count) __preserves_regs(iyh,iyl);

DRESULT disk_ioctl(BYTE pdrv,BYTE cmd,void* buff) __preserves_regs(iyh,iyl);

//
// CSIO SD COMMANDS
//

extern void sd_clock(uint8_t) __preserves_regs(b,c,d,e,iyh,iyl);
extern void sd_clock_fastcall(uint8_t) __preserves_regs(b,c,d,e,iyh,iyl) __z88dk_fastcall;
#define sd_clock(a) sd_clock_fastcall(a)

extern void sd_cs_lower(void) __preserves_regs(b,c,d,e,h,l,iyh,iyl);
extern void sd_cs_lower_fastcall(void) __preserves_regs(b,c,d,e,h,l,iyh,iyl) __z88dk_fastcall;
#define sd_cs_lower(a) sd_cs_lower_fastcall(a)

extern void sd_cs_raise(void) __preserves_regs(b,c,d,e,h,l,iyh,iyl);
extern void sd_cs_raise_fastcall(void) __preserves_regs(b,c,d,e,h,l,iyh,iyl) __z88dk_fastcall;
#define sd_cs_raise(a) sd_cs_raise_fastcall(a)

extern void sd_write_byte(uint8_t) __preserves_regs(b,c,d,e,iyh,iyl);
extern void sd_write_byte_fastcall(uint8_t) __preserves_regs(b,c,d,e,iyh,iyl) __z88dk_fastcall;
#define sd_write_byte(a) sd_write_byte_fastcall(a)

extern void sd_write_block(const uint8_t *from) __preserves_regs(iyh,iyl);
extern void sd_write_block_fastcall(const uint8_t *from) __preserves_regs(iyh,iyl) __z88dk_fastcall;
#define sd_write_block(a) sd_write_block_fastcall(a)

extern uint8_t sd_read_byte(void) __preserves_regs(b,c,d,e,iyh,iyl);
extern uint8_t sd_read_byte_fastcall(void) __preserves_regs(b,c,d,e,iyh,iyl) __z88dk_fastcall;
#define sd_read_byte(a) sd_read_byte_fastcall(a)

extern void sd_read_block(uint8_t *to) __preserves_regs(iyh,iyl);
extern void sd_read_block_fastcall(uint8_t *to) __preserves_regs(iyh,iyl) __z88dk_fastcall;
#define sd_read_block(a) sd_read_block_fastcall(a)

#elif __SCCZ80

//
// IDE DISK COMMANDS
//

DSTATUS __LIB__ disk_initialize(BYTE pdrv) __smallc __z88dk_fastcall;

DSTATUS __LIB__ disk_status(BYTE pdrv) __smallc __z88dk_fastcall;

DRESULT __LIB__ disk_read_callee(BYTE pdrv,BYTE* buff,DWORD sector,UINT count) __smallc __z88dk_callee;
#define disk_read(a,b,c,d) disk_read_callee(a,b,c,d)

DRESULT __LIB__ disk_write_callee(BYTE pdrv,const BYTE* buff,DWORD sector,UINT count) __smallc __z88dk_callee;
#define disk_write(a,b,c,d) disk_write_callee(a,b,c,d)

DRESULT __LIB__ disk_ioctl_callee(BYTE pdrv,BYTE cmd,void* buff) __smallc __z88dk_callee;
#define disk_ioctl(a,b,c) disk_ioctl_callee(a,b,c)

//
// CSIO SD COMMANDS
//

extern void __LIB__ sd_clock(uint8_t) __smallc __z88dk_fastcall;

extern void __LIB__ sd_cs_lower(void) __smallc __z88dk_fastcall;

extern void __LIB__ sd_cs_raise(void) __smallc __z88dk_fastcall;

extern void __LIB__ sd_write_byte(uint8_t) __smallc __z88dk_fastcall;

extern void __LIB__ sd_write_block(const uint8_t *from) __smallc __z88dk_fastcall;

extern uint8_t __LIB__ sd_read_byte(void) __smallc __z88dk_fastcall;

extern void __LIB__ sd_read_block(uint8_t *to) __smallc __z88dk_fastcall;

#endif

#endif /* !_LIB_DISKIO_H_ */
