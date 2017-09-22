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


#ifndef FF_PRIVATE_DEFINED
#define FF_PRIVATE_DEFINED

#ifdef __cplusplus
extern "C" {
#endif

/* ASCII code support macros */
#define IsUpper(c)    ((c) >= 'A' && (c) <= 'Z')
#define IsLower(c)    ((c) >= 'a' && (c) <= 'z')
#define IsDigit(c)    ((c) >= '0' && (c) <= '9')


/* Additional file attribute bits for internal use */
#define AM_VOL        0x08    /* Volume label */
#define AM_LFN        0x0F    /* LFN entry */
#define AM_MASK        0x3F    /* Mask of defined bits */


/* Additional file access control and file status flags for internal use */
#define FA_SEEKEND    0x20    /* Seek to end of the file on file open */
#define FA_MODIFIED    0x40    /* File has been modified */
#define FA_DIRTY    0x80    /* FIL.buf[] needs to be written-back */


/* Name status flags in fn[11] */
#define NSFLAG        11        /* Index of the name status byte */
#define NS_LOSS        0x01    /* Out of 8.3 format */
#define NS_LFN        0x02    /* Force to create LFN entry */
#define NS_LAST        0x04    /* Last segment */
#define NS_BODY        0x08    /* Lower case flag (body) */
#define NS_EXT        0x10    /* Lower case flag (ext) */
#define NS_DOT        0x20    /* Dot entry */
#define NS_NOLFN    0x40    /* Do not find LFN */
#define NS_NONAME    0x80    /* Not followed */


/* Limits and boundaries */
#define MAX_DIR        0x200000        /* Max size of FAT directory */
#define MAX_DIR_EX    0x10000000        /* Max size of exFAT directory */
#define MAX_FAT12    0xFF5            /* Max FAT12 clusters (differs from specs, but correct for real DOS/Windows behavior) */
#define MAX_FAT16    0xFFF5            /* Max FAT16 clusters (differs from specs, but correct for real DOS/Windows behavior) */
#define MAX_FAT32    0x0FFFFFF5        /* Max FAT32 clusters (not specified, practical limit) */
#define MAX_EXFAT    0x7FFFFFFD        /* Max exFAT clusters (differs from specs, implementation limit) */


/* FatFs refers the FAT structure as simple byte array instead of structure member
/ because the C structure is not binary compatible between different platforms */

#define BS_JmpBoot            0        /* x86 jump instruction (3-byte) */
#define BS_OEMName            3        /* OEM name (8-byte) */
#define BPB_BytsPerSec        11        /* Sector size [byte] (WORD) */
#define BPB_SecPerClus        13        /* Cluster size [sector] (BYTE) */
#define BPB_RsvdSecCnt        14        /* Size of reserved area [sector] (WORD) */
#define BPB_NumFATs            16        /* Number of FATs (BYTE) */
#define BPB_RootEntCnt        17        /* Size of root directory area for FAT [entry] (WORD) */
#define BPB_TotSec16        19        /* Volume size (16-bit) [sector] (WORD) */
#define BPB_Media            21        /* Media descriptor byte (BYTE) */
#define BPB_FATSz16            22        /* FAT size (16-bit) [sector] (WORD) */
#define BPB_SecPerTrk        24        /* Number of sectors per track for int13h [sector] (WORD) */
#define BPB_NumHeads        26        /* Number of heads for int13h (WORD) */
#define BPB_HiddSec            28        /* Volume offset from top of the drive (DWORD) */
#define BPB_TotSec32        32        /* Volume size (32-bit) [sector] (DWORD) */
#define BS_DrvNum            36        /* Physical drive number for int13h (BYTE) */
#define BS_NTres            37        /* WindowsNT error flag (BYTE) */
#define BS_BootSig            38        /* Extended boot signature (BYTE) */
#define BS_VolID            39        /* Volume serial number (DWORD) */
#define BS_VolLab            43        /* Volume label string (8-byte) */
#define BS_FilSysType        54        /* Filesystem type string (8-byte) */
#define BS_BootCode            62        /* Boot code (448-byte) */
#define BS_55AA                510        /* Signature word (WORD) */

#define BPB_FATSz32            36        /* FAT32: FAT size [sector] (DWORD) */
#define BPB_ExtFlags32        40        /* FAT32: Extended flags (WORD) */
#define BPB_FSVer32            42        /* FAT32: Filesystem version (WORD) */
#define BPB_RootClus32        44        /* FAT32: Root directory cluster (DWORD) */
#define BPB_FSInfo32        48        /* FAT32: Offset of FSINFO sector (WORD) */
#define BPB_BkBootSec32        50        /* FAT32: Offset of backup boot sector (WORD) */
#define BS_DrvNum32            64        /* FAT32: Physical drive number for int13h (BYTE) */
#define BS_NTres32            65        /* FAT32: Error flag (BYTE) */
#define BS_BootSig32        66        /* FAT32: Extended boot signature (BYTE) */
#define BS_VolID32            67        /* FAT32: Volume serial number (DWORD) */
#define BS_VolLab32            71        /* FAT32: Volume label string (8-byte) */
#define BS_FilSysType32        82        /* FAT32: Filesystem type string (8-byte) */
#define BS_BootCode32        90        /* FAT32: Boot code (420-byte) */

#define BPB_ZeroedEx        11        /* exFAT: MBZ field (53-byte) */
#define BPB_VolOfsEx        64        /* exFAT: Volume offset from top of the drive [sector] (QWORD) */
#define BPB_TotSecEx        72        /* exFAT: Volume size [sector] (QWORD) */
#define BPB_FatOfsEx        80        /* exFAT: FAT offset from top of the volume [sector] (DWORD) */
#define BPB_FatSzEx            84        /* exFAT: FAT size [sector] (DWORD) */
#define BPB_DataOfsEx        88        /* exFAT: Data offset from top of the volume [sector] (DWORD) */
#define BPB_NumClusEx        92        /* exFAT: Number of clusters (DWORD) */
#define BPB_RootClusEx        96        /* exFAT: Root directory start cluster (DWORD) */
#define BPB_VolIDEx            100        /* exFAT: Volume serial number (DWORD) */
#define BPB_FSVerEx            104        /* exFAT: Filesystem version (WORD) */
#define BPB_VolFlagEx        106        /* exFAT: Volume flags (BYTE) */
#define BPB_ActFatEx        107        /* exFAT: Active FAT flags (BYTE) */
#define BPB_BytsPerSecEx    108        /* exFAT: Log2 of sector size in unit of byte (BYTE) */
#define BPB_SecPerClusEx    109        /* exFAT: Log2 of cluster size in unit of sector (BYTE) */
#define BPB_NumFATsEx        110        /* exFAT: Number of FATs (BYTE) */
#define BPB_DrvNumEx        111        /* exFAT: Physical drive number for int13h (BYTE) */
#define BPB_PercInUseEx        112        /* exFAT: Percent in use (BYTE) */
#define BPB_RsvdEx            113        /* exFAT: Reserved (7-byte) */
#define BS_BootCodeEx        120        /* exFAT: Boot code (390-byte) */

#define DIR_Name            0        /* Short file name (11-byte) */
#define DIR_Attr            11        /* Attribute (BYTE) */
#define DIR_NTres            12        /* Lower case flag (BYTE) */
#define DIR_CrtTime10        13        /* Created time sub-second (BYTE) */
#define DIR_CrtTime            14        /* Created time (DWORD) */
#define DIR_LstAccDate        18        /* Last accessed date (WORD) */
#define DIR_FstClusHI        20        /* Higher 16-bit of first cluster (WORD) */
#define DIR_ModTime            22        /* Modified time (DWORD) */
#define DIR_FstClusLO        26        /* Lower 16-bit of first cluster (WORD) */
#define DIR_FileSize        28        /* File size (DWORD) */
#define LDIR_Ord            0        /* LFN: LFN order and LLE flag (BYTE) */
#define LDIR_Attr            11        /* LFN: LFN attribute (BYTE) */
#define LDIR_Type            12        /* LFN: Entry type (BYTE) */
#define LDIR_Chksum            13        /* LFN: Checksum of the SFN (BYTE) */
#define LDIR_FstClusLO        26        /* LFN: MBZ field (WORD) */
#define XDIR_Type            0        /* exFAT: Type of exFAT directory entry (BYTE) */
#define XDIR_NumLabel        1        /* exFAT: Number of volume label characters (BYTE) */
#define XDIR_Label            2        /* exFAT: Volume label (11-WORD) */
#define XDIR_CaseSum        4        /* exFAT: Sum of case conversion table (DWORD) */
#define XDIR_NumSec            1        /* exFAT: Number of secondary entries (BYTE) */
#define XDIR_SetSum            2        /* exFAT: Sum of the set of directory entries (WORD) */
#define XDIR_Attr            4        /* exFAT: File attribute (WORD) */
#define XDIR_CrtTime        8        /* exFAT: Created time (DWORD) */
#define XDIR_ModTime        12        /* exFAT: Modified time (DWORD) */
#define XDIR_AccTime        16        /* exFAT: Last accessed time (DWORD) */
#define XDIR_CrtTime10        20        /* exFAT: Created time subsecond (BYTE) */
#define XDIR_ModTime10        21        /* exFAT: Modified time subsecond (BYTE) */
#define XDIR_CrtTZ            22        /* exFAT: Created timezone (BYTE) */
#define XDIR_ModTZ            23        /* exFAT: Modified timezone (BYTE) */
#define XDIR_AccTZ            24        /* exFAT: Last accessed timezone (BYTE) */
#define XDIR_GenFlags        33        /* exFAT: General secondary flags (BYTE) */
#define XDIR_NumName        35        /* exFAT: Number of file name characters (BYTE) */
#define XDIR_NameHash        36        /* exFAT: Hash of file name (WORD) */
#define XDIR_ValidFileSize    40        /* exFAT: Valid file size (QWORD) */
#define XDIR_FstClus        52        /* exFAT: First cluster of the file data (DWORD) */
#define XDIR_FileSize        56        /* exFAT: File/Directory size (QWORD) */

#define SZDIRE                32        /* Size of a directory entry */
#define DDEM                0xE5    /* Deleted directory entry mark set to DIR_Name[0] */
#define RDDEM                0x05    /* Replacement of the character collides with DDEM */
#define LLEF                0x40    /* Last long entry flag in LDIR_Ord */

#define FSI_LeadSig            0        /* FAT32 FSI: Leading signature (DWORD) */
#define FSI_StrucSig        484        /* FAT32 FSI: Structure signature (DWORD) */
#define FSI_Free_Count        488        /* FAT32 FSI: Number of free clusters (DWORD) */
#define FSI_Nxt_Free        492        /* FAT32 FSI: Last allocated cluster (DWORD) */

#define MBR_Table            446        /* MBR: Offset of partition table in the MBR */
#define SZ_PTE                16        /* MBR: Size of a partition table entry */
#define PTE_Boot            0        /* MBR PTE: Boot indicator */
#define PTE_StHead            1        /* MBR PTE: Start head */
#define PTE_StSec            2        /* MBR PTE: Start sector */
#define PTE_StCyl            3        /* MBR PTE: Start cylinder */
#define PTE_System            4        /* MBR PTE: System ID */
#define PTE_EdHead            5        /* MBR PTE: End head */
#define PTE_EdSec            6        /* MBR PTE: End sector */
#define PTE_EdCyl            7        /* MBR PTE: End cylinder */
#define PTE_StLba            8        /* MBR PTE: Start in LBA */
#define PTE_SizLba            12        /* MBR PTE: Size in LBA */


/* Post process after fatal error on file operation */
#define ABORT(fs, res)        { fp->err = (BYTE)(res); LEAVE_FF(fs, res); }


/* Reentrancy related */
#if FF_FS_REENTRANT
#if FF_USE_LFN == 1
#error Static LFN work area cannot be used at thread-safe configuration
#endif
#define LEAVE_FF(fs, res)    { unlock_fs(fs, res); return res; }
#else
#define LEAVE_FF(fs, res)    return res
#endif


/* Definitions of volume - partition conversion */
#if FF_MULTI_PARTITION
#define LD2PD(vol) VolToPart[vol].pd    /* Get physical drive number */
#define LD2PT(vol) VolToPart[vol].pt    /* Get partition index */
#else
#define LD2PD(vol) (BYTE)(vol)    /* Each logical drive is bound to the same physical drive number */
#define LD2PT(vol) 0            /* Find first valid partition or in SFD */
#endif


/* Definitions of sector size */
#if (FF_MAX_SS < FF_MIN_SS) || (FF_MAX_SS != 512 && FF_MAX_SS != 1024 && FF_MAX_SS != 2048 && FF_MAX_SS != 4096) || (FF_MIN_SS != 512 && FF_MIN_SS != 1024 && FF_MIN_SS != 2048 && FF_MIN_SS != 4096)
#error Wrong sector size configuration
#endif
#if FF_MAX_SS == FF_MIN_SS
#define SS(fs)    ((UINT)FF_MAX_SS)    /* Fixed sector size */
#else
#define SS(fs)    ((fs)->ssize)    /* Variable sector size */
#endif


/* Timestamp */
#if FF_FS_NORTC == 1
#if FF_NORTC_YEAR < 1980 || FF_NORTC_YEAR > 2107 || FF_NORTC_MON < 1 || FF_NORTC_MON > 12 || FF_NORTC_MDAY < 1 || FF_NORTC_MDAY > 31
#error Invalid FF_FS_NORTC settings
#endif
#define GET_FATTIME()    ((DWORD)(FF_NORTC_YEAR - 1980) << 25 | (DWORD)FF_NORTC_MON << 21 | (DWORD)FF_NORTC_MDAY << 16)
#else
#define GET_FATTIME()    get_fattime()
#endif


/* File lock controls */
#if FF_FS_LOCK != 0
#if FF_FS_READONLY
#error FF_FS_LOCK must be 0 at read-only configuration
#endif
typedef struct {
    FATFS *fs;        /* Object ID 1, volume (NULL:blank entry) */
    DWORD clu;        /* Object ID 2, containing directory (0:root) */
    DWORD ofs;        /* Object ID 3, offset in the directory */
    WORD ctr;        /* Object open counter, 0:none, 0x01..0xFF:read mode open count, 0x100:write mode */
} FILESEM;
#endif



/*------------------------------------------------------------------------*/
/* LFN/Directory working buffer                                           */
/*------------------------------------------------------------------------*/

#if FF_USE_LFN == 0        /* Non-LFN configuration */
#define DEF_NAMBUF
#define INIT_NAMBUF(fs)
#define FREE_NAMBUF()

#else                    /* LFN configurations */
#if FF_MAX_LFN < 12 || FF_MAX_LFN > 255
#error Wrong FF_MAX_LFN setting
#endif

#define MAXDIRB(nc)    ((nc + 44U) / 15 * SZDIRE)    /* exFAT: Size of directory entry block scratchpad buffer needed for the max name length */

#if FF_USE_LFN == 1        /* LFN enabled with static working buffer defined in __ffstore.c */
#if FF_FS_EXFAT
#endif

#define DEF_NAMBUF
#define INIT_NAMBUF(fs)
#define FREE_NAMBUF()

#elif FF_USE_LFN == 2     /* LFN enabled with dynamic working buffer on the stack */
#if FF_FS_EXFAT
#define DEF_NAMBUF        WCHAR lbuf[FF_MAX_LFN+1]; BYTE dbuf[MAXDIRB(FF_MAX_LFN)];    /* LFN working buffer and directory entry block scratchpad buffer */
#define INIT_NAMBUF(fs)    { (fs)->lfnbuf = lbuf; (fs)->dirbuf = dbuf; }
#define FREE_NAMBUF()
#else
#define DEF_NAMBUF        WCHAR lbuf[FF_MAX_LFN+1];    /* LFN working buffer */
#define INIT_NAMBUF(fs)    { (fs)->lfnbuf = lbuf; }
#define FREE_NAMBUF()
#endif

#elif FF_USE_LFN == 3     /* LFN enabled with dynamic working buffer on the heap */
#if FF_FS_EXFAT
#define DEF_NAMBUF        WCHAR *lfn;    /* Pointer to LFN working buffer and directory entry block scratchpad buffer */
#define INIT_NAMBUF(fs)    { lfn = ff_memalloc((FF_MAX_LFN+1)*2 + MAXDIRB(FF_MAX_LFN)); if (!lfn) LEAVE_FF(fs, FR_NOT_ENOUGH_CORE); (fs)->lfnbuf = lfn; (fs)->dirbuf = (BYTE*)(lfn+FF_MAX_LFN+1); }
#define FREE_NAMBUF()    ff_memfree(lfn)
#else
#define DEF_NAMBUF        WCHAR *lfn;    /* Pointer to LFN working buffer */
#define INIT_NAMBUF(fs)    { lfn = ff_memalloc((FF_MAX_LFN+1)*2); if (!lfn) LEAVE_FF(fs, FR_NOT_ENOUGH_CORE); (fs)->lfnbuf = lfn; }
#define FREE_NAMBUF()    ff_memfree(lfn)
#endif

#else
#error Wrong FF_USE_LFN setting

#endif
#endif

/*-----------------------------------------------------------------------*/
/* Load/Store multi-byte word in the FAT structure                       */
/*-----------------------------------------------------------------------*/

WORD ld_word (const BYTE* ptr);         /* Load a 2-byte little-endian word */


DWORD ld_dword (const BYTE* ptr);       /* Load a 4-byte little-endian word */


#if FF_FS_EXFAT
QWORD ld_qword (const BYTE* ptr);       /* Load an 8-byte little-endian word */
#endif

#if !FF_FS_READONLY
void st_word (BYTE* ptr, WORD val);     /* Store a 2-byte word in little-endian */
void st_dword (BYTE* ptr, DWORD val);   /* Store a 4-byte word in little-endian */
#if FF_FS_EXFAT
void st_qword (BYTE* ptr, QWORD val);   /* Store an 8-byte word in little-endian */
#endif
#endif    /* !FF_FS_READONLY */


/*-----------------------------------------------------------------------*/
/* String functions                                                      */
/*-----------------------------------------------------------------------*/

/* Copy memory to memory */
void mem_cpy (void* dst, const void* src, UINT cnt);

/* Fill memory block */
void mem_set (void* dst, int val, UINT cnt);

/* Compare memory block */
int mem_cmp (const void* dst, const void* src, UINT cnt);   /* ZR:same, NZ:different */

/* Check if chr is contained in the string */
int chk_chr (const char* str, int chr);   /* NZ:contained, ZR:not contained */

/* String access macros */

#include <string.h>

#ifdef __STRING_H__     /* We have included the string functions from z88dk */
                        /* This saves space and speeds up functions */
#define MEMCPY(a,b,c)   memcpy(a,b,c)
#define MEMSET(a,b,c)   memset(a,b,c)
#define MEMCMP(a,b,c)   memcmp(a,b,c)
#define CHKCHR(a,b)     strchr(a,b)
#else
#define MEMCPY(a,b,c)   mem_cpy(a,b,c)
#define MEMSET(a,b,c)   mem_set(a,b,c)
#define MEMCMP(a,b,c)   mem_cmp(a,b,c)
#define CHKCHR(a,b)     chk_chr(a,b)
#endif


/*-----------------------------------------------------------------------*/
/* Code Page functions                                                   */
/*-----------------------------------------------------------------------*/

/* Test if the character is DBC 1st byte */
int dbc_1st (BYTE c);

/* Test if the character is DBC 2nd byte */
int dbc_2nd (BYTE c);


/*-----------------------------------------------------------------------*/
/* Additional user defined system functions  (ffsystem.c)                */
/*-----------------------------------------------------------------------*/

/* RTC function */
#if !FF_FS_READONLY && !FF_FS_NORTC
DWORD get_fattime (void);
DWORD get_fattime_basic (void);
#define get_fattime(a) get_fattime_basic(a)
#endif

/* Sync functions */
#if FF_FS_REENTRANT
int ff_cre_syncobj (BYTE vol, FF_SYNC_t* sobj); /* Create a sync object */
int ff_req_grant (FF_SYNC_t sobj);      /* Lock sync object */
void ff_rel_grant (FF_SYNC_t sobj);     /* Unlock sync object */
int ff_del_syncobj (FF_SYNC_t sobj);    /* Delete a sync object */
#endif


/*-----------------------------------------------------------------------*/
/* Additional system functions  (ffunicode.c)                            */
/*-----------------------------------------------------------------------*/

/* LFN support functions */
#if FF_USE_LFN                          /* Code conversion (defined in unicode.c) */
WCHAR ff_oem2uni (WCHAR oem, WORD cp);  /* OEM code to Unicode conversion */
WCHAR ff_uni2oem (WCHAR uni, WORD cp);  /* Unicode to OEM code conversion */
WCHAR ff_wtoupper (WCHAR uni);          /* Unicode upper-case conversion */
#endif
#if FF_USE_LFN == 3                     /* Dynamic memory allocation */
void* ff_memalloc (UINT msize);         /* Allocate memory block */
void ff_memfree (void* mblock);         /* Free memory block */
#endif


/*-----------------------------------------------------------------------*/
/* Request/Release grant to access the volume  (ffsystem.c)              */
/*-----------------------------------------------------------------------*/
#if FF_FS_REENTRANT

int lock_fs (        /* 1:Ok, 0:timeout */
    FATFS* fs        /* Filesystem object */
);

void unlock_fs (
    FATFS* fs,        /* Filesystem object */
    FRESULT res       /* Result code to be returned */
);

#endif


/*-----------------------------------------------------------------------*/
/* File lock control functions                                           */
/*-----------------------------------------------------------------------*/
#if FF_FS_LOCK != 0

FRESULT chk_lock (    /* Check if the file can be accessed */
    DIR* dp,        /* Directory object pointing the file to be checked */
    int acc            /* Desired access type (0:Read mode open, 1:Write mode open, 2:Delete or rename) */
);

int enq_lock (void);    /* Check if an entry is available for a new object */


UINT inc_lock (    /* Increment object open counter and returns its index (0:Internal error) */
    DIR* dp,    /* Directory object pointing the file to register or increment */
    int acc        /* Desired access (0:Read, 1:Write, 2:Delete/Rename) */
);


FRESULT dec_lock (    /* Decrement object open counter */
    UINT i            /* Semaphore index (1..) */
);

void clear_lock (    /* Clear lock entries of the volume */
    FATFS *fs
);

#endif    /* FF_FS_LOCK != 0 */


/*-----------------------------------------------------------------------*/
/* Move/Flush disk access window in the filesystem object                */
/*-----------------------------------------------------------------------*/
#if !FF_FS_READONLY

FRESULT sync_window (   /* Returns FR_OK or FR_DISK_ERR */
    FATFS* fs           /* Filesystem object */
);
#endif

FRESULT move_window (   /* Returns FR_OK or FR_DISK_ERR */
    FATFS* fs,          /* Filesystem object */
    DWORD sector        /* Sector number to make appearance in the fs->win[] */
);


/*-----------------------------------------------------------------------*/
/* Synchronize filesystem and data on the storage                        */
/*-----------------------------------------------------------------------*/
#if !FF_FS_READONLY

FRESULT sync_fs (    /* Returns FR_OK or FR_DISK_ERR */
    FATFS* fs        /* Filesystem object */
);

#endif


/*-----------------------------------------------------------------------*/
/* Get physical sector number from cluster number                        */
/*-----------------------------------------------------------------------*/

DWORD clst2sect (    /* !=0:Sector number, 0:Failed (invalid cluster#) */
    FATFS* fs,        /* Filesystem object */
    DWORD clst        /* Cluster# to be converted */
);

/*-----------------------------------------------------------------------*/
/* FAT access - Read value of a FAT entry                                */
/*-----------------------------------------------------------------------*/

DWORD get_fat (        /* 0xFFFFFFFF:Disk error, 1:Internal error, 2..0x7FFFFFFF:Cluster status */
    FFOBJID* obj,    /* Corresponding object */
    DWORD clst        /* Cluster number to get the value */
);


/*-----------------------------------------------------------------------*/
/* FAT access - Change value of a FAT entry                              */
/*-----------------------------------------------------------------------*/
#if !FF_FS_READONLY

FRESULT put_fat (    /* FR_OK(0):succeeded, !=0:error */
    FATFS* fs,        /* Corresponding filesystem object */
    DWORD clst,        /* FAT index number (cluster number) to be changed */
    DWORD val        /* New value to be set to the entry */
);

#endif /* !FF_FS_READONLY */


/*-----------------------------------------------------------------------*/
/* exFAT: Accessing FAT and Allocation Bitmap                            */
/*-----------------------------------------------------------------------*/
#if FF_FS_EXFAT && !FF_FS_READONLY

/*--------------------------------------*/
/* Find a contiguous free cluster block */
/*--------------------------------------*/

DWORD find_bitmap (    /* 0:Not found, 2..:Cluster block found, 0xFFFFFFFF:Disk error */
    FATFS* fs,    /* Filesystem object */
    DWORD clst,    /* Cluster number to scan from */
    DWORD ncl    /* Number of contiguous clusters to find (1..) */
);


/*----------------------------------------*/
/* Set/Clear a block of allocation bitmap */
/*----------------------------------------*/

FRESULT change_bitmap (
    FATFS* fs,    /* Filesystem object */
    DWORD clst,    /* Cluster number to change from */
    DWORD ncl,    /* Number of clusters to be changed */
    int bv        /* bit value to be set (0 or 1) */
);


/*---------------------------------------------*/
/* Fill the first fragment of the FAT chain    */
/*---------------------------------------------*/

FRESULT fill_first_frag (
    FFOBJID* obj    /* Pointer to the corresponding object */
);


/*---------------------------------------------*/
/* Fill the last fragment of the FAT chain     */
/*---------------------------------------------*/

FRESULT fill_last_frag (
    FFOBJID* obj,    /* Pointer to the corresponding object */
    DWORD lcl,        /* Last cluster of the fragment */
    DWORD term        /* Value to set the last FAT entry */
);

#endif    /* FF_FS_EXFAT && !FF_FS_READONLY */


/*-----------------------------------------------------------------------*/
/* FAT handling - Remove a cluster chain                                 */
/*-----------------------------------------------------------------------*/
#if !FF_FS_READONLY

FRESULT remove_chain (    /* FR_OK(0):succeeded, !=0:error */
    FFOBJID* obj,        /* Corresponding object */
    DWORD clst,            /* Cluster to remove a chain from */
    DWORD pclst            /* Previous cluster of clst (0:entire chain) */
);


/*-----------------------------------------------------------------------*/
/* FAT handling - Stretch a chain or Create a new chain                  */
/*-----------------------------------------------------------------------*/
DWORD create_chain (    /* 0:No free cluster, 1:Internal error, 0xFFFFFFFF:Disk error, >=2:New cluster# */
    FFOBJID* obj,        /* Corresponding object */
    DWORD clst            /* Cluster# to stretch, 0:Create a new chain */
);

#endif /* !FF_FS_READONLY */


/*-----------------------------------------------------------------------*/
/* FAT handling - Convert offset into cluster with link map table        */
/*-----------------------------------------------------------------------*/
#if FF_USE_FASTSEEK

DWORD clmt_clust (    /* <2:Error, >=2:Cluster number */
    FIL* fp,        /* Pointer to the file object */
    FSIZE_t ofs        /* File offset to be converted to cluster# */
);

#endif    /* FF_USE_FASTSEEK */


/*-----------------------------------------------------------------------*/
/* Directory handling - Fill a cluster with zeros                        */
/*-----------------------------------------------------------------------*/
#if !FF_FS_READONLY

FRESULT dir_clear (    /* Returns FR_OK or FR_DISK_ERR */
    FATFS *fs,        /* Filesystem object */
    DWORD clst        /* Directory table to clear */
);

#endif    /* !FF_FS_READONLY */


/*-----------------------------------------------------------------------*/
/* Directory handling - Set directory index                              */
/*-----------------------------------------------------------------------*/

FRESULT dir_sdi (    /* FR_OK(0):succeeded, !=0:error */
    DIR* dp,        /* Pointer to directory object */
    DWORD ofs        /* Offset of directory table */
);


/*-----------------------------------------------------------------------*/
/* Directory handling - Move directory table index next                  */
/*-----------------------------------------------------------------------*/

FRESULT dir_next (    /* FR_OK(0):succeeded, FR_NO_FILE:End of table, FR_DENIED:Could not stretch */
    DIR* dp,        /* Pointer to the directory object */
    int stretch        /* 0: Do not stretch table, 1: Stretch table if needed */
);


/*-----------------------------------------------------------------------*/
/* Directory handling - Reserve a block of directory entries             */
/*-----------------------------------------------------------------------*/
#if !FF_FS_READONLY

FRESULT dir_alloc (    /* FR_OK(0):succeeded, !=0:error */
    DIR* dp,        /* Pointer to the directory object */
    UINT nent        /* Number of contiguous entries to allocate */
);

#endif    /* !FF_FS_READONLY */


/*-----------------------------------------------------------------------*/
/* FAT: Directory handling - Load/Store start cluster number             */
/*-----------------------------------------------------------------------*/

DWORD ld_clust (    /* Returns the top cluster value of the SFN entry */
    FATFS* fs,        /* Pointer to the fs object */
    const BYTE* dir    /* Pointer to the key entry */
);


#if !FF_FS_READONLY
void st_clust (
    FATFS* fs,    /* Pointer to the fs object */
    BYTE* dir,    /* Pointer to the key entry */
    DWORD cl    /* Value to be set */
);
#endif


/*--------------------------------------------------------*/
/* FAT-LFN: Compare a part of file name with an LFN entry */
/*--------------------------------------------------------*/
#if FF_USE_LFN

int cmp_lfn (                /* 1:matched, 0:not matched */
    const WCHAR* lfnbuf,    /* Pointer to the LFN working buffer to be compared */
    BYTE* dir                /* Pointer to the directory entry containing the part of LFN */
);


/*-----------------------------------------------------*/
/* FAT-LFN: Pick a part of file name from an LFN entry */
/*-----------------------------------------------------*/
#if FF_FS_MINIMIZE <= 1 || FF_FS_RPATH >= 2 || FF_USE_LABEL || FF_FS_EXFAT

int pick_lfn (            /* 1:succeeded, 0:buffer overflow or invalid LFN entry */
    WCHAR* lfnbuf,        /* Pointer to the LFN working buffer */
    BYTE* dir            /* Pointer to the LFN entry */
);

#endif


/*-----------------------------------------*/
/* FAT-LFN: Create an entry of LFN entries */
/*-----------------------------------------*/
#if !FF_FS_READONLY

void put_lfn (
    const WCHAR* lfn,    /* Pointer to the LFN */
    BYTE* dir,            /* Pointer to the LFN entry to be created */
    BYTE ord,            /* LFN order (1-20) */
    BYTE sum            /* Checksum of the corresponding SFN */
);

#endif    /* !FF_FS_READONLY */
#endif    /* FF_USE_LFN */


/*-----------------------------------------------------------------------*/
/* FAT-LFN: Create a Numbered SFN                                        */
/*-----------------------------------------------------------------------*/
#if FF_USE_LFN && !FF_FS_READONLY

void gen_numname (
    BYTE* dst,            /* Pointer to the buffer to store numbered SFN */
    const BYTE* src,    /* Pointer to SFN */
    const WCHAR* lfn,    /* Pointer to LFN */
    UINT seq            /* Sequence number */
);
#endif    /* FF_USE_LFN && !FF_FS_READONLY */


/*-----------------------------------------------------------------------*/
/* FAT-LFN: Calculate checksum of an SFN entry                           */
/*-----------------------------------------------------------------------*/
#if FF_USE_LFN

BYTE sum_sfn (
    const BYTE* dir        /* Pointer to the SFN entry */
);

#endif    /* FF_USE_LFN */


/*-----------------------------------------------------------------------*/
/* exFAT: Checksum                                                       */
/*-----------------------------------------------------------------------*/
#if FF_FS_EXFAT

WORD xdir_sum (            /* Get checksum of the directoly entry block */
    const BYTE* dir        /* Directory entry block to be calculated */
);


WORD xname_sum (        /* Get check sum (to be used as hash) of the name */
    const WCHAR* name    /* File name to be calculated */
);


#if !FF_FS_READONLY && FF_USE_MKFS
DWORD xsum32 (
    BYTE  dat,    /* Byte to be calculated */
    DWORD sum    /* Previous sum */
);
#endif


/*------------------------------------------------------*/
/* exFAT: Get object information from a directory block */
/*------------------------------------------------------*/
#if FF_FS_MINIMIZE <= 1 || FF_FS_RPATH >= 2

void get_xdir_info (
    BYTE* dirb,            /* Pointer to the direcotry entry block 85+C0+C1s */
    FILINFO* fno        /* Buffer to store the extracted file information */
);

#endif    /* FF_FS_MINIMIZE <= 1 || FF_FS_RPATH >= 2 */


/*-----------------------------------*/
/* exFAT: Get a directry entry block */
/*-----------------------------------*/

FRESULT load_xdir (    /* FR_INT_ERR: invalid entry block */
    DIR* dp            /* Reading direcotry object pointing top of the entry block to load */
);


/*------------------------------------------------*/
/* exFAT: Load the object's directory entry block */
/*------------------------------------------------*/
#if !FF_FS_READONLY || FF_FS_RPATH != 0

FRESULT load_obj_xdir (    
    DIR* dp,            /* Blank directory object to be used to access containing direcotry */
    const FFOBJID* obj    /* Object with its containing directory information */
);
#endif


/*----------------------------------------*/
/* exFAT: Store the directory entry block */
/*----------------------------------------*/
#if !FF_FS_READONLY

FRESULT store_xdir (
    DIR* dp                /* Pointer to the direcotry object */
);


/*-------------------------------------------*/
/* exFAT: Create a new directory enrty block */
/*-------------------------------------------*/

void create_xdir (
    BYTE* dirb,            /* Pointer to the direcotry entry block buffer */
    const WCHAR* lfn    /* Pointer to the object name */
);

#endif    /* !FF_FS_READONLY */
#endif    /* FF_FS_EXFAT */


/*-----------------------------------------------------------------------*/
/* Read an object from the directory                                     */
/*-----------------------------------------------------------------------*/
#if FF_FS_MINIMIZE <= 1 || FF_FS_RPATH >= 2 || FF_USE_LABEL || FF_FS_EXFAT

FRESULT dir_read (
    DIR* dp,        /* Pointer to the directory object */
    int vol            /* Filtered by 0:file/directory or 1:volume label */
);
#endif    /* FF_FS_MINIMIZE <= 1 || FF_USE_LABEL || FF_FS_RPATH >= 2 */


/*-----------------------------------------------------------------------*/
/* Directory handling - Find an object in the directory                  */
/*-----------------------------------------------------------------------*/

FRESULT dir_find (    /* FR_OK(0):succeeded, !=0:error */
    DIR* dp            /* Pointer to the directory object with the file name */
);


/*-----------------------------------------------------------------------*/
/* Register an object to the directory                                   */
/*-----------------------------------------------------------------------*/
#if !FF_FS_READONLY

FRESULT dir_register (    /* FR_OK:succeeded, FR_DENIED:no free entry or too many SFN collision, FR_DISK_ERR:disk error */
    DIR* dp                /* Target directory with object name to be created */
);

#endif /* !FF_FS_READONLY */


/*-----------------------------------------------------------------------*/
/* Remove an object from the directory                                   */
/*-----------------------------------------------------------------------*/
#if !FF_FS_READONLY && FF_FS_MINIMIZE == 0

FRESULT dir_remove (    /* FR_OK:Succeeded, FR_DISK_ERR:A disk error */
    DIR* dp                /* Directory object pointing the entry to be removed */
);

#endif /* !FF_FS_READONLY && FF_FS_MINIMIZE == 0 */


/*-----------------------------------------------------------------------*/
/* Get file information from directory entry                             */
/*-----------------------------------------------------------------------*/
#if FF_FS_MINIMIZE <= 1 || FF_FS_RPATH >= 2

void get_fileinfo (        /* No return code */
    DIR* dp,            /* Pointer to the directory object */
    FILINFO* fno         /* Pointer to the file information to be filled */
);

#endif /* FF_FS_MINIMIZE <= 1 || FF_FS_RPATH >= 2 */


/*-----------------------------------------------------------------------*/
/* Pattern matching                                                      */
/*-----------------------------------------------------------------------*/
#if FF_USE_FIND && FF_FS_MINIMIZE <= 1

WCHAR get_achar (        /* Get a character and advances ptr 1 or 2 */
    const TCHAR** ptr    /* Pointer to pointer to the SBCS/DBCS/Unicode string */
);

int pattern_matching (    /* 0:not matched, 1:matched */
    const TCHAR* pat,    /* Matching pattern */
    const TCHAR* nam,    /* String to be tested */
    int skip,            /* Number of pre-skip chars (number of ?s) */
    int inf                /* Infinite search (* specified) */
);

#endif /* FF_USE_FIND && FF_FS_MINIMIZE <= 1 */


/*-----------------------------------------------------------------------*/
/* Pick a top segment and create the object name in directory form       */
/*-----------------------------------------------------------------------*/

FRESULT create_name (    /* FR_OK: successful, FR_INVALID_NAME: could not create */
    DIR* dp,            /* Pointer to the directory object */
    const TCHAR** path    /* Pointer to pointer to the segment in the path string */
);


/*-----------------------------------------------------------------------*/
/* Follow a file path                                                    */
/*-----------------------------------------------------------------------*/

FRESULT follow_path (    /* FR_OK(0): successful, !=0: error code */
    DIR* dp,            /* Directory object to return last directory and found object */
    const TCHAR* path    /* Full-path string to find a file or directory */
);


/*-----------------------------------------------------------------------*/
/* Get logical drive number from path name                               */
/*-----------------------------------------------------------------------*/

int get_ldnumber (        /* Returns logical drive number (-1:invalid drive) */
    const TCHAR** path    /* Pointer to pointer to the path name */
);


/*-----------------------------------------------------------------------*/
/* Load a sector and check if it is an FAT VBR                           */
/*-----------------------------------------------------------------------*/

BYTE check_fs (    /* 0:FAT, 1:exFAT, 2:Valid BS but not FAT, 3:Not a BS, 4:Disk error */
    FATFS* fs,    /* Filesystem object */
    DWORD sect    /* Sector# (lba) to load and check if it is an FAT-VBR or not */
);


/*-----------------------------------------------------------------------*/
/* Find logical drive and check if the volume is mounted                 */
/*-----------------------------------------------------------------------*/

FRESULT find_volume (    /* FR_OK(0): successful, !=0: any error occurred */
    const TCHAR** path,    /* Pointer to pointer to the path name (drive number) */
    FATFS** rfs,        /* Pointer to pointer to the found filesystem object */
    BYTE mode            /* !=0: Check write protection for write access */
);


/*-----------------------------------------------------------------------*/
/* Check if the file/directory object is valid or not                    */
/*-----------------------------------------------------------------------*/

FRESULT validate (    /* Returns FR_OK or FR_INVALID_OBJECT */
    FFOBJID* obj,    /* Pointer to the FFOBJID, the 1st member in the FIL/DIR object, to check validity */
    FATFS** rfs        /* Pointer to pointer to the owner filesystem object to return */
);


#ifdef __cplusplus
}
#endif

#endif /* FF_PRIVATE_DEFINED */

