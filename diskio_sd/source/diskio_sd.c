/*------------------------------------------------------------------------/
/  MMCv3/SDv1/SDv2 (in SPI mode) control module
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

#include "diskio_sd.h"          /* Type definitions */

#if __SCZ180
#include <arch/scz180.h>        /* Device I/O functions */
#include <arch/hbios.h>         /* HBIOS I/O functions for timers */
#else
#error I do not have a SD card driver, sorry!
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

#define DATA_RES_ACCEPTED       0x05    /** write data & read data accepted token */
#define DATA_RES_CRC_ERROR      0x0B    /** write data failed CRC error token */
#define DATA_RES_WRITE_ERROR    0x0D    /** write data failed write error */

#define DATA_RES_MASK           0x1F    /** mask for data response tokens after a write block operation */
#define R1_RES_MASK             0x7F    /** mask for R1 result mask */

#define HIGH_CAPACITY_SUPPORT   (0x40000000UL)    /* High Capacity Support argument used in ACMD41 during initialisation. */
#define CRC_ON                  (0x00000001UL)
#define CRC_OFF                 (0x00000000UL)
#define CRC_INIT                (0x0000)

#define READ_ATTEMPTS           (10)    /* Attempts to read block. */
#define WRITE_ATTEMPTS          (10)    /* Attempts to write block. */

/*-----------------------------------------------------------------------*/
/* Static variables                                                      */
/*-----------------------------------------------------------------------*/

static volatile DSTATUS Stat;   /* Disk status */

static BYTE CardType;   /* b0:MMC, b1:SDv1, b2:SDv2, b3:Block addressing */

/*---------------------------------------------------------------------- */
/* Private Functions                                                     */
/*-----------------------------------------------------------------------*/

#if __SDCC
static inline void deselect (void);
static inline void select (BYTE pdrv);
#elif __SCCZ80
static void deselect (void);
static void select (BYTE pdrv);
#endif

static BYTE wait_ready (uint8_t want_ff);
static BYTE read_data (BYTE *buff, BYTE length);
static BYTE read_sector (BYTE *buff);
static BYTE write_sector (const BYTE *buff, BYTE token);
static WORD send_cmd (BYTE cmd, DWORD arg);

/*-----------------------------------------------------------------------*/
/* Deselect the card and release SPI bus                                 */
/*-----------------------------------------------------------------------*/

#if __SDCC
static
inline void deselect (void)
#elif __SCCZ80
static
void deselect (void)
#endif
{
    sd_cs_raise();                              /* Set CS# high */
    sd_write_byte(0xFF);
}

/*-----------------------------------------------------------------------*/
/* Select the card and wait for ready                                    */
/*-----------------------------------------------------------------------*/

#if __SDCC
static
inline void select (BYTE pdrv)
#elif __SCCZ80
static
void select (BYTE pdrv)
#endif
{
    sd_cs_lower(pdrv);                          /* Set CS# low */
}

/*-----------------------------------------------------------------------*/
/* Wait for card ready                                                   */
/*-----------------------------------------------------------------------*/

static
BYTE wait_ready (uint8_t want_ff)
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
/* Receive a data block from the card                                    */
/*-----------------------------------------------------------------------*/

static
BYTE read_data (        /* DATA_RES_ACCEPTED:OK, 0:Failed */
    BYTE *buff,         /* Data buffer to store received data */
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

    return DATA_RES_ACCEPTED;                   /* Return with success */
}

/*-----------------------------------------------------------------------*/
/* Receive a sector from the card                                        */
/*-----------------------------------------------------------------------*/

static
BYTE read_sector (      /* DATA_RES_ACCEPTED:OK, 0:Failed */
    BYTE *buff          /* Data buffer to store received data */
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

    return DATA_RES_ACCEPTED;                   /* Return with success */
}

/*-----------------------------------------------------------------------*/
/* Send a sector to the card                                             */
/*-----------------------------------------------------------------------*/

static
BYTE write_sector (     /* DATA_RES_ACCEPTED:OK, 0:Failed */
    const BYTE *buff,   /* 512 byte data block to be transmitted */
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

        return (sd_read_byte() & DATA_RES_MASK);    /* Receive data response. If accepted then xxx00101 */
    }
    return DATA_RES_ACCEPTED;       /* STOP_TRAN_TOKEN received so return with success */
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
    WORD resp;
    BYTE *ptr;
    BYTE n;

    if (cmd != CMD0 && cmd != CMD12) {          /* Long wait for ready except to initialise or to stop multiple block read */
        wait_ready(true);                       /* Wait while SD busy (0x00 signal) */
    }

    if (cmd & 0x80) {                           /* ACMD<n> is the command sequence of CMD55 + CMD<n> */

        /* Send special CMD55 as precursor to Alternate Command packet */
        sd_write_byte(0x40 | CMD55);            /* Start Bit + Command index */
        sd_write_byte(0x00);                    /* Stuff Bits: Null argument for CMD55 */
        sd_write_byte(0x00);
        sd_write_byte(0x00);
        sd_write_byte(0x00);
        sd_write_byte(0xA5 | 0x01);             /* Dummy CRC + Stop */

        sd_read_byte();                         /* Skip a stuff byte to avoid MISO pull-up problem */   

        /* Receive command response */          /* Wait for a valid response within 8 attempts */     
        for ( uint8_t n = 8; ((resp = sd_read_byte()) & 0x80) && n; --n) ;

        if (resp > 0x01) {
            deselect();                         /* raise CS */
            return (resp << 8);                 /* something bad happened so we didn't see R1 = 0x01 */
        }

        sd_write_byte(0xFF);                   /* Insert a stuff byte. Needed allow precursor command processing */
    }

    /* Send command byte */
    sd_write_byte((0x40|cmd)&0x7F);             /* Start + Command index */

    /* Send command arguments */
    ptr = (BYTE *)&arg+3;                       /* Unusual grammar to optimise compiled code */
    sd_write_byte(*ptr--);                      /* Argument[31..24] */
    sd_write_byte(*ptr--);                      /* Argument[23..16] */
    sd_write_byte(*ptr--);                      /* Argument[15..8] */
    sd_write_byte(*ptr);                        /* Argument[7..0] */

    /* there's only a few commands (when still in native mode) that need correct CRCs */
    n = 0xA5 | 0x01;                            /* Dummy CRC + Stop */
    if (cmd == CMD0) n = 0x95;                  /* Valid CRC for CMD0(0) */
    if (cmd == CMD8) n = 0x87;                  /* Valid CRC for CMD8(0x1AA) */
    sd_write_byte(n);

    sd_read_byte();                             /* Skip a stuff byte, and avoid MISO pull-up problem */

    /* Receive command response */              /* Wait for a valid response within 8 attempts */     
    for ( uint8_t n = 8; ((resp = sd_read_byte()) & 0x80) && n; --n) ;

    resp <<= 8;                                 /* shift first part of response word up */

    if ( cmd == CMD12 || cmd == CMD38 ) {       /* Skip busy signal for command CMD12 (stop transmission) and CMD38 (erase) */
        wait_ready(true);                       /* Wait while SD busy (0x00 signal) */
    }

    if ( cmd == CMD13 || cmd == ACMD13) {       /* Capture R2 response second byte from CMD13 (or ACMD13),*/
        resp |= sd_read_byte();                 /* collect a R2 second byte response*/
    }

    return resp;                                /* Return with the R1 (and R2) response value */
                                                /* in uint16 (two bytes) */
}

/*------------------------------------------------------------------------

   Public Functions

-------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------*/
/* Initialize Disk Drive                                                 */
/*-----------------------------------------------------------------------*/

#if __SDCC
DSTATUS disk_initialize_fastcall (
    BYTE pdrv               /* Physical drive number (0 or 1) */
) __preserves_regs(iyh,iyl) __z88dk_fastcall
#elif __SCCZ80
DSTATUS disk_initialize (
    BYTE pdrv               /* Physical drive number (0 or 1) */
) __smallc __z88dk_fastcall
#endif
{
    BYTE buff[4];
    WORD resp;
    BYTE *ptr;
    uint32_t endTime, currentTime;

    CardType = 0;                                           /* Set invalid SD card type, initially */
    Stat = STA_NOINIT;                                      /* Set uninitialised, initially */

    if ( pdrv > 1) return STA_NOINIT;                       /* only drive 0 and 1 supported */

    sd_clock(__IO_CNTR_SS_DIV_160);                         /* Slow clock to between 100kHz and 400kHz (115kHz to 230kHz) */

    for ( uint8_t n = 10; n; --n ) sd_write_byte(0xFF);     /* 80 (minimum 74) dummy clocks on SPI bus; without SD card selected. */

    select(pdrv);                                           /* MISO will be low on CMD0 initially */

    endTime = hbios( BF_SYSGET<<8|BF_SYSGET_TIMER );
    endTime += 200;                                         /* Try for 4 seconds; 4000ms/20ms ticks */
    do{                                                     /* Don't give up easily trying to get to idle state, try for 4 seconds */
        if ( (resp = send_cmd(CMD0, 0)) == R1_IDLE_STATE ) break;

        sd_write_byte(STOP_TRAN_TOKEN);                     /* stop interrupted multi-block write */
        for ( uint16_t i = 0; i < 520; ++i ) {              /* finish interrupted block transfer */
            sd_read_byte();
        }

        currentTime = hbios( BF_SYSGET<<8|BF_SYSGET_TIMER );
    } while ( endTime > currentTime );

    if ( resp == R1_IDLE_STATE) {                           /* Entered Idle state */
        if ((resp = send_cmd(CMD8, 0x1AA)) == R1_IDLE_STATE ) {     /* SDv2? */
            ptr = buff;                                     /* Unusual grammar to optimise compiled code */
            *ptr++ =sd_read_byte();                         /* Get trailing return value of R7 resp */
            *ptr++ =sd_read_byte();
            *ptr++ =sd_read_byte();
            *ptr   =sd_read_byte();

            if ( buff[2] == 0x01 && buff[3] == 0xAA ) {     /* The card can work at vdd range of 2.7-3.6V */
                                                            /* And the echo bits were correctly replied */

                endTime = hbios( BF_SYSGET<<8|BF_SYSGET_TIMER );
                endTime += 50;                              /* Initialise for 1 second; 1000ms/20ms ticks */
                do {                                        /* Wait for leaving idle state (ACMD41 with HCS bit) */
                    if ((resp = send_cmd(ACMD41, HIGH_CAPACITY_SUPPORT)) == R1_READY_STATE ) break; /* Initialisation timeout of 1 second (High Capacity Support) Bit 30*/
                    sd_write_byte(0xFF);                    /* Give SD Card 8 Clocks to complete command, before trying again. */

                    currentTime = hbios( BF_SYSGET<<8|BF_SYSGET_TIMER );
                } while ( endTime > currentTime );

                if (resp == R1_READY_STATE) {
                    if ( send_cmd(CMD58, 0) ==  R1_READY_STATE) {   /* Check CCS bit in the OCR */
                        ptr = buff;                         /* Unusual grammar to optimise compiled code */
                        *ptr++ =sd_read_byte();             /* Get trailing return value of R3 resp */
                        *ptr++ =sd_read_byte();
                        *ptr++ =sd_read_byte();
                        *ptr   =sd_read_byte();
                         CardType = (buff[0] & 0x40) ? CT_SD2|CT_BLOCK : CT_SD2;    /* SDv2 */                   
                    }
                }
            }
        } else if ( resp == (R1_ILLEGAL_COMMAND | R1_IDLE_STATE) ) {                /* SDv1 or MMCv3 */

            endTime = hbios( BF_SYSGET<<8|BF_SYSGET_TIMER );
            endTime += 50;                                  /* Initialise for 1 second; 1000ms/20ms ticks */
            do {
                if((resp = send_cmd(ACMD41, 0x00)) == R1_READY_STATE ) break;
                sd_write_byte(0xFF);                        /* Give SD Card 8 Clocks to complete command. */

                currentTime = hbios( BF_SYSGET<<8|BF_SYSGET_TIMER );
            } while ( endTime > currentTime );

            if (resp == R1_READY_STATE) {                   /* SDv1 */
                CardType = CT_SD1;

            } else {                                        /* MMCv3 ?? */

                endTime = hbios( BF_SYSGET<<8|BF_SYSGET_TIMER );
                endTime += 50;                              /* Initialise for 1 second; 1000ms/20ms ticks */
                do {
                    if ((resp = send_cmd(CMD1, 0x00)) == R1_IDLE_STATE ) break; /* initialise for 1 second */
                    sd_write_byte(0xFF);                    /* Give SD Card 8 Clocks to complete command. */

                    currentTime = hbios( BF_SYSGET<<8|BF_SYSGET_TIMER );
                } while ( endTime > currentTime );

                if (resp == R1_IDLE_STATE)                  /* MMCv3 */
                    CardType = CT_MMC;
                else
                    CardType = 0;
            }
        } else {                                            /* some kind of unknown error in initialisation */
            CardType = 0;
        }

        if ( (CardType != 0) && (CardType != CT_SD2|CT_BLOCK) )     /* If it is NOT a Block Address SD Version 2 device */
            if ( send_cmd(CMD16, 512) != R1_READY_STATE )   /* Try to set R/W block length to 512 */
                CardType = 0;
    }

    sd_clock(__IO_CNTR_SS_DIV_20);                          /* Maximum clock is Phi/20 */
    deselect();                                             /* Give SD Card 8 Clocks to complete command. */

    if (CardType) {                                         /* Initialisation succeeded */
        Stat &= ~STA_NOINIT;                                /* Clear STA_NOINIT */
    }

    return Stat;
}

/*-----------------------------------------------------------------------*/
/* Get Disk Status                                                       */
/*-----------------------------------------------------------------------*/

#if __SDCC
DSTATUS disk_status_fastcall (
    BYTE pdrv              /* Physical drive number (0 or 1) */
) __preserves_regs(iyh,iyl) __z88dk_fastcall
#elif __SCCZ80
DSTATUS disk_status (
    BYTE pdrv              /* Physical drive number (0 or 1) */
) __smallc __z88dk_fastcall
#endif
{
    if (pdrv == 0 || pdrv == 1)
        return Stat;
    else
        return STA_NOINIT;
}

/*-----------------------------------------------------------------------*/
/* Read Sector(s)                                                        */
/*-----------------------------------------------------------------------*/

#if __SDCC
DRESULT disk_read (
    BYTE pdrv,              /* Physical drive number (0 or 1) */
    BYTE *buff,             /* Pointer to the data buffer to store read data */
    LBA_t sector,           /* Start sector number (LBA) */
    UINT count              /* Sector count (1..128) */
) __preserves_regs(iyh,iyl)
#elif __SCCZ80
DRESULT disk_read (
    BYTE pdrv,              /* Physical drive number (0 or 1) */
    BYTE *buff,             /* Pointer to the data buffer to store read data */
    LBA_t sector,           /* Start sector number (LBA) */
    UINT count              /* Sector count (1..128) */
) __smallc
#endif
{
    uint8_t rattempt = READ_ATTEMPTS;               /* Read attempts */
    uint8_t resp = 0;
    BYTE cmd;

    if ( pdrv > 1 || !count) return RES_PARERR;     /* only drive 0 and 1 supported, and sector count can't be zero */
    if (Stat & STA_NOINIT) return RES_NOTRDY;       /* drive must be initialised */

    do {
        select(pdrv);

        if (!(CardType & CT_BLOCK)) sector *= 512;  /* Convert LBA to byte address if needed */

        cmd = count > 1 ? CMD18 : CMD17;            /*  READ_MULTIPLE_BLOCK : READ_SINGLE_BLOCK */

        if (send_cmd(cmd, sector) == R1_READY_STATE) {
            do {
                if ((resp = read_sector(buff) ) != DATA_RES_ACCEPTED) break;
                buff += 512;
            } while (--count);
            if (cmd == CMD18) send_cmd(CMD12, 0x00);/* STOP_TRANSMISSION */
        }

        deselect();                                 /* Give SD Card 8 Clocks to complete command. */

    } while  ((--rattempt != 0) && (resp != DATA_RES_ACCEPTED) );

    return resp == DATA_RES_ACCEPTED ? RES_OK : RES_ERROR;
}


/*-----------------------------------------------------------------------*/
/* Write Sector(s)                                                       */
/*-----------------------------------------------------------------------*/

#if __SDCC
DRESULT disk_write (
    BYTE pdrv,              /* Physical drive number (0 or 1) */
    const BYTE *buff,       /* Pointer to the data to be written */
    LBA_t sector,           /* Start sector number (LBA) */
    UINT count              /* Sector count (1..128) */
) __preserves_regs(iyh,iyl)
#elif __SCCZ80
DRESULT disk_write (
    BYTE pdrv,              /* Physical drive number (0 or 1 ) */
    const BYTE *buff,       /* Pointer to the data to be written */
    LBA_t sector,           /* Start sector number (LBA) */
    UINT count              /* Sector count (1..128) */
) __smallc
#endif
{
    uint8_t wattempt = WRITE_ATTEMPTS;              /* Write attempts */
    uint8_t resp = 0;

    if (pdrv > 1 || !count) return RES_PARERR;      /* only drive 0 and 1 supported, and sector count can't be zero */
    if (Stat & STA_NOINIT) return RES_NOTRDY;
    if (Stat & STA_PROTECT) return RES_WRPRT;

    do {
        select(pdrv);

        if (!(CardType & CT_BLOCK)) sector *= 512;  /* Convert LBA to byte address if needed */

        if (count == 1) {                           /* Single block write */
            if ((send_cmd(CMD24, sector) == R1_READY_STATE)) {  /* WRITE_BLOCK */
                resp = write_sector(buff, DATA_START_BLOCK);
                count = 0;
            }
        } else {                                    /* Multiple block write */
            if (CardType & CT_SDC)
                send_cmd(ACMD23, count);
            if (send_cmd(CMD25, sector) == R1_READY_STATE) {    /* WRITE_MULTIPLE_BLOCK */
                do {
                    if ((resp = write_sector(buff, WRITE_MULTIPLE_TOKEN)) != DATA_RES_ACCEPTED) break;
                    buff += 512;
                } while (--count);
                if ((CardType & CT_SDC) && (resp == DATA_RES_ACCEPTED))
                    write_sector(0, STOP_TRAN_TOKEN);        /* Send STOP_TRAN token for SD cards. */
            }
        }

        deselect();                                 /* Give SD Card 8 Clocks to complete command. */

    } while ((--wattempt != 0) && (resp != DATA_RES_ACCEPTED) );

    return count ? RES_ERROR : RES_OK;
}


/*-----------------------------------------------------------------------*/
/* Miscellaneous Functions                                               */
/*-----------------------------------------------------------------------*/

#if __SDCC
DRESULT disk_ioctl (
    BYTE pdrv,              /* Physical drive number (0 or 1) */
    BYTE cmd,               /* Control code */
    void *buff              /* Buffer to send/receive control data */
) __preserves_regs(iyh,iyl)
#elif __SCCZ80
DRESULT disk_ioctl (
    BYTE pdrv,              /* Physical drive number (0 or 1) */
    BYTE cmd,               /* Control code */
    void *buff              /* Buffer to send/receive control data */
) __smallc
#endif
{
    DRESULT resp;
    DWORD *dp, st, ed, cs;
    BYTE n, csd[16];

    BYTE *ptr;          /*  8 bit integers for normal addresses */
    uint32_t *erasePtr; /* 32 bit integers for the erase sector (or byte) addresses */

    ptr = (BYTE *)buff;
    erasePtr = (uint32_t *)buff;

#if __SDCC
    if (disk_status_fastcall(pdrv) & STA_NOINIT) return RES_NOTRDY;  /* Check if card is in the socket */
#elif __SCCZ80
    if (disk_status(pdrv) & STA_NOINIT) return RES_NOTRDY;  /* Check if card is in the socket */
#endif

    select(pdrv);

    resp = RES_ERROR;

    switch (cmd) {

        case CTRL_SYNC :        /* Make sure that no pending write process */
            resp = RES_OK;
            break;

        case GET_SECTOR_COUNT :    /* Get number of sectors on the disk (DWORD) */
            if ((send_cmd(CMD9, 0) == R1_READY_STATE) && read_data(csd, 16)) {
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
                if ((send_cmd(ACMD13, 0) == R1_READY_STATE) && read_data(csd, 16))
                {                /* Read partial block */
                    for (n = 64 - 16; n; --n) sd_read_byte();       /* Purge trailing data */
                    *(uint32_t*)buff = 16UL << (csd[10] >> 4);
                    resp = RES_OK;
                }
            } else {                    /* SDv1 or MMCv3 */
                if ((send_cmd(CMD9, 0) == R1_READY_STATE) && read_data(csd, 16))
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

        case CTRL_TRIM:     /* Erase a block of sectors (used when _USE_TRIM in ffconf.h is 1) */
            if (!(CardType & CT_SDC)) break;                /* Check if the card is SDC */
            if (disk_ioctl(pdrv, MMC_GET_CSD, csd)) break;  /* Get CSD */
            if (!(csd[0] >> 6) && !(csd[10] & 0x40)) break; /* Check if sector erase can be applied to the card */
            dp = buff; st = dp[0]; ed = dp[1];              /* Load sector block */
            if (!(CardType & CT_BLOCK)) {
                st *= 512; ed *= 512;
            }
            if (send_cmd(CMD32, st) == 0 && send_cmd(CMD33, ed) == 0 && send_cmd(CMD38, 0) == 0 && wait_ready(true))   /* Erase sector block */
                resp = RES_OK;  /* FatFs does not check result of this command */
            break;

    /* Following commands are never used by FatFs module */

        case MMC_GET_TYPE :     /* Get card type flags (1 byte) */
            *ptr = CardType;
            resp = RES_OK;
            break;

        case MMC_GET_CSD :      /* Receive CSD as a data block (16 bytes) */
            if ((send_cmd(CMD9, 0) == R1_READY_STATE) && read_data(ptr, 16))     /* READ_CSD */
                resp = RES_OK;
            break;

        case MMC_GET_CID :      /* Receive CID as a data block (16 bytes) */
            if ((send_cmd(CMD10, 0) == R1_READY_STATE) && read_data(ptr, 16))    /* READ_CID */
                resp = RES_OK;
            break;

        case MMC_GET_OCR :      /* Receive OCR as an R3 response (4 bytes) */
            if (send_cmd(CMD58, 0) == R1_READY_STATE) {    /* READ_OCR */
                for (n = 4; n; --n) *ptr++ = sd_read_byte();
                resp = RES_OK;
            }
            break;

        case MMC_GET_SDSTAT :   /* Receive SD status as a data block (64 bytes) */
            if ((send_cmd(ACMD13, 0) == R1_READY_STATE) && read_data(ptr, 64))   /* SD_STATUS */
                resp = RES_OK;
            break;

        default:
            resp = RES_PARERR;
            break;
    }
    deselect();                 /* Give SD Card 8 Clocks to complete command. */

    return resp;
}
