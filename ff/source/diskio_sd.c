/*------------------------------------------------------------------------/
/  Foolproof MMCv3/SDv1/SDv2 (in SPI mode) control module
/-------------------------------------------------------------------------/
/
/  Copyright (C) 2019, ChaN, all right reserved.
/
/ * This software is a free software and there is NO WARRANTY.
/ * No restriction on use. You can use, modify and redistribute it for
/   personal, non-profit or commercial products UNDER YOUR RESPONSIBILITY.
/ * Redistributions of source code must retain the above copyright notice.
/
/------------------------------------------------------------------------*/

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#include "diskio.h"

#if __YAZ180
#include <lib/yaz180/ff.h>       /* Declarations of FatFs API */
#elif __SCZ180
#include <lib/scz180/ff.h>       /* Declarations of FatFs API */
#else
#error Do you have FAT?
#endif

/*--------------------------------------------------------------------------

   Module Private Functions

---------------------------------------------------------------------------*/

/* MMC/SD command (SPI mode) */
#define CMD0    (0)             /* GO_IDLE_STATE */
#define CMD1    (1)             /* SEND_OP_COND */
#define ACMD41  (0x80+41)       /* SEND_OP_COND (SDC) */
#define CMD8    (8)             /* SEND_IF_COND */
#define CMD9    (9)             /* SEND_CSD */
#define CMD10   (10)            /* SEND_CID */
#define CMD12   (12)            /* STOP_TRANSMISSION */
#define CMD13   (13)            /* SEND_STATUS */
#define ACMD13  (0x80+13)       /* SD_STATUS (SDC) */
#define CMD16   (16)            /* SET_BLOCKLEN */
#define CMD17   (17)            /* READ_SINGLE_BLOCK */
#define CMD18   (18)            /* READ_MULTIPLE_BLOCK */
#define CMD23   (23)            /* SET_BLOCK_COUNT */
#define ACMD23  (0x80+23)       /* SET_WR_BLK_ERASE_COUNT (SDC) */
#define CMD24   (24)            /* WRITE_BLOCK */
#define CMD25   (25)            /* WRITE_MULTIPLE_BLOCK */
#define CMD32   (32)            /* ERASE_ER_BLK_START */
#define CMD33   (33)            /* ERASE_ER_BLK_END */
#define CMD38   (38)            /* ERASE */
#define CMD55   (55)            /* APP_CMD */
#define CMD58   (58)            /* READ_OCR */

// Return from send_cmd with two bytes to allow for R2 responses.
// R1 responses are contained in the MSB of the word.
// R2 responses consist of R1 followed by the LSB of the word.

#define R2_CARD_LOCKED          0x0001  /** status bit for card locked by user */
#define R2_WRITE_VIOLATION      0x0002  /** status bit for write protect or lock / unlock sequence error */
#define R2_UNKNOWN_ERROR        0x0004  /** status bit for generalised fubar */
#define R2_CC_ERROR             0x0008  /** status bit for internal card controller error */
#define R2_CARD_ECC_FAIL        0x0010  /** status bit for internal ECC application failed, therefore invalid data */
#define R2_WRITE_PROTECT        0x0020  /** status bit for attempted write of write protected block */
#define R2_ERASE_PARAM_ERROR    0x0040  /** status bit for invalid selection for erase, sectors, or groups */
#define R2_RANGE_ERROR          0x0080  /** status bit for out of range or attempted CSD overwrite error */


#define R1_READY_STATE          0x0000  /** status for card in the ready state */

#define R1_IDLE_STATE           0x0100  /** status for card in the idle state */
#define R1_ERASE_RESET          0x0200  /** status for erase sequence cleared before executing, out of sequence command */
#define R1_ILLEGAL_COMMAND      0x0400  /** status bit for illegal command */
#define R1_CRC_ERROR            0x0800  /** status bit for CRC check for last command failed */
#define R1_ERASE_SEQ_ERROR      0x1000  /** status bit for erase command sequence error */
#define R1_ADDRESS_ERROR        0x2000  /** status bit for misaligned address not matching block length */
#define R1_PARAMETER_ERROR      0x4000  /** status bit for parameter error in command's argument outside this card range */

#define WRITE_MULTIPLE_TOKEN    0xFC    /** start data token for write multiple blocks*/
#define STOP_TRAN_TOKEN         0xFD    /** stop token for write multiple blocks*/

#define DATA_START_BLOCK        0xFE    /** start data token for read or write single block*/

#define DATA_RES_ACCEPTED       0x05    /** write data accepted token */
#define DATA_RES_CRC_ERROR      0x0B    /** write data failed CRC error token */
#define DATA_RES_WRITE_ERROR    0x0D    /** write data failed write error */

#define DATA_RES_MASK           0x1F    /** mask for data response tokens after a write block operation */
#define R1_RES_MASK             0x7F    /** mask for R1 result mask */

#define HIGH_CAPACITY_SUPPORT   (0x40000000UL)    /* High Capacity Support argument used in ACMD41 during initialisation. */
#define CRC_ON                  (0x00000001UL)
#define CRC_OFF                 (0x00000000UL)
#define CRC_INIT                (0x0000)

/*-----------------------------------------------------------------------*/
/* Global variables                                                      */
/*-----------------------------------------------------------------------*/

static DSTATUS Stat = STA_NOINIT;   /* Disk status */

static BYTE CardType;               /* b0:MMC, b1:SDv1, b2:SDv2, b3:Block addressing */

/*---------------------------------------------------------------------- */
/*   Private Functions                                                   */
/*-----------------------------------------------------------------------*/

static BYTE wait_ready (uint8_t want_ff);
static void deselect (void);
static BYTE select (void);
static BYTE sd_read_data (BYTE *buff, BYTE length);
static BYTE sd_read_sector (BYTE *buff);
static BYTE sd_write_sector (const BYTE *buff, BYTE token);
static WORD send_cmd (BYTE cmd, DWORD arg);

/*-----------------------------------------------------------------------*/
/* Wait for card ready                                                   */
/*-----------------------------------------------------------------------*/

static
BYTE wait_ready (uint8_t want_ff)    /* 1:OK, 0:Timeout */
{
    uint8_t b;

    while(true){
        b = sd_read_byte();
        if(want_ff){
            if(b == 0xFF) break;
        }else{
            if(b != 0xFF) break;
        }
    }
    return b;
}

/*-----------------------------------------------------------------------*/
/* Deselect the card and release SPI bus                                 */
/*-----------------------------------------------------------------------*/

static
void deselect (void)
{
    sd_cs_raise();
    sd_read_byte();
}

/*-----------------------------------------------------------------------*/
/* Select the card and wait for ready                                    */
/*-----------------------------------------------------------------------*/

static
BYTE select (void)    /* 1:OK, 0:Timeout */
{
    sd_cs_lower();                 /* Set CS# low */
    sd_read_byte();                /* Dummy clock (force DO enabled) */
    if (wait_ready(true)) return 1; /* Wait for card ready */

    deselect();
    return 0;                       /* Failed */
}

/*-----------------------------------------------------------------------*/
/* Receive a data block from the card                                    */
/*-----------------------------------------------------------------------*/

static
BYTE sd_read_data (     /* 1:OK, 0:Failed */
    BYTE *buff,          /* Data buffer to store received data */
    BYTE length         /* Length of data to read */
)
{
    BYTE token;
    BYTE attempt;

    for (attempt=0; attempt<100; ++attempt) {   /* Wait for data packet */
        token = sd_read_byte();
        if (token != 0xFF) break;
    }
    if (token != DATA_START_BLOCK) return 0;    /* If not valid data token, return with error */

    do {
       *buff++ = sd_read_byte();                /* Receive the data block into buffer */
    } while (--length);

    sd_read_byte();                             /* Discard CRC */
    sd_read_byte();

    return 1;                                   /* Return with success */
}

/*-----------------------------------------------------------------------*/
/* Receive a sector from the card                                        */
/*-----------------------------------------------------------------------*/

static
BYTE sd_read_sector (   /* 1:OK, 0:Failed */
    BYTE *buff           /* Data buffer to store received data */
)
{
    BYTE token;
    BYTE attempt;

    for (attempt=0; attempt<100; ++attempt) {   /* Wait for data packet */
        token = sd_read_byte();
        if (token != 0xFF) break;
    }
    if (token != DATA_START_BLOCK) return 0;    /* If not valid data token, return with error */

    sd_read_block(buff);                        /* Receive the data block into buffer */
    sd_read_byte();                             /* Discard CRC */
    sd_read_byte();

    return 1;                                   /* Return with success */
}

/*-----------------------------------------------------------------------*/
/* Send a sector to the card                                             */
/*-----------------------------------------------------------------------*/

static
BYTE sd_write_sector (   /* 1:OK, 0:Failed */
    const BYTE *buff,    /* 512 byte data block to be transmitted */
    BYTE token          /* Data/Stop token */
)
{
    if (!wait_ready(true))
        return 0;

    sd_write_byte(token);           /* Xmit a token */
    if (token != STOP_TRAN_TOKEN) { /* Is it data token? */
        sd_write_block(buff);       /* Xmit the 512 byte data block to MMC */
        sd_write_byte(0xFF);        /* Xmit dummy CRC (0xFF,0xFF) */
        sd_write_byte(0xFF);

        if ((sd_read_byte() & DATA_RES_MASK) != DATA_RES_ACCEPTED)
            return 0;               /* If not accepted, return with error */
    }
    return 1;
}

/*-----------------------------------------------------------------------*/
/* Send a command packet to the card                                     */
/*-----------------------------------------------------------------------*/

static
WORD send_cmd (         /* Returns command response (bit7==1:Send failed)*/
    BYTE cmd,           /* Command byte */
    DWORD arg           /* Argument */
)
{
    BYTE n;
    WORD d;
    BYTE *p;

    if (cmd & 0x80) {    /* ACMD<n> is the command sequense of CMD55-CMD<n> */
        cmd &= 0x7F;
        n = send_cmd(CMD55, 0);
        if (n > 1) return n;
    }

    /* Select the card and wait for ready except to stop multiple block read */
    if (cmd != CMD12) {
        /* Select the card and wait for ready */
        deselect(); /* raise CS, then sends 8 clocks (some cards require this) */
        sd_cs_lower();

        if(cmd != CMD0 && wait_ready(true) != 0xFF)
            return 0xFF;
    }

    /* Send command packet */
    sd_write_byte(cmd);                         /* Start + Command index */

    /* sdcc sadly unable to figure this out for itself yet */
    p = (BYTE *)&arg+3;
    sd_write_byte(*p);                          /* Argument[31..24] */
    --p;
    sd_write_byte(*p);                          /* Argument[23..16] */
    --p;
    sd_write_byte(*p);                          /* Argument[15..8] */
    --p;
    sd_write_byte(*p);                          /* Argument[7..0] */

    /* there's only a few commands (in native mode) that need correct CRCs */
    n = 0x01;                                   /* Dummy CRC + Stop */
    if (cmd == CMD0) n = 0x95;                  /* Valid CRC for CMD0(0) */
    if (cmd == CMD8) n = 0x87;                  /* Valid CRC for CMD8(0x1AA) */
    sd_write_byte(n);

    /* Receive command response */
    if (cmd == CMD12) d = sd_read_byte();       /* Skip a stuff byte when stop reading */
    n = 10;                                     /* Wait for a valid response in timeout of 10 attempts */
    do
        d = sd_read_byte();
    while ((d & 0x80) && --n);
    
    d <<= 8;                                    /* shift first part of response */

    if ( cmd == CMD12 || cmd == CMD38 ){        /* Skip busy signal for command CMD12 (stop transmission) and CMD38 (erase) */
        n = 0;
        while ( (--n != 0) && (sd_read_byte() == 0x00) ); /* Wait while SD busy (0x00 signal) */
    }

    if (cmd == CMD13 || cmd == ACMD13)          /* Capture R2 response second byte from CMD13 (or ACMD13),*/
        d |= sd_read_byte();                    /* collect a R2 second byte response*/

    return d;   /* Return with the R1 (and R2) response value in uint16 (two bytes) */
}

/*------------------------------------------------------------------------

   Public Functions

-------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------*/
/* Initialize Disk Drive                                                 */
/*-----------------------------------------------------------------------*/

#if __SDCC
DSTATUS disk_initialize_fastcall (
    BYTE pdrv               /* Physical drive number (0) */
) __preserves_regs(b,c,d,e,iyh,iyl) __z88dk_fastcall
#elif __SCCZ80
DSTATUS disk_initialize (
    BYTE pdrv               /* Physical drive number (0) */
)
#endif
{
    BYTE n, ty, cmd, buff[4];
    UINT tmr;
    DSTATUS s;
    BYTE *ptr;

    ptr = buff;

    if (pdrv) return RES_NOTRDY;
    
    sd_clock(__IO_CNTR_SS_DIV_160);             /* Slow clock to between 100kHz and 400kHz (115kHz to 230kHz) */

    for (n = 100; n; n--) sd_write_byte(0xFF);  /* Apply 800 dummy clocks and the card gets ready to receive command */

    ty = 0;
    while (send_cmd(CMD0, 0) != R1_IDLE_STATE && --n);  /* Don't give up easily trying to get to idle state */

    if (send_cmd(CMD0, 0) == R1_IDLE_STATE) {           /* Enter Idle state */
        if (send_cmd(CMD8, 0x1AA) == R1_IDLE_STATE) {   /* SDv2? */
            *ptr++ =sd_read_byte();                     /* Get trailing return value of R7 resp */
            *ptr++ =sd_read_byte();
            *ptr++ =sd_read_byte();
            *ptr   =sd_read_byte();

            if (buff[2] == 0x01 && buff[3] == 0xAA) {   /* The card can work at vdd range of 2.7-3.6V */
                for (tmr = 1000; tmr; tmr--) {          /* Wait for leaving idle state (ACMD41 with HCS bit) */
                    if (send_cmd(ACMD41, HIGH_CAPACITY_SUPPORT) == 0) break;
//TODO                 dly_us(1000);
                }
                if (tmr && send_cmd(CMD58, 0) == 0) {   /* Check CCS bit in the OCR */
                    *ptr++ =sd_read_byte();
                    *ptr++ =sd_read_byte();
                    *ptr++ =sd_read_byte();
                    *ptr   =sd_read_byte();

                    ty = (buff[0] & 0x40) ? CT_SD2 | CT_BLOCK : CT_SD2;    /* SDv2 */
                }
            }
        } else {                            /* SDv1 or MMCv3 */
            if (send_cmd(ACMD41, 0) <= 1)     {
                ty = CT_SD1; cmd = ACMD41;    /* SDv1 */
            } else {
                ty = CT_MMC; cmd = CMD1;    /* MMCv3 */
            }
            for (tmr = 0; tmr = 0; ++tmr) {            /* Wait for leaving idle state */
                if (send_cmd(cmd, 0) == 0) break;
            }
            if (!tmr || send_cmd(CMD16, 512) != 0)    /* Set R/W block length to 512 */
                ty = 0;
        }
    }
    CardType = ty;
    s = ty ? 0 : STA_NOINIT;
    Stat = s;

    sd_clock(__IO_CNTR_SS_DIV_20);      /* Maximum clock is Phi/20 */
    sd_write_byte(0xFF);                /* Give SD Card 8 Clocks to complete command. */
    deselect();

    return s;
}

/*-----------------------------------------------------------------------*/
/* Get Disk Status                                                       */
/*-----------------------------------------------------------------------*/

#if __SDCC
DSTATUS disk_status_fastcall (
    BYTE pdrv               /* Drive number (always 0) */
) __preserves_regs(d,e,iyh,iyl) __z88dk_fastcall
#elif __SCCZ80
DSTATUS disk_status_fastcall (
    BYTE pdrv               /* Drive number (always 0) */
) __smallc __z88dk_fastcall
#endif
{
    if (pdrv) return STA_NOINIT;

    return Stat;
}

/*-----------------------------------------------------------------------*/
/* Read Sector(s)                                                        */
/*-----------------------------------------------------------------------*/

#if __SDCC
DRESULT disk_read(
    BYTE pdrv,               /* Physical drive number (0) */
    BYTE *buff,              /* Pointer to the data buffer to store read data */
    LBA_t sector,           /* Start sector number (LBA) */
    UINT count              /* Sector count (1..128) */
) __preserves_regs(iyh,iyl)
#elif __SCCZ80
DRESULT disk_read (
    BYTE pdrv,               /* Physical drive number (0) */
    BYTE *buff,              /* Pointer to the data buffer to store read data */
    LBA_t sector,           /* Start sector number (LBA) */
    UINT count              /* Sector count (1..128) */
)
#endif
{
    BYTE cmd;
    DWORD sect = (DWORD)sector;


    if (disk_status(pdrv) & STA_NOINIT) return RES_NOTRDY;
    if (!(CardType & CT_BLOCK)) sect *= 512;    /* Convert LBA to byte address if needed */

    cmd = count > 1 ? CMD18 : CMD17;            /*  READ_MULTIPLE_BLOCK : READ_SINGLE_BLOCK */
    if (send_cmd(cmd, sect) == 0) {
        do {
            if (!sd_read_sector(buff)) break;
            buff += 512;
        } while (--count);
        if (cmd == CMD18) send_cmd(CMD12, 0);    /* STOP_TRANSMISSION */
    }
    deselect();

    return count ? RES_ERROR : RES_OK;
}



/*-----------------------------------------------------------------------*/
/* Write Sector(s)                                                       */
/*-----------------------------------------------------------------------*/

#if __SDCC
DRESULT disk_write (
    BYTE pdrv,              /* Physical drive number (0) */
    const BYTE *buff,       /* Pointer to the data to be written */
    LBA_t sector,           /* Start sector number (LBA) */
    UINT count              /* Sector count (1..128) */
) __preserves_regs(iyh,iyl)
#elif __SCCZ80
DRESULT disk_write_callee (
    BYTE pdrv,              /* Physical drive number (0) */
    const BYTE *buff,       /* Pointer to the data to be written */
    LBA_t sector,           /* Start sector number (LBA) */
    UINT count              /* Sector count (1..128) */
) __smallc __z88dk_callee
#endif
{
    DWORD sect = (DWORD)sector;

    if (disk_status(pdrv) & STA_NOINIT) return RES_NOTRDY;
    if (!(CardType & CT_BLOCK)) sect *= 512;    /* Convert LBA to byte address if needed */

    if (count == 1) {    /* Single block write */
        if ((send_cmd(CMD24, sect) == 0)    /* WRITE_BLOCK */
            && sd_write_sector(buff, 0xFE))
            count = 0;
    }
    else {                /* Multiple block write */
        if (CardType & CT_SDC) send_cmd(ACMD23, count);
        if (send_cmd(CMD25, sect) == 0) {    /* WRITE_MULTIPLE_BLOCK */
            do {
                if (!sd_write_sector(buff, WRITE_MULTIPLE_TOKEN)) break;
                buff += 512;
            } while (--count);
            if (!sd_write_sector(0, STOP_TRAN_TOKEN))   /* STOP_TRAN token */
                count = 1;
        }
    }
    deselect();

    return count ? RES_ERROR : RES_OK;
}


/*-----------------------------------------------------------------------*/
/* Miscellaneous Functions                                               */
/*-----------------------------------------------------------------------*/

#if __SDCC
DRESULT disk_ioctl (
    BYTE pdrv,              /* Physical drive number (0) */
    BYTE cmd,               /* Control code */
    void *buff              /* Buffer to send/receive control data */
) __preserves_regs(d,e,iyh,iyl)
#elif __SCCZ80
DRESULT disk_ioctl_callee (
    BYTE pdrv,              /* Physical drive number (0) */
    BYTE cmd,               /* Control code */
    void *buff              /* Buffer to send/receive control data */
) __smallc __z88dk_callee
#endif
{
    DRESULT resp;
    DWORD cs;
    BYTE n, csd[16];

    BYTE *ptr;          /*  8 bit integers for normal addresses */
    uint32_t *erasePtr; /* 32 bit integers for the erase sector (or byte) addresses */

    ptr = (BYTE *)buff;
    erasePtr = (uint32_t *)buff;

    if (disk_status(pdrv) & STA_NOINIT) return RES_NOTRDY;  /* Check if card is in the socket */

    resp = RES_ERROR;

    switch (cmd) {
        case CTRL_SYNC :        /* Make sure that no pending write process */
            if (select()) resp = RES_OK;
            break;

        case GET_SECTOR_COUNT :    /* Get number of sectors on the disk (DWORD) */
            if ((send_cmd(CMD9, 0) == R1_READY_STATE) && sd_read_data(csd, 16)) {
                if ((csd[0] >> 6) == 1) {    /* SDC ver 2.00 */
                    cs = csd[9] + ((WORD)csd[8] << 8) + ((DWORD)(csd[7] & 63) << 16) + 1;
                    *(LBA_t*)buff = cs << 10;
                } else {                    /* SDC ver 1.XX or MMC */
                    n = (csd[5] & 15) + ((csd[10] & 128) >> 7) + ((csd[9] & 3) << 1) + 2;
                    cs = (csd[8] >> 6) + ((WORD)csd[7] << 2) + ((WORD)(csd[6] & 3) << 10) + 1;
                    *(LBA_t*)buff = cs << (n - 9);
                }
                resp = RES_OK;
            }
            break;

        case GET_SECTOR_SIZE :    /* Get R/W sector size (uint16_t) */
            *(uint16_t*)buff = 512;
            resp = RES_OK;
            break;

        case GET_BLOCK_SIZE :    /* Get erase block size in unit of sector (DWORD) */
            if (CardType & CT_SD2) {    /* SDv2? */
                if ((send_cmd(ACMD13, 0) == R1_READY_STATE) && sd_read_data(csd, 16))
                {                /* Read partial block */
                    for (n = 64 - 16; n; --n) sd_read_byte();       /* Purge trailing data */
                    *(uint32_t*)buff = 16UL << (csd[10] >> 4);
                    resp = RES_OK;
                }
            } else {                    /* SDv1 or MMCv3 */
                if ((send_cmd(CMD9, 0) == R1_READY_STATE) && sd_read_data(csd, 16))
                {    /* Read CSD */
                    if (CardType & CT_SD1) {    /* SDv1 */
                        *(uint32_t*)buff = (((csd[10] & 63) << 1) + ((uint16_t)(csd[11] & 128) >> 7) + 1) << ((csd[13] >> 6) - 1);
                    } else {                    /* MMCv3 */
                        *(uint32_t*)buff = ((uint16_t)((csd[10] & 124) >> 2) + 1) * (((csd[11] & 3) << 3) + ((csd[11] & 224) >> 5) + 1);
                    }
                    resp = RES_OK;
                }
            }
            break;

        case MMC_GET_TYPE :        /* Get card type flags (1 byte) */
            *ptr = CardType;
            resp = RES_OK;
            break;

        case MMC_GET_CSD :        /* Receive CSD as a data block (16 bytes) */
            if ((send_cmd(CMD9, 0) == R1_READY_STATE) && sd_read_data(ptr, 16))     /* READ_CSD */
                resp = RES_OK;
            break;

        case MMC_GET_CID :        /* Receive CID as a data block (16 bytes) */
            if ((send_cmd(CMD10, 0) == R1_READY_STATE) && sd_read_data(ptr, 16))    /* READ_CID */
                resp = RES_OK;
            break;

        case MMC_GET_OCR :        /* Receive OCR as an R3 response (4 bytes) */
            if (send_cmd(CMD58, 0) == R1_READY_STATE) {    /* READ_OCR */
                for (n = 4; n; --n) *ptr++ = sd_read_byte();
                resp = RES_OK;
            }
            break;

        case MMC_GET_SDSTAT :    /* Receive SD status as a data block (64 bytes) */
            if ((send_cmd(ACMD13, 0) == R1_READY_STATE) && sd_read_data(ptr, 64))   /* SD_STATUS */
                resp = RES_OK;
            break;

        default:
            resp = RES_PARERR;
            break;
    }
    sd_read_byte();            /* Give SD Card 8 Clocks to complete command. */
    deselect();

    return resp;
}
